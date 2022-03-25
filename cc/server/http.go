// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"fmt"
	"log"
	"net/http"
)

func (s *Server) HTTPListen() (err error) {
	mux := &http.ServeMux{}
	mux.HandleFunc("/", HTTPHandle)
	s.HTTPServer = &http.Server{Addr: fmt.Sprintf("127.0.0.1:%d", s.Port), Handler: mux}
	if err != nil {
		log.Print(err)
		return
	}
	return nil
}

func HTTPHandle(w http.ResponseWriter, r *http.Request) {
	log.Printf("HTTP Handle Recvfrom: %s", r.RemoteAddr)
}
