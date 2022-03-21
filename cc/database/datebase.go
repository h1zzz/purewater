// MIT License Copyright (c) 2021, h1zzz

package database

import (
	"fmt"

	"gorm.io/driver/mysql"
	"gorm.io/gorm"
)

var (
	sql *gorm.DB
)

// InitDatabase ...
func InitDatabase(user, password, addr, dbName string) (err error) {
	dsn := fmt.Sprintf("%s:%s@tcp(%s)/%s?charset=utf8mb4&parseTime=True&loc=Local",
		user, password, addr, dbName)
	sql, err = gorm.Open(mysql.Open(dsn), &gorm.Config{})
	if err != nil {
		return err
	}
	return nil
}
