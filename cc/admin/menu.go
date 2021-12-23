// MIT License Copyright (c) 2021, h1zzz

package admin

import "github.com/chzyer/readline"

// AdminMenuView ...
type AdminMenuView struct{}

// Handler ...
func (view *AdminMenuView) Handler(admin *Admin, args []string) error {
	switch args[0] {
	case "listener":
		admin.PushView(&AdminListenerView{})
	case "agent":
		admin.PushView(&AdminAgentView{})
	default:
		return ErrCommandNotFound
	}
	return nil
}

// Complete ...
func (view *AdminMenuView) Completer() *readline.PrefixCompleter {
	return nil
}

// Usage ...
func (view *AdminMenuView) Usage() string {
	return ""
}

// Name ...
func (view *AdminMenuView) Name() string {
	return "~"
}
