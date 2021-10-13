# MIT License Copyright (c) 2021, h1zzz

FROM golang:alpine as builder

WORKDIR /work
COPY . .
RUN go mod download
RUN go build -ldflags "-s -w" -o cc


FROM alpine:latest

ENV GIN_MODE="release"
COPY --from=builder /work/cc /usr/local/bin
EXPOSE 8080
ENTRYPOINT [ "cc" ]
