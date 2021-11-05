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
	// for {
	// 	t, data, err := conn.ReadMessage()
	// 	if err != nil {
	// 		log.Print(err)
	// 		return
	// 	}
	// 	conn.WriteMessage(t, data)
	// }
	conn.WriteMessage(websocket.TextMessage, []byte("hello world"))
}

func serverRun(addr string) error {
	http.HandleFunc("/ws", handleDevice)
	// http.HandleFunc("/api", nil)
	return http.ListenAndServeTLS(addr, "cert.pem", "key.pem", nil)
}
