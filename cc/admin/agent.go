// MIT License Copyright (c) 2021, h1zzz

package admin

import (
	"fmt"

	"github.com/chzyer/readline"
	"github.com/h1zzz/purewater/cc/database"
)

// AdminAgentView ...
type AdminAgentView struct {
	agent *database.Agent
}

// Handler ...
func (view *AdminAgentView) Handler(admin *Admin, args []string) error {
	switch args[0] {
	case "new":
	case "delete":
	case "list":
	case "info":
	case "interact":
		view.agent = &database.Agent{Host: "192.168.1.1"}
		admin.RefreshView()
	case "shell":
	case "exec":
	case "upload":
	case "download":
	case "proxy":
	case "histroy":
	case "help":
	default:
		return ErrCommandNotFound
	}
	return nil
}

// Completer ...
func (view *AdminAgentView) Completer() *readline.PrefixCompleter {
	return nil
}

// Usage ...
func (view *AdminAgentView) Usage() string {
	return ""
}

// Name ...
func (view *AdminAgentView) Name() string {
	name := "agent"
	if view.agent != nil {
		name += fmt.Sprintf("\033[90m[%s]\033[0m", view.agent.Host)
	}
	return name
}
