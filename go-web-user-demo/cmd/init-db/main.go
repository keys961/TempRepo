package main

import (
	"entry-task/internal/pkg/cfg"
	"entry-task/internal/pkg/hash"
	"entry-task/internal/tcp-server/datasource"
	"entry-task/internal/tcp-server/model"
	"entry-task/internal/tcp-server/repository"
	"fmt"
	"log"
)

func main() {
	engine := datasource.NewMySql(
		&cfg.MySqlConfig{
			Host:         "127.0.0.1:3306",
			Username:     "root",
			Password:     "123456",
			Db:           "test",
			Charset:      "utf8mb4",
			ParseTime:    true,
			MaxIdleConns: 2,
			MaxOpenConns: 2,
		},
	)
	repo := repository.NewUserRepository(engine)

	for i := 10000000; i < 20000000; i++ {
		u := model.User{
			Name:     fmt.Sprintf("%v", i),
			Password: hash.MD5("123456"),
			Nickname: fmt.Sprintf("%v", i),
			Avatar:   "",
			Status:   1,
		}
		err := repo.Create(&u)
		if err != nil {
			log.Fatalf("%v", err)
		}
	}
}
