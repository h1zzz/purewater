
```shell
$ openssl genrsa -out key.pem 2048
$ openssl req -new -x509 -days 3650 -key key.pem -out cert.pem -subj "/C=US/ST= /L= /O= /OU= /CN= /emailAddress= "
$ go run .
```

```shell
$ docker build -t cc .
$ docker run -td -P --name cc cc
```

```
$ docker-compose up -d
```
