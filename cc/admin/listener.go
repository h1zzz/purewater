// MIT License Copyright (c) 2021, h1zzz

package admin

import "github.com/chzyer/readline"

// AdminListenerView ...
type AdminListenerView struct{}

// Handler ...
func (view *AdminListenerView) Handler(admin *Admin, args []string) error {
	switch args[0] {
	case "new":
	case "delete":
	case "list":
	case "info":
	default:
		return ErrCommandNotFound
	}
	return nil
}

// Complete ...
func (view *AdminListenerView) Completer() *readline.PrefixCompleter {
	return nil
}

// Usage ...
func (view *AdminListenerView) Usage() string {
	return ""
}

// Name ...
func (view *AdminListenerView) Name() string {
	return "listener"
}
