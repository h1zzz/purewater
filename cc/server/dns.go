// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"fmt"
	"log"

	"github.com/miekg/dns"
)

func (s *Server) DNSListen() error {
	mux := dns.NewServeMux()
	mux.HandleFunc(".", DNSHandle)
	s.DNSServer = &dns.Server{
		Addr:    fmt.Sprintf("127.0.0.1:%d", s.Port),
		Net:     "udp",
		Handler: mux,
	}
	go func(s *Server) {
		s.DNSServer.ListenAndServe()
		s.setOffline()
	}(s)
	return nil
}

func DNSHandle(w dns.ResponseWriter, m *dns.Msg) {
	log.Printf("DNS Handle Recvfrom: %s", w.RemoteAddr().String())
}
