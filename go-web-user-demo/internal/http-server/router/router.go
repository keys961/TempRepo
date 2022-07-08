package router

import (
	"entry-task/internal/http-server/router/handler"
	"entry-task/internal/http-server/service"
	"entry-task/internal/pkg/cfg"
	"entry-task/internal/pkg/status"
	"github.com/gin-contrib/sessions"
	"github.com/gin-contrib/sessions/redis"
	"github.com/gin-gonic/gin"
)

const sessionKeyPrefix = "session:"

func NewRouter(config *cfg.RedisConfig, service service.UserService) *gin.Engine {
	ping := handler.NewPing()
	user := handler.NewUser(service)

	r := gin.New()

	r.Static("/static", "./static")
	r.LoadHTMLGlob("templates/*")

	store, _ := redis.NewStore(10, "tcp", config.Host, "", []byte("secret"))
	_ = redis.SetKeyPrefix(store, sessionKeyPrefix)
	// TODO: May move it to TCP server's end
	r.Use(sessions.Sessions("user-session", store))
	{
		r.GET("/", user.IndexTemplate)
		r.GET("/ping", ping.Ping)
		userGroup := r.Group("/user/")
		{
			userGroup.GET("/login", user.LoginTemplate)
			userGroup.POST("/login", user.Login)
			userGroup.GET("/register", user.RegisterTemplate)
			userGroup.POST("/register", user.Register)
		}

		userSessionGroup := r.Group("/user/")
		userSessionGroup.Use(sessionCheck)
		{
			userGroup.GET("/logout", user.Logout)
		}

		profileGroup := r.Group("/user/profile/")
		profileGroup.Use(sessionCheck)
		{
			profileGroup.GET("/", user.ProfileTemplate)
			profileGroup.GET("/get", user.GetProfile)
			profileGroup.POST("/update", user.UpdateProfile)
			profileGroup.POST("/avatar", user.UploadAvatar)
		}
	}

	return r
}

func sessionCheck(c *gin.Context) {
	session := sessions.Default(c)
	// store session id -> username
	v := session.Get(handler.SessionKey)
	if v == nil {
		s := status.ErrSessionNotFound
		c.AbortWithStatusJSON(s.HttpStatusCode(), s)
	} else {
		c.Set("username", v)
		c.Next()
	}
}
