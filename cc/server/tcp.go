// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"log"
	"net"
	"sync"
)

type TCPListener struct {
	protocol ListenerProtocol
	port     int
	status   ListenerStatus
	agents   sync.Map
	comment  string
	listener *net.TCPListener
}

func (l *TCPListener) handle(conn net.Conn) {
	defer conn.Close()
	log.Printf("TCP Handle Recvfrom: %s", conn.RemoteAddr().String())
}

func (l *TCPListener) Start(protocol ListenerProtocol, port int) (err error) {
	l.listener, err = net.ListenTCP("tcp", &net.TCPAddr{IP: net.IPv4(0, 0, 0, 0), Port: port})
	if err != nil {
		log.Print(err)
		return
	}

	l.protocol = protocol
	l.port = port
	l.SetOnline()

	go func(l *TCPListener) {
		defer l.listener.Close()
		for {
			conn, err := l.listener.Accept()
			if err != nil {
				log.Print(err)
				l.SetOffline()
				break
			}
			go l.handle(conn)
		}
	}(l)
	return nil
}

func (l *TCPListener) Stop() error                { return l.listener.Close() }
func (l *TCPListener) Agents() *sync.Map          { return &l.agents }
func (l *TCPListener) Status() ListenerStatus     { return l.status }
func (l *TCPListener) SetOffline()                { l.status = ListenerOffline }
func (l *TCPListener) SetOnline()                 { l.status = ListenerOnline }
func (l *TCPListener) Comment() string            { return l.comment }
func (l *TCPListener) SetComment(comment string)  { l.comment = comment }
func (l *TCPListener) Protocol() ListenerProtocol { return l.protocol }
func (l *TCPListener) Port() int                  { return l.port }
