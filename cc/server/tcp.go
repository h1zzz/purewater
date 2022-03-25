// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"fmt"
	"log"
	"net"
)

func TCPListen(port int) (net.Listener, error) {
	l, err := net.Listen("tcp", fmt.Sprintf("127.0.0.1:%d", port))
	if err != nil {
		log.Print(err)
		return nil, err
	}
	go func(l net.Listener) {
		defer l.Close()
		for {
			conn, err := l.Accept()
			if err != nil {
				log.Print(err)
				break
			}
			go TCPHandle(conn)
		}
	}(l)
	return l, nil
}

func TCPHandle(conn net.Conn) {
	defer conn.Close()
	log.Printf("recvfrom: %s", conn.RemoteAddr().String())
}
