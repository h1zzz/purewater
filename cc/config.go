// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"log"
	"os"

	"github.com/joho/godotenv"
)

var (
	serverAddr  = getenv("SERVER_ADDR", ":443")
	sshAddr     = getenv("SSH_ADDR", ":22")
	sshUser     = getenv("SSH_USER", "admin")
	sshPasswd   = getenv("SSH_PASSWORD", "password")
	mysqlAddr   = getenv("MYSQL_ADDR", "mysql:3306")
	mysqlDbName = getenv("MYSQL_DATABASE", "cc")
	mysqlUser   = getenv("MYSQL_USER", "cc")
	mysqlPasswd = getenv("MYSQL_PASSWORD", "password")
)

var envLoaded = false

func getenv(key, defval string) string {
	if !envLoaded {
		if err := godotenv.Load(); err != nil && !os.IsNotExist(err) {
			log.Fatal(err)
		}
		envLoaded = true
	}
	val := os.Getenv(key)
	if val == "" {
		return defval
	}
	return val
}
