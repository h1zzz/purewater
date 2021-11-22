// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"io/ioutil"
	"log"
	"os"
	"path"

	"github.com/gliderlabs/ssh"
)

func main() {
	// err := databaseInit(mysqlUser, mysqlPasswd, mysqlAddr, mysqlDbName)
	// if err != nil {
	// 	log.Fatal(err)
	// }
	// log.Print("database init success")

	go serverRun("127.0.0.1:8080")

	options := []ssh.Option{ssh.HostKeyFile("key.pem")}

	// If the user name and password are not empty, then add the user name and password authentication method
	if sshUser != "" && sshPasswd != "" {
		options = append(options, ssh.PasswordAuth(sshPasswordAuth))
	}

	// If there is a public key, add a way to log in with the public key
	homeDir, err := os.UserHomeDir()
	if err != nil {
		log.Panic(err)
	}

	keyPath := path.Join(homeDir, ".ssh/authorized_keys")

	exists, err := pathExists(keyPath)
	if err != nil {
		log.Panic(err)
	}

	if exists {
		options = append(options, ssh.PublicKeyAuth(func(ctx ssh.Context, key ssh.PublicKey) bool {
			data, err := ioutil.ReadFile(keyPath)
			if err != nil {
				log.Print(err)
				return false
			}
			out, _, _, _, err := ssh.ParseAuthorizedKey(data)
			if err != nil {
				log.Print(err)
				return false
			}
			return ssh.KeysEqual(key, out)
		}))
	}

	log.Printf("start ssh server: %s", sshAddr)
	log.Fatal(ssh.ListenAndServe(sshAddr, sshHandler, options...))
}
