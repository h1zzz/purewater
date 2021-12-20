// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path"
	"time"

	"github.com/gliderlabs/ssh"
)

const (
	sshWelcome = "" +
		"Welcome to Purewater <https://github.com/h1zzz/purewater>\n" +
		"  ______                             _\n" +
		"  | ___ \\                           | |\n" +
		"  | |_/ /   _ _ __ _____      ____ _| |_ ___ _ __\n" +
		"  |  __/ | | | '__/ _ \\ \\ /\\ / / _` | __/ _ \\ '__|\n" +
		"  | |  | |_| | | |  __/\\ V  V / (_| | ||  __/ |\n" +
		"  \\_|   \\__,_|_|  \\___| \\_/\\_/ \\__,_|\\__\\___|_|\n" +
		"\n"
)

func main() {

	var (
		sshAddr   = GetEnv("SSH_ADDR")
		sshUser   = GetEnv("SSH_USER")
		sshPasswd = GetEnv("SSH_PASSWORD")
	)

	options := []ssh.Option{ssh.HostKeyFile("key.pem")}

	// If the user name and password are not empty, then add the user name and password authentication method
	if sshUser != "" && sshPasswd != "" {
		options = append(options, ssh.PasswordAuth(func(ctx ssh.Context, password string) bool {
			if ctx.User() == sshUser && password == sshPasswd {
				return true
			}
			return false
		}))
	}

	// If there is a public key, add a way to log in with the public key
	homeDir, err := os.UserHomeDir()
	if err != nil {
		log.Panic(err)
	}

	keyPath := path.Join(homeDir, ".ssh/authorized_keys")
	exists, err := PathExists(keyPath)
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
			// TODO: verify username
			out, _, _, _, err := ssh.ParseAuthorizedKey(data)
			if err != nil {
				log.Print(err)
				return false
			}
			return ssh.KeysEqual(key, out)
		}))
	}

	log.Printf("start ssh server: %s", sshAddr)

	log.Fatal(ssh.ListenAndServe(sshAddr, func(session ssh.Session) {
		defer session.Close()

		// Echo welcome message
		session.Write([]byte(sshWelcome + fmt.Sprintf("%s login from %s.\n",
			time.Now().Format("Mon Jan 2 15:04:05 MST 2006"), session.RemoteAddr().String())))

		admin, err := NewAdmin(session.User(), session)
		if err != nil {
			log.Print(err)
			return
		}
		defer admin.Close()

		if err := admin.Handler(); err != nil {
			log.Print(err)
		}
	}, options...))
}
