package service

import (
	"context"
	"entry-task/internal/pkg/cfg"
	"entry-task/internal/pkg/hash"
	"entry-task/internal/pkg/rpc"
	pb "entry-task/internal/pkg/rpc"
	"entry-task/internal/tcp-server/cache"
	"entry-task/internal/tcp-server/datasource"
	"entry-task/internal/tcp-server/model"
	"entry-task/internal/tcp-server/repository"
	"entry-task/internal/tcp-server/utils"
	"errors"
	"fmt"
	"github.com/agiledragon/gomonkey"
	"github.com/go-redis/redis/v8"
	"google.golang.org/grpc"
	"gorm.io/gorm"
	"log"
	"net"
	"reflect"
	"sync"
	"testing"
	"time"
)

var s *grpc.Server
var engine *gorm.DB
var client *redis.Client

var dbConfig = &cfg.MySqlConfig{
	Host:         "10.100.1.181:3306",
	Username:     "root",
	Password:     "root",
	Db:           "test",
	Charset:      "utf8mb4",
	ParseTime:    true,
	MaxIdleConns: 1,
	MaxOpenConns: 1,
}

var cacheConfig = &cfg.RedisConfig{
	Host: "10.100.1.181:6379",
	Db:   0,
}

var appConfig = &cfg.AppConfig{
	LogFile:         "./log",
	UploadSavePath:  "./avatar",
	UploadServerUrl: "127.0.0.1:8080/avatar",
}

func startServer(wg *sync.WaitGroup) {
	s = grpc.NewServer()

	engine = datasource.NewMySql(dbConfig)
	client = datasource.NewRedis(cacheConfig)

	userRepo := repository.NewUserRepository(engine)
	userCache := cache.NewUserCache(client)

	pb.RegisterUserServiceServer(s, NewUserServiceGrpc(appConfig, userRepo, userCache))

	socket, err := net.Listen("tcp", fmt.Sprintf("%v:%v", "127.0.0.1", 8001))
	if err != nil {
		log.Fatalf("net.Listen err: %v", err)
	}
	wg.Done()
	err = s.Serve(socket)
	if err != nil {
		log.Fatalf("net.Serve err: %v", err)
	}
}

func shutdownServer() {
	s.Stop()
	_ = client.Close()
}

func TestUserService(t *testing.T) {
	var wg sync.WaitGroup
	wg.Add(1)
	go startServer(&wg)
	wg.Wait()
	time.Sleep(time.Second)

	conn, err := grpc.Dial("127.0.0.1:8001", grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		t.Fatalf("did not connect: %v", err)
	}
	defer func(conn *grpc.ClientConn) {
		_ = conn.Close()
	}(conn)
	c := rpc.NewUserServiceClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()

	t.Run("test-ping", func(t *testing.T) {
		r, err := c.Ping(ctx, &rpc.PingRequest{Msg: "fuckyou"})
		if err != nil {
			t.Fatalf("error: %v", err)
		}
		fmt.Printf("code: %d, msg: %s", r.Code, r.Message)
	})

	t.Run("test-register", func(t *testing.T) {
		req := pb.RegisterRequest{
			Username: "testrpc",
			Password: "123456",
			Nickname: "fuck",
			Avatar:   "",
		}

		r, err := c.Register(ctx, &req)
		if err != nil {
			t.Fatalf("error: %v", err)
		}
		if r.Code != 0 {
			t.Fatalf("error: %+v", r)
		}
		log.Printf("register: %+v", r)
	})

	t.Run("test-login", func(t *testing.T) {
		req := pb.LoginRequest{
			Username: "testrpc",
			Password: "123456",
		}

		r, err := c.Login(ctx, &req)
		if err != nil {
			t.Fatalf("error: %v", err)
		}
		if r.Code != 0 {
			t.Fatalf("error: %+v", r)
		}
		log.Printf("login: %+v", r)

	})

	t.Run("test-get-profile", func(t *testing.T) {
		req := pb.GetProfileRequest{Username: "testrpc"}
		r, err := c.GetProfile(ctx, &req)
		if err != nil {
			t.Fatalf("error: %v", err)
		}
		log.Printf("get-profile: %+v", r)
	})

	t.Run("test-update-profile", func(t *testing.T) {
		req := pb.UpdateProfileRequest{
			Username: "testrpc",
			Nickname: "lebron",
			Avatar:   "hahahahhahahaURL",
		}
		r, err := c.UpdateProfile(ctx, &req)
		if err != nil {
			t.Fatalf("error: %v", err)
		}
		log.Printf("update-profile: %+v", r)
	})

	t.Run("test-upload-avatar", func(t *testing.T) {
		req := pb.UploadRequest{
			Username: "testrpc",
			FileName: "hahaha.txt",
			Content:  []byte("123421321"),
		}
		r, err := c.UploadAvatar(ctx, &req)
		if err != nil {
			t.Fatalf("error: %v", err)
		}
		log.Printf("upload avatar: %+v", r)
	})

	shutdownServer()
}

