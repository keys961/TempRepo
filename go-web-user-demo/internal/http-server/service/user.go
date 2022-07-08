package service

import (
	"context"
	"entry-task/internal/pkg/rpc"
	"entry-task/internal/pkg/status"
	"entry-task/internal/tcp-server/utils"
	"fmt"
	"github.com/silenceper/pool"
	"mime/multipart"
)

type UserService interface {
	Login(req *LoginRequest) *Response
	Register(req *RegisterRequest) *Response
	GetProfile(username string) *GetProfileResponse
	UpdateProfile(req *UpdateProfileRequest) *Response
	UploadAvatar(username string, _ multipart.File, fileHeader *multipart.FileHeader) *UploadAvatarResponse
}

type LoginRequest struct {
	Username string `form:"username" binding:"required,min=6,max=64"`
	Password string `form:"password" binding:"required,min=6,max=64"`
}

type RegisterRequest struct {
	Username string `form:"username" binding:"required,min=6,max=64"`
	Password string `form:"password" binding:"required,min=6,max=64"`
	Nickname string `form:"nickname" binding:"required,min=6,max=64"`
	Avatar   string `form:"avatar" binding:"-"` // Skip validation.
}

type UpdateProfileRequest struct {
	Username string `form:"username" binding:"-"`
	Nickname string `form:"nickname" binding:"min=6,max=64"`
}

type Response struct {
	Status status.Status `json:"status"`
}

type GetProfileResponse struct {
	Status   status.Status `json:"status"`
	Username string        `json:"username"`
	Nickname string        `json:"nickname"`
	Avatar   string        `json:"avatar"`
}

type UploadAvatarResponse struct {
	Status status.Status `json:"status"`
	Avatar string        `json:"avatar"` // avatar url
}

type UserServiceRpc struct {
	ctx        context.Context
	clientPool pool.Pool
}

func NewUserServiceRpc(ctx context.Context, clientPool pool.Pool) *UserServiceRpc {
	return &UserServiceRpc{
		ctx:        ctx,
		clientPool: clientPool,
	}
}

func (s *UserServiceRpc) Login(req *LoginRequest) *Response {
	rpcReq := rpc.LoginReq{
		Username: req.Username,
		Password: req.Password,
	}
	rpcResp := rpc.Resp{}

	c, err := s.clientPool.Get()
	if err != nil {
		return &Response{
			Status: status.Status{
				Code: status.ErrLogin.Code,
				Msg:  fmt.Sprintf("%v", err),
			},
		}
	}
	defer returnClientToPool(s.clientPool, c)
	err = c.(*rpc.Client).Call(s.ctx, "UserService.Login", rpcReq, &rpcResp) // call rpc login
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

func (s *UserServiceRpc) Register(req *RegisterRequest) *Response {
	rpcReq := rpc.RegisterReq{
		Username: req.Username,
		Nickname: req.Nickname,
		Password: req.Password,
		Avatar:   req.Avatar,
	}
	rpcResp := rpc.Resp{}

	c, err := s.clientPool.Get()
	if err != nil {
		return &Response{
			Status: status.Status{
				Code: status.ErrRegister.Code,
				Msg:  fmt.Sprintf("%v", err),
			},
		}
	}
	defer returnClientToPool(s.clientPool, c)
	err = c.(*rpc.Client).Call(s.ctx, "UserService.Register", rpcReq, &rpcResp) // call rpc
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

func (s *UserServiceRpc) GetProfile(username string) *GetProfileResponse {
	rpcReq := rpc.GetProfileReq{
		Username: username,
	}
	rpcResp := rpc.GetProfileResp{}

	c, err := s.clientPool.Get()
	if err != nil {
		return &GetProfileResponse{
			Status:   status.Status{Code: status.ErrGetProfile.Code, Msg: fmt.Sprintf("%v", err)},
			Username: username,
			Nickname: "",
			Avatar:   "",
		}
	}
	defer returnClientToPool(s.clientPool, c)
	err = c.(*rpc.Client).Call(s.ctx, "UserService.GetProfile", rpcReq, &rpcResp) // call rpc
	if err != nil {
		return &GetProfileResponse{
			Status:   status.Status{Code: status.ErrGetProfile.Code, Msg: fmt.Sprintf("%v", err)},
			Username: username,
			Nickname: "",
			Avatar:   "",
		}
	}

	return &GetProfileResponse{
		Status:   status.Status{Code: int(rpcResp.Code), Msg: rpcResp.Message},
		Username: rpcResp.Username,
		Nickname: rpcResp.Nickname,
		Avatar:   rpcResp.Avatar,
	}
}

func (s *UserServiceRpc) UpdateProfile(req *UpdateProfileRequest) *Response {
	rpcReq := rpc.UpdateProfileReq{
		Username: req.Username,
		Nickname: req.Nickname,
		Avatar:   "", // ignore avatar
	}
	rpcResp := rpc.Resp{}
	c, err := s.clientPool.Get()
	if err != nil {
		return &Response{
			Status: status.Status{
				Code: status.ErrUpdateProfile.Code,
				Msg:  fmt.Sprintf("%v", err),
			},
		}
	}
	defer returnClientToPool(s.clientPool, c)
	err = c.(*rpc.Client).Call(s.ctx, "UserService.UpdateProfile", rpcReq, &rpcResp) // call rpc
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

func (s *UserServiceRpc) UploadAvatar(username string, _ multipart.File, fileHeader *multipart.FileHeader) *UploadAvatarResponse {
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
	rpcReq := rpc.UploadAvatarReq{
		Filename: fileHeader.Filename,
		Username: username,
		Content:  content,
	}
	rpcResp := rpc.UploadAvatarResp{}
	c, err := s.clientPool.Get()
	if err != nil {
		return &UploadAvatarResponse{
			Status: status.Status{
				Code: status.ErrUploadAvatar.Code,
				Msg:  fmt.Sprintf("%v", err),
			},
			Avatar: "",
		}
	}
	defer returnClientToPool(s.clientPool, c)
	err = c.(*rpc.Client).Call(s.ctx, "UserService.UploadAvatar", rpcReq, &rpcResp) // call rpc
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
			Code: int(rpcResp.Code),
			Msg:  rpcResp.Message,
		},
		Avatar: rpcResp.Avatar,
	}
}

func returnClientToPool(clientPool pool.Pool, client interface{}) {
	_ = clientPool.Put(client)
}
