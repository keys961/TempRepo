package main

import (
	"context"
	"entry-task/internal/http-server/router"
	"entry-task/internal/http-server/service"
	"entry-task/internal/pkg/cfg"
	"entry-task/internal/pkg/rpc"
	"errors"
	"flag"
	"fmt"
	"github.com/silenceper/pool"
	"google.golang.org/grpc"
	"gopkg.in/natefinch/lumberjack.v2"
	"log"
	"net/http"
	_ "net/http/pprof"
	"os"
	"os/signal"
	"syscall"
	"time"
)

var (
	configFile    string
	config        *cfg.HttpServerConfig
	_             *grpc.ClientConn
	rpcClientPool pool.Pool
	ctx           context.Context
)

func init() {
	initFlag()
	if err := initConfig(); err != nil {
		log.Fatalf("init config err: %v", err)
	}
	initLog()
	if err := initRpc(); err != nil {
		log.Fatalf("init rpc err: %v", err)
	}
}

func initFlag() {
	flag.StringVar(&configFile, "config", "./http.yml", "Configuration file.")
	flag.Parse()
}

func initConfig() error {
	var c *cfg.HttpServerConfig
	c, err := cfg.ReadHttpServerConfig(configFile)
	if err != nil {
		return err
	}
	config = c
	return nil
}

func initLog() {
	log.SetFlags(log.LstdFlags | log.Lmicroseconds | log.Lshortfile)
	if config.App.LogFile == "" {
		log.SetOutput(os.Stdout)
	} else {
		log.SetOutput(&lumberjack.Logger{
			Filename:   config.App.LogFile,
			MaxSize:    100,
			MaxBackups: 5,
			MaxAge:     7,
		})
	}
}

func initGrpc() error {
	c, err := grpc.Dial(config.Rpc.Host, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		return err
	}
	_ = c
	return nil
}

func initRpc() error {
	p, err := rpc.NewClientPool("tcp", config.Rpc.Host,
		config.Rpc.MaxConn, config.Rpc.MaxIdle, config.Rpc.MaxIdleTime, rpc.DefaultOption)
	if err != nil {
		return err
	}
	rpcClientPool = p
	return nil
}

func main() {
	ctx = context.Background()
	// userService := service.NewUserServiceGrpc(ctx, grpcClient)
	userService := service.NewUserServiceRpc(ctx, rpcClientPool)
	r := router.NewRouter(&config.Redis, userService)

	s := &http.Server{
		Addr:    fmt.Sprintf("%v:%v", config.Server.Host, config.Server.Port),
		Handler: r,
	}

	go func() {
		log.Printf("Starting HTTP server on %s...\n", s.Addr)
		if err := s.ListenAndServe(); err != nil && !errors.Is(err, http.ErrServerClosed) {
			log.Fatalf("s.ListenAndServe err: %v", err)
		}
	}()

	NewProfileHttpServer(":9999")

	quit := make(chan os.Signal)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit
	log.Println("Shutting down server...")

	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	if err := s.Shutdown(ctx); err != nil {
		log.Fatalf("Server forced to shutdown: %v", err)
	}

	log.Println("Server exited")
}

func NewProfileHttpServer(addr string) {
	go func() {
		log.Fatalln(http.ListenAndServe(addr, nil))
	}()
}