func TestUserService_Register(t *testing.T) {
	config := &cfg.AppConfig{}
	userRepo := &repository.UserRepository{}
	userCache := &cache.UserCache{}
	svc := NewUserServiceGrpc(config, userRepo, userCache)

	ctx := context.Background()
	username := "test_username"
	nickname := "test_nickname"
	password := "test_password"

	request := pb.RegisterRequest{
		Username: username,
		Nickname: nickname,
		Password: password,
	}

	t.Run("normal register", func(t *testing.T) {
		// Target output
		want := &pb.Reply{
			Code:    0,
			Message: "OK",
		}
		// Mock repository call
		patches := gomonkey.ApplyMethod(reflect.TypeOf(userRepo), "Create",
			func(_ *repository.UserRepository, _ *model.User) error {
				return nil
			}).ApplyMethod(reflect.TypeOf(userRepo), "GetByName",
			func(_ *repository.UserRepository, _ string) (*model.User, error) {
				return nil, gorm.ErrRecordNotFound
			}).ApplyFunc(hash.MD5, func(_ string) string {
			return "mock_hashing"
		})
		defer patches.Reset()
		// Test and compare with reflect.DeepEqual
		resp, err := svc.Register(ctx, &request)
		if err != nil {
			t.Errorf("TestUserService_Register got error %v", err)
		}

		if !reflect.DeepEqual(want, resp) {
			t.Errorf("TestUserService_Register want: %v got %v", want, resp)
		}
	})
	t.Run("invalid register", func(t *testing.T) {
		// Mock GetUser with record found
		patches := gomonkey.ApplyMethod(reflect.TypeOf(userRepo), "GetByName",
			func(_ *repository.UserRepository, _ string) (*model.User, error) {
				return &model.User{}, nil
			})
		defer patches.Reset()

		// should return an error
		_, err := svc.Register(ctx, &request)
		if err == nil {
			t.Error("TestUserService_Register should return error but not")
		}
	})
}

func TestUserService_Login(t *testing.T) {
	config := &cfg.AppConfig{}
	userRepo := &repository.UserRepository{}
	userCache := &cache.UserCache{}
	svc := NewUserServiceGrpc(config, userRepo, userCache)

	ctx := context.Background()
	username := "test_username"
	nickname := "test_nickname"
	password := "test_password"

	request := pb.LoginRequest{
		Username: username,
		Password: password,
	}

	t.Run("normal login", func(t *testing.T) {
		// Mock repository call
		patches := gomonkey.ApplyMethod(reflect.TypeOf(userRepo), "Create",
			func(_ *repository.UserRepository, _ *model.User) error {
				return nil
			}).ApplyMethod(reflect.TypeOf(userRepo), "GetByName",
			func(_ *repository.UserRepository, _ string) (*model.User, error) {
				return &model.User{
					Name:     username,
					Nickname: nickname,
					Password: password,
					Status:   1,
				}, nil
			}).ApplyFunc(hash.MD5, func(_ string) string {
			return password
		}).ApplyMethod(reflect.TypeOf(userCache), "Set", func(_ *cache.UserCache, _ context.Context, _ *model.User) error {
			return nil
		})
		defer patches.Reset()
		// Test and compare with reflect.DeepEqual
		_, err := svc.Login(ctx, &request)
		if err != nil {
			t.Errorf("TestUserService_Login got error %v", err)
		}
	})
	t.Run("login no such user", func(t *testing.T) {
		// Mock DAO call
		patches := gomonkey.ApplyMethod(reflect.TypeOf(userRepo), "GetByName",
			func(_ *repository.UserRepository, _ string) (*model.User, error) {
				return nil, gorm.ErrRecordNotFound
			})
		defer patches.Reset()
		// Test and compare
		_, err := svc.Login(ctx, &request)
		if err == nil {
			t.Errorf("TestUserService_Login should return err but not")
		}
	})

	t.Run("login incorrect password", func(t *testing.T) {
		// Mock DAO call
		patches := gomonkey.ApplyMethod(reflect.TypeOf(userRepo), "GetByName",
			func(_ *repository.UserRepository, _ string) (*model.User, error) {
				return &model.User{
					Name:     username,
					Password: "xxx",
					Nickname: nickname,
					Status:   1,
				}, nil
			})
		defer patches.Reset()
		patches.ApplyFunc(hash.MD5, func(_ string) string {
			return password
		})
		// Test and compare
		_, err := svc.Login(ctx, &request)
		if err == nil {
			t.Errorf("TestUserService_Login should return err but not")
		}
	})
}

