// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"fmt"
	"log"
	"net"
)

func tcpListen(port int) error {
	l, err := net.Listen("tcp", fmt.Sprintf("127.0.0.1:%d", port))
	if err != nil {
		log.Print(err)
		return err
	}
	go func(l net.Listener) {
		defer l.Close()
		for {
			conn, err := l.Accept()
			if err != nil {
				log.Print(err)
				break
			}
			go tcpHandle(conn)
		}
	}(l)
	return nil
}

func tcpHandle(conn net.Conn) {
	defer conn.Close()
	log.Print("hello world")
}
