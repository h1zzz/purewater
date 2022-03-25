// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"log"
	"net"
)

func (s *Server) TCPListen() (err error) {
	s.TCPListener, err = net.ListenTCP("tcp", &net.TCPAddr{IP: net.IPv4(127, 0, 0, 1), Port: s.Port})
	if err != nil {
		log.Print(err)
		return err
	}
	go func(s *Server) {
		defer s.TCPListener.Close()
		for {
			conn, err := s.TCPListener.Accept()
			if err != nil {
				log.Print(err)
				s.setOffline()
				break
			}
			go TCPHandle(conn)
		}
	}(s)
	return
}

func TCPHandle(conn net.Conn) {
	defer conn.Close()
	log.Printf("TCP Handle Recvfrom: %s", conn.RemoteAddr().String())
}
