// MIT License Copyright (c) 2021, h1zzz

package database

type Agent struct {
	ID      string `json:"id" gorm:"id"`
	Host    string `json:"host" gorm:"host"`
	Address string `json:"address" gorm:"address"`
}
