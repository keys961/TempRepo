package rpc

import "encoding/gob"

type PingReq struct {
	Message string
}

type LoginReq struct {
	Username string
	Password string
}

type RegisterReq struct {
	Username string
	Password string
	Nickname string
	Avatar   string
}

type UpdateProfileReq struct {
	Username string
	Nickname string
	Avatar   string
}

type GetProfileReq struct {
	Username string
}

type UploadAvatarReq struct {
	Filename string
	Username string
	Content  []byte
}

type Resp struct {
	Code    int32
	Message string
}

type GetProfileResp struct {
	Resp
	Username string
	Nickname string
	Avatar   string
}

type UploadAvatarResp struct {
	Resp
	Avatar string
}

func init() {
	gob.Register(PingReq{})
	gob.Register(LoginReq{})
	gob.Register(RegisterReq{})
	gob.Register(UpdateProfileReq{})
	gob.Register(GetProfileReq{})
	gob.Register(UploadAvatarReq{})
	gob.Register(Resp{})
	gob.Register(GetProfileResp{})
	gob.Register(UploadAvatarResp{})
}
