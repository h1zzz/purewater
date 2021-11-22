// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"log"
	"net/http"

	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
}

func handleDevice(rw http.ResponseWriter, r *http.Request) {
	log.Printf("[server] new connection from %s", r.RemoteAddr)
	conn, err := upgrader.Upgrade(rw, r, nil)
	if err != nil {
		log.Print(err)
		return
	}
	for {
		s := ""
		for i := 0; i < 200; i++ {
			s += "helloworld"
		}
		conn.WriteMessage(websocket.TextMessage, []byte(s))
		conn.WriteMessage(websocket.TextMessage, []byte("hello world"))
		t, data, err := conn.ReadMessage()
		if err != nil {
			log.Print(err)
			return
		}
		log.Printf("recv: %s", string(data))
		conn.WriteMessage(t, data)
	}
}

func serverRun(addr string) error {
	log.Printf("start server: %s", addr)
	http.HandleFunc("/ws", handleDevice)
	return http.ListenAndServeTLS(addr, "cert.pem", "key.pem", nil)
}
