// MIT License Copyright (c) 2022, h1zzz

package api

import "github.com/gin-gonic/gin"

// APIBaseResponse ...
type APIBaseResponse struct {
	Ret     int         `json:"ret"`
	Msg     string      `json:"msg"`
	Content interface{} `json:"content"`
}

// Register ...
func Register(r *gin.RouterGroup) {
	RegisterServer(r.Group("/server"))
}

func APIReply(c *gin.Context, httpStatus, ret int, err string, content interface{}) {
	c.JSON(httpStatus, APIBaseResponse{Ret: ret, Msg: err, Content: content})
}
