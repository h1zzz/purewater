
```shell
$ openssl genrsa -out key.pem 2048
$ openssl req -new -x509 -days 3650 -key key.pem -out cert.pem -subj "/C=US/ST= /L= /O= /OU= /CN= /emailAddress= "
$ go run .
```

```shell
$ docker build -t cc .
$ docker run -td --name cc --network host -v /etc/localtime:/etc/localtime:ro \
    -e LISTEN_ADDR="0.0.0.0:443" \
    -e MYSQL_ADDR="mysql:3306" \
    -e MYSQL_DATABASE="mysql:3306" \
    -e MYSQL_USER="user" \
    -e MYSQL_PASSWORD="password" cc
```

```
$ docker-compose up -d
```