func TestUserService_GetProfile(t *testing.T) {
	config := &cfg.AppConfig{}
	userRepo := &repository.UserRepository{}
	userCache := &cache.UserCache{}
	svc := NewUserServiceGrpc(config, userRepo, userCache)

	ctx := context.Background()
	username := "test_username"
	nickname := "test_nickname"
	avatar := "test_avatar_url"
	password := "test_password"
	// Input
	request := pb.GetProfileRequest{
		Username: username,
	}

	t.Run("normal get profile from cache", func(t *testing.T) {
		// Mock DAO call
		patches := gomonkey.ApplyMethod(reflect.TypeOf(userCache), "Get", func(_ *cache.UserCache, _ context.Context, _ string) (*model.User, error) {
			return &model.User{
				Name:     username,
				Password: password,
				Nickname: nickname,
				Avatar:   avatar,
				Status:   1,
			}, nil
		})
		defer patches.Reset()

		// Test and compare
		resp, err := svc.GetProfile(ctx, &request)
		if err != nil {
			t.Errorf("TestUserService_GetProfile got error %v", err)
		}

		if resp.Username != username || resp.Nickname != nickname || resp.Avatar != avatar {
			t.Errorf("TestUserService_GetProfile value doesn't match")
		}
	})

	t.Run("normal get profile from db", func(t *testing.T) {
		// Mock DAO call
		patches := gomonkey.ApplyMethod(reflect.TypeOf(userCache), "Get",
			func(_ *cache.UserCache, _ context.Context, _ string) (*model.User, error) {
				return nil, redis.Nil
			}).ApplyMethod(reflect.TypeOf(userRepo), "GetByName",
			func(_ *repository.UserRepository, _ string) (*model.User, error) {
				return &model.User{
					Name:     username,
					Nickname: nickname,
					Password: password,
					Avatar:   avatar,
					Status:   1,
				}, nil
			}).ApplyMethod(reflect.TypeOf(userCache), "Set",
			func(_ *cache.UserCache, _ context.Context, _ *model.User) error {
				return nil
			})
		defer patches.Reset()

		// Test and compare
		resp, err := svc.GetProfile(ctx, &request)
		if err != nil {
			t.Errorf("TestUserService_GetProfile got error %v", err)
		}

		if resp.Username != username || resp.Nickname != nickname || resp.Avatar != avatar {
			t.Errorf("TestUserService_GetProfile value doesn't match")
		}
	})
}

func TestUserService_UpdateProfile(t *testing.T) {
	config := &cfg.AppConfig{}
	userRepo := &repository.UserRepository{}
	userCache := &cache.UserCache{}
	svc := NewUserServiceGrpc(config, userRepo, userCache)

	ctx := context.Background()
	username := "test_username"
	nickname := "test_nickname"
	avatar := "test_avatar_url"
	// Input
	request := pb.UpdateProfileRequest{
		Username: username,
		Nickname: nickname,
		Avatar:   avatar,
	}

	t.Run("normal edit user", func(t *testing.T) {
		// Mock DAO call
		patches := gomonkey.ApplyMethod(reflect.TypeOf(userRepo), "GetByName", func(_ *repository.UserRepository, _ string) (*model.User, error) {
			return &model.User{
				Name:     username,
				Nickname: nickname,
				Avatar:   avatar,
				Status:   1,
			}, nil
		}).ApplyMethod(reflect.TypeOf(userRepo), "UpdateNicknameAndAvatar", func(_ *repository.UserRepository, _ uint, _, _ string) error {
			return nil
		}).ApplyMethod(reflect.TypeOf(userCache), "Delete", func(_ *cache.UserCache, _ context.Context, _ string) error {
			return nil
		})
		defer patches.Reset()
		// Test and compare
		resp, err := svc.UpdateProfile(ctx, &request)
		if err != nil {
			t.Errorf("TestUserService_UpdateProfile got error %v", err)
		}
		if resp.Code != 0 {
			t.Errorf("TestUserService_UpdateProfile returned error: %v", resp)
		}
	})
	t.Run("update failed", func(t *testing.T) {
		// Mock DAO call
		patches := gomonkey.ApplyMethod(reflect.TypeOf(userRepo), "GetByName", func(_ *repository.UserRepository, _ string) (*model.User, error) {
			return &model.User{
				Name:     username,
				Nickname: nickname,
				Avatar:   avatar,
				Status:   1,
			}, nil
		}).ApplyMethod(reflect.TypeOf(userRepo), "UpdateNicknameAndAvatar", func(_ *repository.UserRepository, _ uint, _, _ string) error {
			return errors.New("123")
		}).ApplyMethod(reflect.TypeOf(userCache), "Delete", func(_ *cache.UserCache, _ context.Context, _ string) error {
			return nil
		})
		defer patches.Reset()

		// Test and compare
		_, err := svc.UpdateProfile(ctx, &request)
		if err == nil {
			t.Error("TestUserService_UpdateProfile should return error but not")
		}
	})
}

