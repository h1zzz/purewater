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

	go serverRun("127.0.0.1:8080")

	options := []ssh.Option{
		ssh.HostKeyFile("key.pem"),
		ssh.PasswordAuth(sshPasswordAuth),
	}

	log.Printf("start ssh server: %s", sshAddr)
	log.Fatal(ssh.ListenAndServe(sshAddr, sshHandler, options...))
}
