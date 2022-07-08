package service

import (
	"context"
	pb "entry-task/internal/pkg/rpc"
	"entry-task/internal/pkg/status"
	"entry-task/internal/tcp-server/utils"
	"fmt"
	"google.golang.org/grpc"
	"mime/multipart"
)

type UserServiceGrpc struct {
	ctx       context.Context
	rpcClient *grpc.ClientConn
}

func NewUserServiceGrpc(ctx context.Context, rpcClient *grpc.ClientConn) *UserServiceGrpc {
	return &UserServiceGrpc{
		ctx:       ctx,
		rpcClient: rpcClient,
	}
}

func (s *UserServiceGrpc) Login(req *LoginRequest) *Response {
	c := pb.NewUserServiceClient(s.rpcClient)
	rpcReq := pb.LoginRequest{
		Username: req.Username,
		Password: req.Password,
	}

	rpcResp, err := c.Login(s.ctx, &rpcReq) // call rpc login
	if err != nil {
		return &Response{
			Status: status.Status{
				Code: status.ErrLogin.Code,
				Msg:  fmt.Sprintf("%v", err),
			},
		}
	}

	return &Response{
		Status: status.Status{
			Code: int(rpcResp.Code),
			Msg:  rpcResp.Message,
		},
	}
}

func (s *UserServiceGrpc) Register(req *RegisterRequest) *Response {
	c := pb.NewUserServiceClient(s.rpcClient)
	rpcReq := pb.RegisterRequest{
		Username: req.Username,
		Nickname: req.Nickname,
		Password: req.Password,
		Avatar:   req.Avatar,
	}

	rpcResp, err := c.Register(s.ctx, &rpcReq) // call rpc
	if err != nil {
		return &Response{
			Status: status.Status{
				Code: status.ErrRegister.Code,
				Msg:  fmt.Sprintf("%v", err),
			},
		}
	}

	return &Response{
		Status: status.Status{
			Code: int(rpcResp.Code),
			Msg:  rpcResp.Message,
		},
	}
}

func (s *UserServiceGrpc) GetProfile(username string) *GetProfileResponse {
	c := pb.NewUserServiceClient(s.rpcClient)
	rpcReq := pb.GetProfileRequest{
		Username: username,
	}

	rpcResp, err := c.GetProfile(s.ctx, &rpcReq) // call rpc
	if err != nil {
		return &GetProfileResponse{
			Status:   status.Status{Code: status.ErrGetProfile.Code, Msg: fmt.Sprintf("%v", err)},
			Username: username,
			Nickname: "",
			Avatar:   "",
		}
	}

	return &GetProfileResponse{
		Status:   status.Status{Code: int(rpcResp.Reply.Code), Msg: rpcResp.Reply.Message},
		Username: rpcResp.Username,
		Nickname: rpcResp.Nickname,
		Avatar:   rpcResp.Avatar,
	}
}

func (s *UserServiceGrpc) UpdateProfile(req *UpdateProfileRequest) *Response {
	c := pb.NewUserServiceClient(s.rpcClient)
	rpcReq := pb.UpdateProfileRequest{
		Username: req.Username,
		Nickname: req.Nickname,
		Avatar:   "", // ignore avatar
	}

	rpcResp, err := c.UpdateProfile(s.ctx, &rpcReq) // call rpc
	if err != nil {
		return &Response{
			Status: status.Status{
				Code: status.ErrUpdateProfile.Code,
				Msg:  fmt.Sprintf("%v", err),
			},
		}
	}

	return &Response{
		Status: status.Status{
			Code: int(rpcResp.Code),
			Msg:  rpcResp.Message,
		},
	}
}

func (s *UserServiceGrpc) UploadAvatar(username string, _ multipart.File, fileHeader *multipart.FileHeader) *UploadAvatarResponse {
	c := pb.NewUserServiceClient(s.rpcClient)
	content, err := utils.GetFileBytes(fileHeader)
	if err != nil {
		return &UploadAvatarResponse{
			Status: status.Status{
				Code: status.ErrUploadAvatar.Code,
				Msg:  fmt.Sprintf("%v", err),
			},
			Avatar: "",
		}
	}
	rpcReq := pb.UploadRequest{
		FileName: fileHeader.Filename,
		Username: username,
		Content:  content,
	}

	rpcResp, err := c.UploadAvatar(s.ctx, &rpcReq) // call rpc
	if err != nil {
		return &UploadAvatarResponse{
			Status: status.Status{
				Code: status.ErrUploadAvatar.Code,
				Msg:  fmt.Sprintf("%v", err),
			},
			Avatar: "",
		}
	}

	return &UploadAvatarResponse{
		Status: status.Status{
			Code: int(rpcResp.Reply.Code),
			Msg:  rpcResp.Reply.Message,
		},
		Avatar: rpcResp.Avatar,
	}
}
