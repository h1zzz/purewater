// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"fmt"
	"log"
	"net/http"
)

func handleDevice(rw http.ResponseWriter, r *http.Request) {
	log.Printf("[server] new connection from %s", r.RemoteAddr)
	conn, err := WebSocketUpgrade(rw, r)
	if err != nil {
		log.Print(err)
		return
	}
	defer conn.Close()
	// Handshake
	buf := make([]byte, 32)
	conn.Read(buf)
	fmt.Println(string(buf))
	conn.Write([]byte("server: helloworld"))
}

// ServerRun ...
func ServerRun(addr string) error {
	log.Printf("start server: %s", addr)
	http.HandleFunc("/ws", handleDevice)
	return http.ListenAndServeTLS(addr, "cert.pem", "key.pem", nil)
}
