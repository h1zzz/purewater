# MIT License Copyright (c) 2021, h1zzz

FROM golang:1.17.2-alpine3.14 as builder

WORKDIR /work

COPY . .

RUN apk add --no-cache openssl \
    && openssl genrsa -out key.pem 2048 \
    && openssl req -new -x509 -days 3650 -key key.pem -out cert.pem -subj "/C=US/ST= /L= /O= /OU= /CN= /emailAddress= " \
    && go mod download && go build -ldflags "-s -w" -o cc

FROM alpine:3.14

WORKDIR /app

COPY --from=builder /work/cc /app/cc
COPY --from=builder /work/cert.pem /app/cert.pem
COPY --from=builder /work/key.pem /app/key.pem

ENTRYPOINT [ "/app/cc" ]
