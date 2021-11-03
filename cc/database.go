package main

import (
	"database/sql"
	"fmt"
	"time"

	_ "github.com/go-sql-driver/mysql"
)

type Device struct {
	ID       int
	Host     string
	Status   int
	Arch     string
	OS       string
	Version  string
	Hostname string
	Time     time.Time
}

type Address struct {
	ID       int
	DeviceID int
	IP       string
	Mac      string
}

type Task struct {
	ID       int
	DeviceID int
	Type     string
	Task     string
	Result   string
	Time     time.Time
}

type TaskStatus struct {
	ID       int
	DeviceID int
	TaskID   int
	Status   int
}

type Database struct {
	sql *sql.DB
}

func initMysqlDb() (*sql.DB, error) {
	dsn := fmt.Sprintf("%s:%s@tcp(%s)/%s?charset=utf8mb4&parseTime=True&loc=Local", MYSQL_USER, MYSQL_PASSWORD, MYSQL_ADDR, MYSQL_DATABASE)
	db, err := sql.Open("mysql", dsn)
	if err != nil {
		return nil, err
	}
	return db, nil
}

func NewDatabase() (*Database, error) {
	sql, err := initMysqlDb()
	if err != nil {
		return nil, err
	}
	return &Database{sql: sql}, nil
}

type status_t uint8

const (
	StatusOffline = status_t(iota)
	StatusOnline
)

func (db *Database) SetDeviceStatus(deviceID int, status status_t) error { return nil }
