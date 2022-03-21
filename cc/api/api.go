// MIT License Copyright (c) 2022, h1zzz

package api

import "github.com/gin-gonic/gin"

// ApiBaseResponse ...
type ApiBaseResponse struct {
	Code    int         `json:"code"`
	Err     string      `json:"err"`
	Content interface{} `json:"content"`
}

// Register ...
func Register(r *gin.RouterGroup) {
	RegisterServer(r.Group("/server"))
}

// ApiReply ...
func ApiReply(c *gin.Context, httpStatus, code int, err string, content interface{}) {
	c.JSON(httpStatus, ApiBaseResponse{
		Code:    code,
		Err:     err,
		Content: content,
	})
}
