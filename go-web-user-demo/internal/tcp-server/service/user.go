package service

import (
	"context"
	"entry-task/internal/pkg/cfg"
	"entry-task/internal/pkg/hash"
	"entry-task/internal/pkg/rpc"
	"entry-task/internal/pkg/status"
	"entry-task/internal/tcp-server/cache"
	"entry-task/internal/tcp-server/model"
	"entry-task/internal/tcp-server/repository"
	"entry-task/internal/tcp-server/utils"
	"fmt"
	"os"
)

type UserService struct {
	appConfig      *cfg.AppConfig
	userRepository *repository.UserRepository
	userCache      *cache.UserCache
}

func NewUserService(cfg *cfg.AppConfig, userRepo *repository.UserRepository, userCache *cache.UserCache) *UserService {
	svc := UserService{
		appConfig:      cfg,
		userRepository: userRepo,
		userCache:      userCache,
	}
	return &svc
}

func (u *UserService) Ping(req rpc.PingReq, resp *rpc.Resp) error {
	resp.Message = req.Message
	resp.Code = 0
	return nil
}

func (u *UserService) Login(req rpc.LoginReq, resp *rpc.Resp) (err error) {
	user, err := u.userRepository.GetByName(req.Username)
	if err != nil {
		err = fillRespAndError(status.ErrLogin, fmt.Sprintf("%v", err), resp)
		return
	}

	if user.Status == 0 {
		err = fillRespAndError(status.ErrLogin, "user deactivated", resp)
		return
	}

	hashPass := hash.MD5(req.Password)
	if user.Password != hashPass {
		err = fillRespAndError(status.ErrLogin, "password incorrect", resp)
		return
	}

	err = fillRespAndError(status.Ok, "", resp)
	return
}

func (u *UserService) Register(req rpc.RegisterReq, resp *rpc.Resp) (err error) {
	user, err := u.userRepository.GetByName(req.Username)

	if user != nil {
		err = fillRespAndError(status.ErrRegister, "user existed", resp)
		return
	}

	hashPass := hash.MD5(req.Password)
	newUser := model.User{
		Name:     req.Username,
		Password: hashPass,
		Nickname: req.Nickname,
		Avatar:   req.Avatar,
		Status:   1,
	}
	err = u.userRepository.Create(&newUser)
	if err != nil {
		err = fillRespAndError(status.ErrRegister, fmt.Sprintf("%v", err), resp)
	} else {
		err = fillRespAndError(status.Ok, "", resp)
	}
	return
}

func (u *UserService) GetProfile(req rpc.GetProfileReq, resp *rpc.GetProfileResp) (err error) {
	ctx := context.Background()
	user, err := u.userCache.Get(ctx, req.Username)
	// first query cache
	if err == nil {
		err = fillRespAndError(status.Ok, "", &resp.Resp)
		resp.Nickname = user.Nickname
		resp.Avatar = user.Avatar
		resp.Username = user.Name
		return
	}
	// next query db
	user, err = u.userRepository.GetByName(req.Username)
	if err != nil {
		err = fillRespAndError(status.ErrGetProfile, fmt.Sprintf("%v", err), &resp.Resp)
		return
	}
	if user.Status == 0 {
		err = fillRespAndError(status.ErrGetProfile, "user deactivated", &resp.Resp)
		return
	}
	err = fillRespAndError(status.Ok, "", &resp.Resp)
	resp.Nickname = user.Nickname
	resp.Avatar = user.Avatar
	resp.Username = user.Name
	// finally, load cache
	_ = u.userCache.Set(ctx, user)
	return
}

func (u *UserService) UpdateProfile(req rpc.UpdateProfileReq, resp *rpc.Resp) (err error) {
	ctx := context.Background()
	user, err := u.userRepository.GetByName(req.Username)
	if err != nil {
		err = fillRespAndError(status.ErrUpdateProfile, fmt.Sprintf("%v", err), resp)
		return
	}
	if user.Status == 0 {
		err = fillRespAndError(status.ErrUpdateProfile, "user deactivated", resp)
		return
	}
	err = u.userRepository.UpdateNicknameAndAvatar(user.ID, req.Nickname, req.Avatar)
	if err != nil {
		err = fillRespAndError(status.ErrUpdateProfile, fmt.Sprintf("%v", err), resp)
		return
	}
	// invalidate cache
	_ = u.userCache.Delete(ctx, req.Username)
	err = fillRespAndError(status.Ok, "", resp)
	return
}

func (u *UserService) UploadAvatar(req rpc.UploadAvatarReq, resp *rpc.UploadAvatarResp) (err error) {
	ctx := context.Background()
	user, err := u.userRepository.GetByName(req.Username)
	if err != nil {
		err = fillRespAndError(status.ErrUploadAvatar, fmt.Sprintf("%v", err), &resp.Resp)
		return
	}
	if user.Status == 0 {
		err = fillRespAndError(status.ErrUploadAvatar, "user deactivated", &resp.Resp)
		return
	}

	filename := utils.HashedFileName(req.Filename, req.Username)
	path := u.appConfig.UploadSavePath
	dst := path + string(os.PathSeparator) + filename

	if !utils.CheckPath(path) {
		if err = utils.CreatePath(path, os.ModePerm); err != nil {
			err = fillRespAndError(status.ErrUploadAvatar, fmt.Sprintf("%v", err), &resp.Resp)
			return
		}
	}

	if !utils.CheckPermission(path) {
		err = fillRespAndError(status.ErrUploadAvatar, "directory permission denied", &resp.Resp)
		return
	}

	if err = utils.SaveFileBytes(&req.Content, dst); err != nil {
		err = fillRespAndError(status.ErrUploadAvatar, fmt.Sprintf("%v", err), &resp.Resp)
		return
	}
	// avatar url is for http server static resource...
	resp.Avatar = u.appConfig.UploadServerUrl + "/" + filename
	// update db
	user.Avatar = resp.Avatar
	err = u.userRepository.Update(user)
	if err != nil {
		err = fillRespAndError(status.ErrUploadAvatar, fmt.Sprintf("%v", err), &resp.Resp)
		return
	}
	// invalidate cache
	_ = u.userCache.Delete(ctx, req.Username)
	err = fillRespAndError(status.Ok, "", &resp.Resp)
	return
}

func fillRespAndError(s status.Status, msg string, reply *rpc.Resp) error {
	reply.Code = int32(s.Code)
	if s == status.Ok {
		reply.Message = s.Msg
		return nil
	}
	reply.Message = fmt.Sprintf("%s: %s", s.Msg, msg)
	return fmt.Errorf("%s: %s", s.Msg, msg)
}
