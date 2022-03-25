// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"fmt"
	"log"
	"net"
)

// Server ...
type Server struct {
	Protocol string       `json:"protocol"`
	Port     int          `json:"port"`
	Listen   net.Listener `json:"-"`
}

func (s *Server) Stop() {
	log.Printf("Stop listener, protocol: %s, port: %d", s.Protocol, s.Port)

	switch s.Protocol {
	case "tcp":
		s.Listen.Close()
	case "udp":
	case "http":
	case "dns":
	default:
	}
}

func Start(protocol string, port int) (s *Server, err error) {
	s = &Server{Protocol: protocol, Port: port}

	switch protocol {
	case "tcp":
		s.Listen, err = TCPListen(port)
		if err != nil {
			log.Print(err)
			return
		}
	case "udp":
	case "http":
	case "dns":
	default:
		return nil, fmt.Errorf("no support protocol: %s", protocol)
	}

	log.Printf("Start listener, protocol: %s, port: %d", protocol, port)
	return
}
