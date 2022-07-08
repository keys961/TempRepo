package handler

import (
	"github.com/gin-gonic/gin"
	"net/http"
)

type Ping struct{}

func NewPing() Ping {
	return Ping{}
}

func (p *Ping) Ping(c *gin.Context) {
	c.JSON(http.StatusOK, gin.H{"msg": "pong"})
}
