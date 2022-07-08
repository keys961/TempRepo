package handler

import (
	"entry-task/internal/http-server/service"
	"entry-task/internal/pkg/status"
	"fmt"
	"github.com/gin-contrib/sessions"
	"github.com/gin-gonic/gin"
	"net/http"
)

const SessionKey = "session-key"

type User struct {
	service service.UserService
}

func NewUser(service service.UserService) User {
	return User{
		service: service,
	}
}

func (u *User) Login(c *gin.Context) {
	param := service.LoginRequest{}
	err := c.ShouldBind(&param)
	if err != nil {
		resp := service.Response{Status: status.Status{
			Code: status.ErrLogin.Code,
			Msg:  fmt.Sprintf("%v", err),
		}}
		c.JSON(status.ErrLogin.HttpStatusCode(), resp)
		return
	}
	resp := u.service.Login(&param)
	if resp.Status.Code == status.Ok.Code {
		session := sessions.Default(c)
		session.Set(SessionKey, param.Username) // set session with username
		// TODO: Session handling may be in TCP server
		err = session.Save()
		if err != nil {
			resp = &service.Response{Status: status.Status{
				Code: status.ErrLogin.Code,
				Msg:  fmt.Sprintf("%v", err),
			}}
			c.JSON(status.ErrLogin.HttpStatusCode(), resp)
			return
		}
	}
	c.JSON(resp.Status.HttpStatusCode(), resp)
}

func (u *User) Register(c *gin.Context) {
	param := service.RegisterRequest{}
	err := c.ShouldBind(&param)
	if err != nil {
		resp := service.Response{Status: status.Status{
			Code: status.ErrRegister.Code,
			Msg:  fmt.Sprintf("%v", err),
		}}
		c.JSON(status.ErrRegister.HttpStatusCode(), resp)
		return
	}
	resp := u.service.Register(&param)
	c.JSON(resp.Status.HttpStatusCode(), resp)
}

func (u *User) UpdateProfile(c *gin.Context) {
	param := service.UpdateProfileRequest{}
	username, _ := c.Get("username")
	err := c.ShouldBind(&param) // ignored avatar
	param.Username = fmt.Sprintf("%v", username)
	if err != nil {
		resp := service.Response{Status: status.Status{
			Code: status.ErrUpdateProfile.Code,
			Msg:  fmt.Sprintf("%v", err),
		}}
		c.JSON(status.ErrUpdateProfile.HttpStatusCode(), resp)
		return
	}
	resp := u.service.UpdateProfile(&param)
	c.JSON(resp.Status.HttpStatusCode(), resp)
}

func (u *User) GetProfile(c *gin.Context) {
	username, _ := c.Get("username")
	resp := u.service.GetProfile(fmt.Sprintf("%v", username))
	c.JSON(resp.Status.HttpStatusCode(), resp)
}

func (u *User) UploadAvatar(c *gin.Context) {
	username, _ := c.Get("username")
	file, fileHeader, err := c.Request.FormFile("file")
	if err != nil {
		resp := service.UploadAvatarResponse{
			Status: status.Status{
				Code: status.ErrUploadAvatar.Code,
				Msg:  fmt.Sprintf("%v", err),
			},
			Avatar: "",
		}
		c.JSON(resp.Status.HttpStatusCode(), resp)
		return
	}

	if fileHeader == nil {
		resp := service.UploadAvatarResponse{
			Status: status.Status{
				Code: status.ErrUploadAvatar.Code,
				Msg:  "file header is empty, invalid param",
			},
			Avatar: "",
		}
		c.JSON(resp.Status.HttpStatusCode(), resp)
		return
	}

	resp := u.service.UploadAvatar(fmt.Sprintf("%v", username), file, fileHeader)
	c.JSON(resp.Status.HttpStatusCode(), resp)
}

func (u *User) Logout(c *gin.Context) {
	session := sessions.Default(c)
	// store session id -> username
	session.Delete(SessionKey)
	session.Clear()
	_ = session.Save()
	c.Redirect(http.StatusMovedPermanently, "/")
}

func (u *User) IndexTemplate(c *gin.Context) {
	c.HTML(http.StatusOK, "home.gohtml", gin.H{
		"title":   "Home",
		"IsLogin": isLogin(c),
	})
}

func (u *User) RegisterTemplate(c *gin.Context) {
	c.HTML(http.StatusOK, "register.gohtml", gin.H{
		"title":   "Register",
		"IsLogin": isLogin(c),
	})
}

func (u *User) LoginTemplate(c *gin.Context) {
	c.HTML(http.StatusOK, "login.gohtml", gin.H{
		"title":   "Login",
		"IsLogin": isLogin(c),
	})
}

func (u *User) ProfileTemplate(c *gin.Context) {
	username, _ := c.Get("username")
	resp := u.service.GetProfile(fmt.Sprintf("%v", username))
	c.HTML(http.StatusOK, "profile.gohtml", gin.H{
		"title":    "Login",
		"IsLogin":  isLogin(c),
		"Username": resp.Username,
		"Nickname": resp.Nickname,
		"Avatar":   resp.Avatar,
	})
}

func isLogin(c *gin.Context) bool {
	session := sessions.Default(c)
	// store session id -> username
	v := session.Get(SessionKey)
	return v != nil
}
