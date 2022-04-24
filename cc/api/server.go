// MIT License Copyright (c) 2022, h1zzz

package api

import (
	"fmt"
	"log"
	"net/http"
	"strconv"

	"github.com/gin-gonic/gin"
	"github.com/h1zzz/purewater/cc/server"
	"github.com/h1zzz/purewater/cc/util"
)

var (
	Server = &server.Server{}
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

	if !util.ValidPort(params.Port) {
		APIReply(c, http.StatusBadRequest, -1, "Invalid port", nil)
		return
	}

	err := Server.Start(server.ListenerProtocol(params.Protocol), params.Port)
	if err != nil {
		log.Print(err)
		APIReply(c, http.StatusBadRequest, -1, err.Error(), nil)
		return
	}

	APIReply(c, http.StatusOK, 0, "", nil)
}

func ServerStop(c *gin.Context) {
	var params ServerParam
	if err := c.ShouldBindJSON(&params); err != nil {
		log.Print(err)
		APIReply(c, http.StatusBadRequest, -1, err.Error(), nil)
		return
	}

	if !util.ValidPort(params.Port) {
		APIReply(c, http.StatusBadRequest, -1, "Invalid port", nil)
		return
	}

	err := Server.Stop(params.Port)
	if err != nil {
		log.Print(err)
		APIReply(c, http.StatusBadRequest, -1, err.Error(), nil)
		return
	}

	APIReply(c, http.StatusOK, 0, "", nil)
}

func ServerInfo(c *gin.Context) {
	port, err := strconv.Atoi(c.Query("port"))
	if err != nil {
		log.Print(err)
		APIReply(c, http.StatusBadRequest, -1, "invalid port", nil)
		return
	}

	if !util.ValidPort(port) {
		APIReply(c, http.StatusBadRequest, -1, "Invalid port", nil)
		return
	}

	if v, ok := Server.Listeners().Load(port); ok {
		l := v.(server.Listener)
		APIReply(c, http.StatusOK, 0, "", gin.H{
			"protocol": l.Protocol(),
			"port":     l.Port(),
			"comment":  l.Comment(),
			"status":   l.Status(),
		})
		return
	}

	APIReply(c, http.StatusBadRequest, -1, fmt.Sprintf("port not in use: %d", port), nil)
}

func ServerList(c *gin.Context) {
	var list []gin.H
	Server.Listeners().Range(func(k, v interface{}) bool {
		l := v.(server.Listener)
		list = append(list, gin.H{
			"protocol": l.Protocol(),
			"port":     l.Port(),
			"comment":  l.Comment(),
			"status":   l.Status(),
		})
		return true
	})
	APIReply(c, http.StatusOK, 0, "", list)
}
