// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"os"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/joho/godotenv"
	"github.com/sirupsen/logrus"
)

var (
	StartWithDockerCompose = false
)

func main() {
	logrus.SetFormatter(&logrus.TextFormatter{FullTimestamp: true})

	if os.Getenv("DOCKER_COMPOSE") == "true" {
		StartWithDockerCompose = true
	}

	// If it is not started with docker-compose, you need to read the
	// environment variables from the .env file
	if !StartWithDockerCompose {
		if err := godotenv.Load(); err != nil {
			logrus.Fatal(err)
		}
	}

	for {
		if err := initMySqlDatabase(); err != nil {
			// If you use docker-compose to start, and it is the first time to
			// start, then the database may not be initialized successfully,
			// you need to wait a while and try again
			if StartWithDockerCompose {
				time.Sleep(time.Second * 5)
				logrus.Info("Init mysql database try again.")
				continue
			}
			logrus.Fatal(err)
		}
		logrus.Info("Init mysql database success.")
		break
	}

	logrus.Fatal(startListen())
}

func startListen() error {
	r := gin.Default()

	cc := r.Group("/cc")
	cc.POST("/helloworld", ccHelloWorld)

	api := r.Group("/api")
	api.GET("/helloworld", apiHelloWorld)

	return r.Run(os.Getenv("LISTEN_ADDR"))
}
