// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"log"
	"net"
)

func (s *Server) UDPListen() (err error) {
	s.UDPConn, err = net.ListenUDP("udp", &net.UDPAddr{IP: net.IPv4(127, 0, 0, 1), Port: s.Port})
	if err != nil {
		log.Print(err)
		return
	}
	go func(s *Server) {
		if err := UDPHandle(*s.UDPConn); err != nil {
			log.Print(err)
			s.setOffline()
		}
	}(s)
	return
}

func UDPHandle(conn net.UDPConn) error {
	defer conn.Close()
	log.Printf("UDP Handle Recvfrom: %s", conn.RemoteAddr().String())
	return nil
}
