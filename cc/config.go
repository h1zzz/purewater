package main

import (
	"os"

	"github.com/joho/godotenv"
)

var (
	LISTEN_ADDR         string
	MYSQL_ROOT_PASSWORD string
	MYSQL_ADDR          string
	MYSQL_DATABASE      string
	MYSQL_USER          string
	MYSQL_PASSWORD      string
	// MINIO_ADDR          string
	// MINIO_ROOT_USER     string
	// MINIO_ROOT_PASSWORD string
	// REDIS_ADDR          string
)

func InitConfig() error {
	// If it is not started with docker-compose, you need to read the
	// environment variables from the .env file
	if os.Getenv("DOCKER_COMPOSE") != "true" {
		if err := godotenv.Load(); err != nil {
			return err
		}
	}

	LISTEN_ADDR = os.Getenv("LISTEN_ADDR")
	MYSQL_ROOT_PASSWORD = os.Getenv("MYSQL_ROOT_PASSWORD")
	MYSQL_ADDR = os.Getenv("MYSQL_ADDR")
	MYSQL_DATABASE = os.Getenv("MYSQL_DATABASE")
	MYSQL_USER = os.Getenv("MYSQL_USER")
	MYSQL_PASSWORD = os.Getenv("MYSQL_PASSWORD")
	// MINIO_ADDR = os.Getenv("MINIO_ADDR")
	// MINIO_ROOT_USER = os.Getenv("MINIO_ROOT_USER")
	// MINIO_ROOT_PASSWORD = os.Getenv("MINIO_ROOT_PASSWORD")
	// REDIS_ADDR = os.Getenv("REDIS_ADDR")

	return nil
}
