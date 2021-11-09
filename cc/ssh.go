// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"fmt"
	"io"
	"log"
	"os"
	"path"
	"strings"
	"time"

	"github.com/chzyer/readline"
	"github.com/gliderlabs/ssh"
)

func usage(rw io.ReadWriter) {
	io.WriteString(rw, "usage: \n")
}

func console(s ssh.Session, args []string) {
	switch args[0] {
	case "h", "help", "?":
		usage(s)
	case "clear", "cls":
		io.WriteString(s, "\033[2J\033[0;0H")
	default:
		// TODO: Can go to execute system commands
		io.WriteString(s, fmt.Sprintf("command not found: %s\n", args[0]))
	}
}

var (
	sshLastLoginFrom = ""
	sshLastLoginTime = ""
)

func sshWelcome(s ssh.Session) {
	io.WriteString(s, "Welcome to Purewater <https://github.com/h1zzz/purewater>\n")
	io.WriteString(s, "  ______                             _\n")
	io.WriteString(s, "  | ___ \\                           | |\n")
	io.WriteString(s, "  | |_/ /   _ _ __ _____      ____ _| |_ ___ _ __\n")
	io.WriteString(s, "  |  __/ | | | '__/ _ \\ \\ /\\ / / _` | __/ _ \\ '__|\n")
	io.WriteString(s, "  | |  | |_| | | |  __/\\ V  V / (_| | ||  __/ |\n")
	io.WriteString(s, "  \\_|   \\__,_|_|  \\___| \\_/\\_/ \\__,_|\\__\\___|_|\n")
	io.WriteString(s, "\n")
	if sshLastLoginFrom != "" && sshLastLoginTime != "" {
		io.WriteString(s, fmt.Sprintf("Last login: %s from %s\n", sshLastLoginTime, sshLastLoginFrom))
	}
}

func sshPasswordAuth(ctx ssh.Context, password string) bool {
	remote := strings.Split(ctx.RemoteAddr().String(), ":")[0]
	log.Printf("[ssh] new connection from %s (%s) try login", remote, ctx.ClientVersion())
	if ctx.User() == sshUser && password == sshPasswd {
		log.Printf("[ssh] connection %s (%s) login success", remote, ctx.User())
		return true
	}
	log.Printf("[ssh] connection %s (%s:%s) login failure", remote, ctx.User(), password)
	return false
}

func sshHandler(s ssh.Session) {
	remote := strings.Split(s.RemoteAddr().String(), ":")[0]
	sshWelcome(s)
	// Record the information of successful login
	sshLastLoginFrom = remote
	sshLastLoginTime = time.Now().Format("Mon Jan 2 15:04:05 MST 2006")
	homeDir, err := os.UserHomeDir()
	if err != nil {
		log.Print(err)
		return
	}
	conf := &readline.Config{
		Prompt:            "\033[4mpw\033[0m > ",
		HistoryFile:       path.Join(homeDir, ".pw_history"),
		HistorySearchFold: true,
		Stdin:             s,
		StdinWriter:       s,
		Stdout:            s,
		Stderr:            s,
	}
	rline, err := readline.NewEx(conf)
	if err != nil {
		log.Print(err)
		return
	}
	defer rline.Close()
	for {
		line, err := rline.Readline()
		if err != nil && err != readline.ErrInterrupt {
			break
		}
		if line == "exit" {
			io.WriteString(s, "logout\n")
			break
		}
		args := strings.Fields(strings.TrimSpace(line))
		if len(args) != 0 {
			console(s, args)
		}
	}
	log.Printf("[ssh] connection %s (%s) logout", remote, s.User())
}
