# MIT License Copyright (c) 2021, h1zzz

FROM golang:1.17.2-alpine3.14 as builder

WORKDIR /work
COPY . .
RUN go mod download && go build -ldflags "-s -w" -o cc

FROM alpine:3.14

ENV GIN_MODE="release"
COPY --from=builder /work/cc /usr/local/bin
EXPOSE 8080
ENTRYPOINT [ "cc" ]
