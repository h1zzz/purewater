// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"log"
	"os"

	"github.com/joho/godotenv"
)

var envLoaded = false

// GetEnv ...
func GetEnv(key string) string {
	if !envLoaded {
		if err := godotenv.Load(); err != nil && !os.IsNotExist(err) {
			log.Fatal(err)
		}
		envLoaded = true
	}
	return os.Getenv(key)
}
