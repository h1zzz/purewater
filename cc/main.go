// MIT License Copyright (c) 2021, h1zzz

package main

import "log"

func main() {

	if err := InitConfig(); err != nil {
		log.Panic(err)
	}

}
