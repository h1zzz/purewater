// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"fmt"
	"log"
	"sync"
)

type ListenerStatus int
type ListenerProtocol string

const (
	ListenerOffline = ListenerStatus(0)
	ListenerOnline  = ListenerStatus(1)
)

const (
	ListenerTCP   = ListenerProtocol("tcp")
	ListenerUDP   = ListenerProtocol("udp")
	ListenerHTTP  = ListenerProtocol("http")
	ListenerHTTPS = ListenerProtocol("https")
	ListenerDNS   = ListenerProtocol("dns")
)

type Listener interface {
	Start(protocol ListenerProtocol, port int) (err error)
	// Restart() error
	Stop() error
	Agents() *sync.Map
	Status() ListenerStatus
	SetOffline()
	SetOnline()
	Comment() string
	SetComment(comment string)
	Protocol() ListenerProtocol
	Port() int
}

type Server struct {
	listeners sync.Map
}

func (s *Server) Start(protocol ListenerProtocol, port int) error {
	if v, ok := s.listeners.Load(port); ok {
		l := v.(Listener)
		return fmt.Errorf("The port is already in use by the listener, %s, %d, %s", l.Protocol(), l.Port(), l.Comment())
	}

	var listener Listener

	switch protocol {
	case ListenerTCP:
		listener = &TCPListener{}
	case ListenerUDP:
		listener = &UDPListener{}
	case ListenerHTTP, ListenerHTTPS:
		listener = &HTTPListener{}
	case ListenerDNS:
		listener = &DNSListener{}
	default:
		return fmt.Errorf("unsupported protocol")
	}

	err := listener.Start(protocol, port)
	if err != nil {
		log.Print(err)
		return err
	}

	s.listeners.Store(port, listener)

	return nil
}

func (s *Server) Stop(port int) error {
	if v, ok := s.listeners.Load(port); ok {
		l := v.(Listener)
		s.listeners.Delete(port)
		return l.Stop()
	}
	return fmt.Errorf("Listener does not exist")
}

func (s *Server) Listeners() *sync.Map { return &s.listeners }
