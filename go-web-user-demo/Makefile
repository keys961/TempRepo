all: http-server tcp-server

http-server:
	go build -o http-server ./cmd/http-server/main.go

tcp-server:
	go build -o tcp-server ./cmd/tcp-server/main.go

clean:
	rm -rf tcp-server
	rm -rf http-server