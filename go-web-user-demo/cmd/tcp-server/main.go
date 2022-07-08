package main

import (
	"entry-task/internal/pkg/cfg"
	"entry-task/internal/pkg/rpc"
	"entry-task/internal/tcp-server/cache"
	"entry-task/internal/tcp-server/datasource"
	"entry-task/internal/tcp-server/repository"
	"entry-task/internal/tcp-server/service"
	"flag"
	"fmt"
	"github.com/go-redis/redis/v8"
	"google.golang.org/grpc"
	"gopkg.in/natefinch/lumberjack.v2"
	"gorm.io/gorm"
	"log"
	"net"
	"net/http"
	_ "net/http/pprof"
	"os"
)

var (
	configFile string
	config     *cfg.TcpServerConfig

	_ *grpc.Server
	s *rpc.Server

	engine *gorm.DB
	client *redis.Client
)

func init() {
	initFlag()
	if err := initConfig(); err != nil {
		log.Fatalf("init config err: %v", err)
	}
	initLog()
	_ = initMySql()
	_ = initRedis()
}

func initFlag() {
	flag.StringVar(&configFile, "config", "./tcp.yml", "Configuration file.")
	flag.Parse()
}

func initConfig() error {
	var c *cfg.TcpServerConfig
	c, err := cfg.ReadTcpServerConfig(configFile)
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

func initMySql() error {
	engine = datasource.NewMySql(&config.Mysql)
	return nil
}

func initRedis() error {
	client = datasource.NewRedis(&config.Redis)
	return nil
}

func main() {
	s = rpc.NewServer()
	err := s.Register(service.NewUserService(&config.App, repository.NewUserRepository(engine), cache.NewUserCache(client)))

	if err != nil {
		log.Fatalf("failed to register user service: %v", err)
	}
	// sGrpc = grpc.NewServer()
	//
	// pb.RegisterUserServiceServer(sGrpc, service.NewUserServiceGrpc(&config.App,
	//	 repository.NewUserRepository(engine),
	//	 cache.NewUserCache(client)))

	socket, err := net.Listen("tcp", fmt.Sprintf("%v:%v", config.Server.Host, config.Server.Port))
	if err != nil {
		log.Fatalf("net.Listen err: %v", err)
	}
	NewProfileHttpServer(":19999")
	log.Println("Starting TCP server...")
	s.Accept(socket)
	// err = sGrpc.Serve(socket)
	if err != nil {
		log.Fatalf("net.Serve err: %v", err)
	}
}

func NewProfileHttpServer(addr string) {
	go func() {
		log.Fatalln(http.ListenAndServe(addr, nil))
	}()
}
