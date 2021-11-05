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

	go func() {
		options := []ssh.Option{
			ssh.HostKeyFile("key.pem"),
			ssh.PasswordAuth(func(ctx ssh.Context, password string) bool {
				log.Printf("[ssh] new connection from %s (%s) try login", ctx.RemoteAddr().String(), ctx.ClientVersion())
				return (ctx.User() == sshUser && password == sshPasswd)
			}),
		}
		log.Print("start ssh server")
		log.Fatal(ssh.ListenAndServe(sshAddr, sshHandle, options...))
	}()

	log.Print("start server")
	log.Fatal(serverRun(serverAddr))
}
