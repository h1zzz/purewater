// MIT License Copyright (c) 2022, h1zzz

package api

import (
	"fmt"
	"net/http"
	"sync"

	"github.com/gin-gonic/gin"
)

var (
	serverMap = sync.Map{}
)

// RegisterServer ...
func RegisterServer(r *gin.RouterGroup) {
	r.POST("/start", startServer)
	r.POST("/stop", startServer)
}

type startServerParam struct {
	Protocol string `json:"protocol"`
	Port     int    `json:"port"`
}

func startServer(c *gin.Context) {
	params := startServerParam{}

	if err := c.ShouldBindJSON(params); err != nil {
		ApiReply(c, http.StatusBadRequest, -1, err.Error(), nil)
		return
	}

	if params.Port <= 0 || params.Port >= 65535 {
		ApiReply(c, http.StatusBadRequest, -1, fmt.Sprintf("Invalid port: %d", params.Port), nil)
		return
	}

	if _, ok := serverMap.Load(params.Port); ok {
		ApiReply(c, http.StatusBadRequest, -1, fmt.Sprintf("Port occupation: %d", params.Port), nil)
		return
	}

	ApiReply(c, http.StatusOK, 0, "", nil)
}

func stopServer(c *gin.Context) {
}
