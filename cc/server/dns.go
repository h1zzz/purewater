// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"fmt"
	"log"
	"sync"

	"github.com/miekg/dns"
)

type DNSListener struct {
	protocol ListenerProtocol
	port     int
	status   ListenerStatus
	agents   sync.Map
	comment  string
	server   *dns.Server
}

func (l *DNSListener) handle(w dns.ResponseWriter, m *dns.Msg) {
	log.Printf("DNS Handle Recvfrom: %s", w.RemoteAddr().String())
}

func (l *DNSListener) Start(protocol ListenerProtocol, port int) (err error) {
	mux := dns.NewServeMux()
	mux.HandleFunc(".", l.handle)

	l.server = &dns.Server{
		Addr:    fmt.Sprintf("0.0.0.0:%d", port),
		Net:     "udp",
		Handler: mux,
	}

	l.protocol = protocol
	l.port = port
	l.SetOnline()

	go func(l *DNSListener) {
		if err := l.server.ListenAndServe(); err != nil {
			log.Print(err)
			l.SetOffline()
		}
	}(l)

	return
}

func (l *DNSListener) Stop() error                { return l.server.Shutdown() }
func (l *DNSListener) Agents() *sync.Map          { return &l.agents }
func (l *DNSListener) Status() ListenerStatus     { return l.status }
func (l *DNSListener) SetOffline()                { l.status = ListenerOffline }
func (l *DNSListener) SetOnline()                 { l.status = ListenerOnline }
func (l *DNSListener) Comment() string            { return l.comment }
func (l *DNSListener) SetComment(comment string)  { l.comment = comment }
func (l *DNSListener) Protocol() ListenerProtocol { return l.protocol }
func (l *DNSListener) Port() int                  { return l.port }
