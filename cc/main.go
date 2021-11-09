// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"log"

	"github.com/gliderlabs/ssh"
)

func main() {
	// err := databaseInit(mysqlUser, mysqlPasswd, mysqlAddr, mysqlDbName)
	// if err != nil {
	// 	log.Fatal(err)
	// }
	// log.Print("database init success")

	options := []ssh.Option{
		ssh.HostKeyFile("key.pem"),
		ssh.PasswordAuth(sshPasswordAuth),
	}
	log.Print("start ssh server")
	log.Fatal(ssh.ListenAndServe(sshAddr, sshHandler, options...))
}
