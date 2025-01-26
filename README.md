# Voxy
A voxel game in pure C and OpenGL just because I can.

## Screenshot
![Awesome screenshot](cursed.png)
It would be a shame to not put an image that show how awesome ~~cursed~~ this
game is.

## Usage
Generate a self-signed server certificate:
```shell
$ openssl genrsa -out key.pem 4096 # Generate RSA private key
$ openssl req -new -key key.pem -out csr.pem
$ openssl x509 -req -in csr.pem -signkey key.pem -out cert.pem
```

Compile:
```shell
$ export CFLAGS=...
$ make -j
```

Run:
```
$ export PORT=...
$ export ADDRESS=...
$ ./bin/voxy/server/voxy-server $PORT cert.pem key.pem ./bin/mod/base/server/base-server.so # Server
$ ./bin/voxy/client/voxy-client $ADDRESS $PORT world ./bin/mod/base/client/base-client.so # Client
```

