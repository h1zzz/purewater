// MIT License Copyright (c) 2021, h1zzz

package admin

import "github.com/chzyer/readline"

// AdminView ...
type AdminView interface {
	// Handler ...
	Handler(admin *Admin, args []string) error
	// Completer ...
	Completer() *readline.PrefixCompleter
	// Usage ...
	Usage() string
	// Name ...
	Name() string
}
