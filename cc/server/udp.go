// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"log"
	"net"
	"sync"
)

type UDPListener struct {
	protocol ListenerProtocol
	port     int
	status   ListenerStatus
	agents   sync.Map
	comment  string
	conn     *net.UDPConn
}

func (l *UDPListener) handle() error {
	defer l.conn.Close()
	for {
		buf := make([]byte, 2048)
		_, err := l.conn.Read(buf)
		if err != nil {
			log.Print(err)
			return err
		}
		log.Printf("UDP Handle Recvfrom: %s", l.conn.RemoteAddr().String())
	}
}

func (l *UDPListener) Start(protocol ListenerProtocol, port int) (err error) {
	l.conn, err = net.ListenUDP("udp", &net.UDPAddr{IP: net.IPv4(0, 0, 0, 0), Port: port})
	if err != nil {
		log.Print(err)
		return
	}

	l.protocol = protocol
	l.port = port
	l.SetOnline()

	go func(l *UDPListener) {
		if err := l.handle(); err != nil {
			log.Print(err)
			l.SetOffline()
		}
	}(l)

	return nil
}

func (l *UDPListener) Stop() error                { return l.conn.Close() }
func (l *UDPListener) Agents() *sync.Map          { return &l.agents }
func (l *UDPListener) Status() ListenerStatus     { return l.status }
func (l *UDPListener) SetOffline()                { l.status = ListenerOffline }
func (l *UDPListener) SetOnline()                 { l.status = ListenerOnline }
func (l *UDPListener) Comment() string            { return l.comment }
func (l *UDPListener) SetComment(comment string)  { l.comment = comment }
func (l *UDPListener) Protocol() ListenerProtocol { return l.protocol }
func (l *UDPListener) Port() int                  { return l.port }
