// MIT License Copyright (c) 2021, h1zzz

package main

import (
	"fmt"
	"log"
	"net/http"

	"github.com/gorilla/websocket"
)

// WebSocketConn ...
type WebSocketConn struct {
	*websocket.Conn
}

// WebSocketUpgrade ...
func WebSocketUpgrade(rw http.ResponseWriter, r *http.Request) (*WebSocketConn, error) {
	conn, err := websocket.Upgrade(rw, r, nil, 4096, 4096)
	if err != nil {
		return nil, err
	}
	return &WebSocketConn{conn}, nil
}

// Read ...
func (ws *WebSocketConn) Read(p []byte) (n int, err error) {
	// Only receive messages of type Text
	messageType, r, err := ws.NextReader()
	if err != nil {
		return 0, err
	}
	switch messageType {
	case websocket.TextMessage:
		return r.Read(p)
		/* ... */
	default:
		// No non-TEXT type message is used in the communication.
		// If a non-TEXT type message is received, the connection should be disconnected
		log.Printf("[warn] websocket message type error")
		return 0, fmt.Errorf("unsupported protocol type")
	}
}

// Write ...
func (ws *WebSocketConn) Write(p []byte) (n int, err error) {
	w, err := ws.NextWriter(websocket.TextMessage)
	if err != nil {
		return 0, err
	}
	n, err = w.Write(p)
	if err != nil {
		return 0, err
	}
	if err := w.Close(); err != nil {
		return 0, err
	}
	return n, nil
}
