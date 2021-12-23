// MIT License Copyright (c) 2021, h1zzz

package database

import (
	"fmt"
	"log"
	"time"

	"gorm.io/driver/mysql"
	"gorm.io/gorm"
)

// Agent ...
type Agent struct {
	// ID agent id
	ID int `gorm:"id"`
	// IP address connected to C2
	Host string `gorm:"host"`
	// Agent status, 1 online and 0 offline
	Status int `gorm:"status"`
	// Device architecture information
	Arch string `gorm:"arch"`
	// Operating system
	OS string `gorm:"os"`
	// Agent version
	Version string `gorm:"version"`
	// Device host name
	Hostname string `gorm:"hostname"`
	// Time to connect to C2
	Time time.Time `gorm:"time"`
}

// Device ...
type Device struct {
	ID       int       `gorm:"id"`
	Host     string    `gorm:"host"`     // IP address connected to C2
	Status   int       `gorm:"status"`   // Agent status, 1 online and 0 offline
	Arch     string    `gorm:"arch"`     // Device architecture information
	OS       string    `gorm:"os"`       // Operating system
	Version  string    `gorm:"version"`  // Agent version
	Hostname string    `gorm:"hostname"` // Device host name
	Time     time.Time `gorm:"time"`     // Time to connect to C2
}

// Address ...
type Address struct {
	ID       int    `gorm:"id"`
	DeviceID int    `gorm:"device_id"`
	Name     string `gorm:"name"` // Network card name
	IP       string `gorm:"ip"`   // IP address
	Mac      string `gorm:"mac"`  // Mac address of the network card
}

// Task ...
type Task struct {
	ID       int       `gorm:"id"`
	DeviceID int       `gorm:"device_id"`
	Type     string    `gorm:"type"`   // Task type
	Param    string    `gorm:"param"`  // Task parameters
	Status   int       `gorm:"status"` // Task execution status, 0 is executed successfully, 1 is executing, and 2 has failed to execute
	Result   string    `gorm:"result"` // Result of task execution
	Time     time.Time `gorm:"time"`   // Task execution time
}

// Satus ...
type Status struct {
	ID          int    `gorm:"id"`
	DeviceID    int    `gorm:"device_id"`
	ServiceName string `gorm:"service_name"` // Service name
	Status      int    `gorm:"status"`       // Service running status
}

// Database ...
type Database struct {
	sql *gorm.DB
}

// NewDatabase ...
func NewDatabase(user, passwd, addr, dbName string) (*Database, error) {
	dsn := fmt.Sprintf("%s:%s@tcp(%s)/%s?charset=utf8mb4&parseTime=True&loc=Local", user, passwd, addr, dbName)
	sql, err := gorm.Open(mysql.Open(dsn), &gorm.Config{})
	if err != nil {
		log.Print(err)
		return nil, err
	}
	return &Database{
		sql: sql,
	}, nil
}