func TestUserService_UploadAvatar(t *testing.T) {
	config := &cfg.AppConfig{}
	userRepo := &repository.UserRepository{}
	userCache := &cache.UserCache{}
	svc := NewUserServiceGrpc(config, userRepo, userCache)

	ctx := context.Background()

	// Mock stuffs
	fileName := "test.png"
	hashFileName := "hash-test.png"
	username := "test_username"

	// Input
	request := pb.UploadRequest{
		Username: username,
		FileName: fileName,
		Content:  make([]byte, 32),
	}

	t.Run("normal upload file", func(t *testing.T) {
		// Mock DAO call
		patches := gomonkey.ApplyMethod(reflect.TypeOf(userRepo), "GetByName",
			func(_ *repository.UserRepository, _ string) (*model.User, error) {
				return &model.User{
					Name:   username,
					Status: 1,
				}, nil
			}).ApplyFunc(utils.HashedFileName, func(_, _ string) string {
			return hashFileName
		}).ApplyFunc(utils.CheckPath, func(string) bool {
			return true
		}).ApplyFunc(utils.CheckPermission, func(string) bool {
			return true
		}).ApplyFunc(utils.SaveFileBytes, func(*[]byte, string) error {
			return nil
		}).ApplyMethod(reflect.TypeOf(userCache), "Delete", func(_ *cache.UserCache, _ context.Context, _ string) error {
			return nil
		}).ApplyMethod(reflect.TypeOf(userRepo), "Update",
			func(_ *repository.UserRepository, _ *model.User) error {
				return nil
			})
		defer patches.Reset()

		// Test and compare
		resp, err := svc.UploadAvatar(ctx, &request)
		if err != nil {
			t.Errorf("TestUserService_UploadAvatar got error %v", err)
		}
		if resp.Reply.Code != 0 {
			t.Errorf("TestUserService_UploadAvatar returned error: %v", resp)
		}

	})
	t.Run("upload file failed", func(t *testing.T) {
		// Mock DAO call
		patches := gomonkey.ApplyMethod(reflect.TypeOf(userRepo), "GetByName",
			func(_ *repository.UserRepository, _ string) (*model.User, error) {
				return &model.User{
					Name:   username,
					Status: 1,
				}, nil
			}).ApplyFunc(utils.HashedFileName, func(_, _ string) string {
			return hashFileName
		}).ApplyFunc(utils.CheckPath, func(string) bool {
			return true
		}).ApplyFunc(utils.CheckPermission, func(string) bool {
			return true
		}).ApplyFunc(utils.SaveFileBytes, func(*[]byte, string) error {
			return errors.New("123")
		}).ApplyMethod(reflect.TypeOf(userCache), "Delete", func(_ *cache.UserCache, _ context.Context, _ string) error {
			return nil
		}).ApplyMethod(reflect.TypeOf(userRepo), "Update",
			func(_ *repository.UserRepository, _ *model.User) error {
				return nil
			})
		defer patches.Reset()
		// Test and compare
		resp, err := svc.UploadAvatar(ctx, &request)
		if err == nil {
			t.Errorf("TestUserService_UploadAvatar should return error but not")
		}
		if resp.Reply.Code == 0 {
			t.Errorf("TestUserService_UploadAvatar returned not error but need error: %v", resp)
		}
	})
}
