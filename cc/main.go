// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"fmt"
	"log"
	"os"

	"github.com/gin-gonic/gin"
	"github.com/h1zzz/purewater/cc/api"
	"github.com/joho/godotenv"
)

const (
	welcomeText = "" +
		"Welcome to Purewater <https://github.com/h1zzz/purewater>\n" +
		"  ______                             _\n" +
		"  | ___ \\                           | |\n" +
		"  | |_/ /   _ _ __ _____      ____ _| |_ ___ _ __\n" +
		"  |  __/ | | | '__/ _ \\ \\ /\\ / / _` | __/ _ \\ '__|\n" +
		"  | |  | |_| | | |  __/\\ V  V / (_| | ||  __/ |\n" +
		"  \\_|   \\__,_|_|  \\___| \\_/\\_/ \\__,_|\\__\\___|_|\n" +
		"\n"
)

func init() {
	log.SetFlags(log.LstdFlags | log.Lshortfile)
	gin.SetMode(gin.ReleaseMode)

	if err := godotenv.Load(); err != nil && !os.IsNotExist(err) {
		log.Fatal(err)
	}
}

func main() {
	log.Print(welcomeText)

	// err := database.InitDatabase(os.Getenv("MYSQL_USER"), os.Getenv("MYSQL_PASSWORD"), os.Getenv("MYSQL_ADDR"),
	//     os.Getenv("MYSQL_DATABASE"))
	// if err != nil {
	//     log.Fatal(err)
	// }

	router := gin.Default()
	api.Register(router.Group("/api"))

	log.Fatal(router.RunTLS(fmt.Sprintf("0.0.0.0:%s", os.Getenv("API_LISTEN_PORT")), "cert.pem", "key.pem"))
}
