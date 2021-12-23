// MIT License Copyright (c) 2021, h1zzz

package admin

import (
	"errors"
	"fmt"
	"io"
	"log"
	"strings"

	"github.com/chzyer/readline"
)

const (
	adminHostName = "purewater"
)

var (
	// ErrCommandNotFound ...
	ErrCommandNotFound = errors.New("command not found")
)

// Admin ...
type Admin struct {
	user      string
	rw        io.ReadWriteCloser
	rl        *readline.Instance
	viewStack []AdminView
}

// NewAdmin ...
func New(user string, rw io.ReadWriteCloser) (*Admin, error) {
	rl, err := readline.NewEx(&readline.Config{
		Stdin:       rw,
		StdinWriter: rw,
		Stdout:      rw,
		Stderr:      rw,
	})
	if err != nil {
		return nil, err
	}
	return &Admin{
		user: user,
		rw:   rw,
		rl:   rl,
	}, nil
}

// Close ...
func (admin *Admin) Close() {
	admin.rl.Close()
}

// Top ...
func (admin *Admin) TopView() AdminView {
	length := len(admin.viewStack)
	if length == 0 {
		return nil
	}
	return admin.viewStack[length-1]
}

// Push ...
func (admin *Admin) PushView(view AdminView) {
	admin.viewStack = append(admin.viewStack, view)
	admin.RefreshView()
}

// Pop ...
func (admin *Admin) PopView() AdminView {
	length := len(admin.viewStack)
	if length == 0 {
		return nil
	}
	// Get the contents of the top node of the stack
	top := admin.viewStack[length-1]
	// Delete the top node of the stack
	admin.viewStack = admin.viewStack[:length-1]
	if admin.TopView() != nil {
		admin.RefreshView()
	}
	return top
}

// RefreshView ...
func (admin *Admin) RefreshView() {
	names := []string{}
	for _, view := range admin.viewStack {
		names = append(names, view.Name())
	}
	path := strings.Join(names, "/")
	admin.rl.SetPrompt(fmt.Sprintf("\033[32m%s@%s\033[0m:\033[34m%s\033[0m$ ",
		admin.user, adminHostName, path))
	admin.rl.Config.AutoComplete = admin.TopView().Completer()
}

func (admin *Admin) readArgs() ([]string, error) {
	line, err := admin.rl.Readline()
	if err != nil && err != readline.ErrInterrupt {
		log.Print(err)
		return nil, err
	}
	args := strings.Fields(strings.TrimSpace(line))
	if len(args) == 0 {
		return nil, nil
	}
	return args, nil
}

// Handler ...
func (admin *Admin) Handler() error {
	admin.PushView(&AdminMenuView{})
	for {
		args, err := admin.readArgs()
		if err != nil {
			return err
		}
		if len(args) == 0 {
			continue
		}
		// General command
		switch args[0] {
		case "clear":
			admin.rw.Write([]byte("\033[2J\033[0;0H"))
			continue
		case "exit":
			admin.PopView()
			if admin.TopView() == nil {
				return nil
			}
			continue
		}
		// Processing command
		err = admin.TopView().Handler(admin, args)
		if err != nil {
			switch err {
			case ErrCommandNotFound:
				admin.rw.Write([]byte(fmt.Sprintf("command not found: %s", args[0])))
			default:
				return err
			}
		}
	}
}
