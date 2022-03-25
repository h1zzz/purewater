// MIT License Copyright (c) 2022, h1zzz

package api

import "github.com/gin-gonic/gin"

// Register ...
func Register(r *gin.RouterGroup) {
	RegisterServer(r.Group("/server"))
}

func APIReply(c *gin.Context, httpStatus, ret int, err string, content interface{}) {
	resp := gin.H{"code": ret, "msg": err}
	if err == "" && ret == 0 {
		resp["msg"] = "success"
	}
	if content != nil {
		resp["content"] = content
	}
	c.JSON(httpStatus, resp)
}
