// MIT License Copyright (c) 2022, h1zzz

package api

import (
	"fmt"
	"log"
	"net/http"
	"sync"

	"github.com/gin-gonic/gin"
	"github.com/h1zzz/purewater/cc/server"
)

var (
	ServersMap sync.Map
)

// RegisterServer ...
func RegisterServer(r *gin.RouterGroup) {
	r.POST("/start", ServerStart)
	r.POST("/stop", ServerStop)
	r.GET("/info", ServerInfo)
	r.GET("/list", ServerList)
}

type ServerParam struct {
	Protocol string `json:"protocol"`
	Port     int    `json:"port"`
}

func ServerStart(c *gin.Context) {
	var params ServerParam

	if err := c.ShouldBindJSON(&params); err != nil {
		log.Print(err)
		APIReply(c, http.StatusBadRequest, -1, err.Error(), nil)
		return
	}

	if params.Port <= 0 || params.Port >= 65535 {
		APIReply(c, http.StatusBadRequest, -1, fmt.Sprintf("invalid port: %d", params.Port), nil)
		return
	}

	if _, ok := ServersMap.Load(params.Port); ok {
		APIReply(c, http.StatusBadRequest, -1, fmt.Sprintf("port occupation: %d", params.Port), nil)
		return
	}

	l, err := server.Start(params.Protocol, params.Port)
	if err != nil {
		log.Print(err)
		APIReply(c, http.StatusBadRequest, -1, err.Error(), nil)
		return
	}

	ServersMap.Store(params.Port, l)

	APIReply(c, http.StatusOK, 0, "", nil)
}

func ServerStop(c *gin.Context) {
	var params ServerParam

	if err := c.ShouldBindJSON(&params); err != nil {
		log.Print(err)
		APIReply(c, http.StatusBadRequest, -1, err.Error(), nil)
		return
	}

	if params.Port <= 0 || params.Port >= 65535 {
		APIReply(c, http.StatusBadRequest, -1, fmt.Sprintf("invalid port: %d", params.Port), nil)
		return
	}

	if v, ok := ServersMap.Load(params.Port); ok {
		v.(*server.Server).Stop()
		ServersMap.Delete(params.Port)
		APIReply(c, http.StatusOK, 0, "", nil)
		return
	}

	APIReply(c, http.StatusBadRequest, -1, fmt.Sprintf("listener does not exist, %d", params.Port), nil)
}

func ServerInfo(c *gin.Context) {
	var params ServerParam

	if err := c.ShouldBindJSON(&params); err != nil {
		log.Print(err)
		APIReply(c, http.StatusBadRequest, -1, err.Error(), nil)
		return
	}

	if params.Port <= 0 || params.Port >= 65535 {
		APIReply(c, http.StatusBadRequest, -1, fmt.Sprintf("invalid port: %d", params.Port), nil)
		return
	}

	if v, ok := ServersMap.Load(params.Port); ok {
		s := v.(*server.Server)
		ServersMap.Delete(params.Port)
		APIReply(c, http.StatusOK, 0, "", gin.H{"protocol": s.Protocol, "port": s.Port})
		return
	}

	APIReply(c, http.StatusBadRequest, -1, fmt.Sprintf("port not in use: %d", params.Port), nil)
}

func ServerList(c *gin.Context) {
	var list []server.Server
	ServersMap.Range(func(k, v interface{}) bool {
		list = append(list, *(v.(*server.Server)))
		return true
	})
	APIReply(c, http.StatusOK, 0, "", list)
}
