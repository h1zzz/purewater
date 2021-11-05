// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"fmt"
	"io"
	"log"
	"os"
	"path"
	"strings"

	"github.com/chzyer/readline"
	"github.com/gliderlabs/ssh"
)

func sshHandle(s ssh.Session) {
	defer log.Printf("[ssh] connection %s (%s) logout", s.RemoteAddr().String(), s.User())
	log.Printf("[ssh] connection %s (%s) login success", s.RemoteAddr().String(), s.User())
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
			break
		}
		line = strings.TrimSpace(line)
		args := strings.Fields(line)
		if len(args) != 0 {
			consoleHandle(s, args)
		}
	}
}

func consoleUsage(rw io.ReadWriter) {
	io.WriteString(rw, "usage: \n")
}

func consoleHandle(s ssh.Session, args []string) {
	switch args[0] {
	case "h", "help", "?":
		consoleUsage(s)
	case "clear", "cls":
		io.WriteString(s, "\033[2J\033[0;0H")
	default:
		io.WriteString(s, fmt.Sprintf("command not found: %s\n", args[0]))
	}
}
