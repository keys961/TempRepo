package service

import (
	"context"
	"entry-task/internal/pkg/cfg"
	"entry-task/internal/pkg/hash"
	pb "entry-task/internal/pkg/rpc"
	"entry-task/internal/pkg/status"
	"entry-task/internal/tcp-server/cache"
	"entry-task/internal/tcp-server/model"
	"entry-task/internal/tcp-server/repository"
	"entry-task/internal/tcp-server/utils"
	"fmt"
	"os"
)

type UserServiceGrpc struct {
	appConfig      *cfg.AppConfig
	userRepository *repository.UserRepository
	userCache      *cache.UserCache
}

func NewUserServiceGrpc(cfg *cfg.AppConfig, userRepo *repository.UserRepository, userCache *cache.UserCache) *UserServiceGrpc {
	svc := UserServiceGrpc{
		appConfig:      cfg,
		userRepository: userRepo,
		userCache:      userCache,
	}
	return &svc
}

func (u *UserServiceGrpc) Ping(_ context.Context, request *pb.PingRequest) (*pb.Reply, error) {
	return &pb.Reply{
		Code:    0,
		Message: request.Msg,
	}, nil
}

func (u *UserServiceGrpc) Login(_ context.Context, request *pb.LoginRequest) (reply *pb.Reply, err error) {
	reply = &pb.Reply{}

	user, err := u.userRepository.GetByName(request.Username)
	if err != nil {
		err = fillReplyAndError(status.ErrLogin, fmt.Sprintf("%v", err), reply)
		return
	}

	if user.Status == 0 {
		err = fillReplyAndError(status.ErrLogin, "user deactivated", reply)
		return
	}

	hashPass := hash.MD5(request.Password)
	if user.Password != hashPass {
		err = fillReplyAndError(status.ErrLogin, "password incorrect", reply)
		return
	}

	err = fillReplyAndError(status.Ok, "", reply)
	return
}

func (u *UserServiceGrpc) Register(_ context.Context, request *pb.RegisterRequest) (reply *pb.Reply, err error) {
	reply = &pb.Reply{}

	user, err := u.userRepository.GetByName(request.Username)
	if user != nil {
		err = fillReplyAndError(status.ErrRegister, "user existed", reply)
		return
	}

	hashPass := hash.MD5(request.Password)
	newUser := model.User{
		Name:     request.Username,
		Password: hashPass,
		Nickname: request.Nickname,
		Avatar:   request.Avatar,
		Status:   1,
	}
	err = u.userRepository.Create(&newUser)
	if err != nil {
		err = fillReplyAndError(status.ErrRegister, fmt.Sprintf("%v", err), reply)
	} else {
		err = fillReplyAndError(status.Ok, "", reply)
	}
	return
}

func (u *UserServiceGrpc) GetProfile(ctx context.Context, request *pb.GetProfileRequest) (reply *pb.GetProfileReply, err error) {
	reply = &pb.GetProfileReply{}
	reply.Reply = &pb.Reply{}

	user, err := u.userCache.Get(ctx, request.Username)
	// first query cache
	if err == nil {
		err = fillReplyAndError(status.Ok, "", reply.Reply)
		reply.Nickname = user.Nickname
		reply.Avatar = user.Avatar
		reply.Username = user.Name
		return
	}
	// next query db
	user, err = u.userRepository.GetByName(request.Username)
	if err != nil {
		err = fillReplyAndError(status.ErrGetProfile, fmt.Sprintf("%v", err), reply.Reply)
		return
	}
	if user.Status == 0 {
		err = fillReplyAndError(status.ErrGetProfile, "user deactivated", reply.Reply)
		return
	}
	err = fillReplyAndError(status.Ok, "", reply.Reply)
	reply.Nickname = user.Nickname
	reply.Avatar = user.Avatar
	reply.Username = user.Name
	// finally, load cache
	_ = u.userCache.Set(ctx, user)
	return
}

func (u *UserServiceGrpc) UpdateProfile(ctx context.Context, request *pb.UpdateProfileRequest) (reply *pb.Reply, err error) {
	reply = &pb.Reply{}
	// query and update db
	user, err := u.userRepository.GetByName(request.Username)
	if err != nil {
		err = fillReplyAndError(status.ErrUpdateProfile, fmt.Sprintf("%v", err), reply)
		return
	}
	if user.Status == 0 {
		err = fillReplyAndError(status.ErrUpdateProfile, "user deactivated", reply)
		return
	}
	err = u.userRepository.UpdateNicknameAndAvatar(user.ID, request.Nickname, request.Avatar)
	if err != nil {
		err = fillReplyAndError(status.ErrUpdateProfile, fmt.Sprintf("%v", err), reply)
		return
	}
	// invalidate cache
	_ = u.userCache.Delete(ctx, request.Username)
	err = fillReplyAndError(status.Ok, "", reply)
	return
}

func (u *UserServiceGrpc) UploadAvatar(ctx context.Context, request *pb.UploadRequest) (reply *pb.UploadReply, err error) {
	reply = &pb.UploadReply{}
	reply.Reply = &pb.Reply{}

	user, err := u.userRepository.GetByName(request.Username)
	if err != nil {
		err = fillReplyAndError(status.ErrUploadAvatar, fmt.Sprintf("%v", err), reply.Reply)
		return
	}
	if user.Status == 0 {
		err = fillReplyAndError(status.ErrUploadAvatar, "user deactivated", reply.Reply)
		return
	}

	filename := utils.HashedFileName(request.FileName, request.Username)
	path := u.appConfig.UploadSavePath
	dst := path + string(os.PathSeparator) + filename

	if !utils.CheckPath(path) {
		if err = utils.CreatePath(path, os.ModePerm); err != nil {
			err = fillReplyAndError(status.ErrUploadAvatar, fmt.Sprintf("%v", err), reply.Reply)
			return
		}
	}

	if !utils.CheckPermission(path) {
		err = fillReplyAndError(status.ErrUploadAvatar, "directory permission denied", reply.Reply)
		return
	}

	if err = utils.SaveFileBytes(&request.Content, dst); err != nil {
		err = fillReplyAndError(status.ErrUploadAvatar, fmt.Sprintf("%v", err), reply.Reply)
		return
	}
	// avatar url is for http server static resource...
	reply.Avatar = u.appConfig.UploadServerUrl + "/" + filename
	// update db
	user.Avatar = reply.Avatar
	err = u.userRepository.Update(user)
	if err != nil {
		err = fillReplyAndError(status.ErrUploadAvatar, fmt.Sprintf("%v", err), reply.Reply)
		return
	}
	// invalidate cache
	_ = u.userCache.Delete(ctx, request.Username)
	err = fillReplyAndError(status.Ok, "", reply.Reply)
	return
}

func fillReplyAndError(s status.Status, msg string, reply *pb.Reply) error {
	reply.Code = int32(s.Code)
	if s == status.Ok {
		reply.Message = s.Msg
		return nil
	}
	reply.Message = fmt.Sprintf("%s: %s", s.Msg, msg)
	return fmt.Errorf("%s: %s", s.Msg, msg)
}
