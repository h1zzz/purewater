// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"fmt"
	"log"
	"net"
	"net/http"

	"github.com/miekg/dns"
)

// Server ...
type Server struct {
	Protocol    string           `json:"protocol"`
	Port        int              `json:"port"`
	Status      int              `json:"status"`
	Comment     string           `json:"comment"`
	TCPListener *net.TCPListener `json:"-"`
	UDPConn     *net.UDPConn     `json:"-"`
	DNSServer   *dns.Server      `json:"-"`
	HTTPServer  *http.Server     `json:"-"`
}

func (s *Server) Stop() {
	log.Printf("Stop listener, protocol: %s, port: %d", s.Protocol, s.Port)
	switch s.Protocol {
	case "tcp":
		s.TCPListener.Close()
	case "udp":
		s.UDPConn.Close()
	case "http":
		s.HTTPServer.Close()
	case "dns":
		s.DNSServer.Shutdown()
	default:
	}
	s.setOffline()
}

func Start(protocol string, port int) (s *Server, err error) {
	s = &Server{Protocol: protocol, Port: port}

	switch protocol {
	case "tcp":
		err = s.TCPListen()
	case "udp":
		err = s.UDPListen()
	case "http":
		err = s.HTTPListen()
	// case "https":
	// TODO: support https
	case "dns":
		err = s.DNSListen()
	default:
		return nil, fmt.Errorf("no support protocol: %s", protocol)
	}

	if err != nil {
		log.Print(err)
		return
	}

	s.setOnline()
	log.Printf("Start listener, protocol: %s, port: %d", protocol, port)

	return
}

func (s *Server) setOffline() {
	s.Status = 0
}

func (s *Server) setOnline() {
	s.Status = 1
}
