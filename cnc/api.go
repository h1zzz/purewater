// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

func apiHelloWorld(c *gin.Context) {
	c.JSON(http.StatusOK, gin.H{"msg": "hello world"})
}
