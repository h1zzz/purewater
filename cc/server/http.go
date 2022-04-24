// MIT License Copyright (c) 2022, h1zzz

package server

import (
	"fmt"
	"log"
	"net/http"
	"sync"
)

type HTTPListener struct {
	protocol ListenerProtocol
	port     int
	status   ListenerStatus
	agents   sync.Map
	comment  string
	server   *http.Server
}

func (l *HTTPListener) handle(w http.ResponseWriter, r *http.Request) {
	log.Printf("HTTP Handle Recvfrom: %s", r.RemoteAddr)
}

func (l *HTTPListener) Start(protocol ListenerProtocol, port int) (err error) {
	mux := &http.ServeMux{}
	mux.HandleFunc("/", l.handle)

	l.server = &http.Server{Addr: fmt.Sprintf("0.0.0.0:%d", port), Handler: mux}
	l.protocol = protocol
	l.port = port
	l.SetOnline()

	go func() {
		switch protocol {
		case ListenerHTTP:
			err = l.server.ListenAndServe()
		case ListenerHTTPS:
			err = l.server.ListenAndServeTLS("cert.pem", "key.pem")
		default:
			log.Fatal("no support protocol")
		}
		if err != nil {
			log.Print(err)
			l.SetOffline()
		}
	}()

	return
}

func (l *HTTPListener) Stop() error                { return l.server.Close() }
func (l *HTTPListener) Agents() *sync.Map          { return &l.agents }
func (l *HTTPListener) Status() ListenerStatus     { return l.status }
func (l *HTTPListener) SetOffline()                { l.status = ListenerOffline }
func (l *HTTPListener) SetOnline()                 { l.status = ListenerOnline }
func (l *HTTPListener) Comment() string            { return l.comment }
func (l *HTTPListener) SetComment(comment string)  { l.comment = comment }
func (l *HTTPListener) Protocol() ListenerProtocol { return l.protocol }
func (l *HTTPListener) Port() int                  { return l.port }
