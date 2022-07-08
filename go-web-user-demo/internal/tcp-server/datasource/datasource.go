package datasource

import (
	"entry-task/internal/pkg/cfg"
	"fmt"
	"github.com/go-redis/redis/v8"
)

import (
	"gorm.io/driver/mysql"
	"gorm.io/gorm"
)

func NewMySql(cfg *cfg.MySqlConfig) *gorm.DB {
	db, err := gorm.Open(mysql.Open(fmt.Sprintf("%s:%s@tcp(%s)/%s?charset=%s&parseTime=%t&loc=Local",
		cfg.Username,
		cfg.Password,
		cfg.Host,
		cfg.Db,
		cfg.Charset,
		cfg.ParseTime,
	)))

	if err != nil {
		return nil
	}

	sql, err := db.DB()
	if err != nil {
		return nil
	}
	sql.SetMaxIdleConns(cfg.MaxIdleConns)
	sql.SetMaxOpenConns(cfg.MaxOpenConns)

	return db
}

func NewRedis(cfg *cfg.RedisConfig) *redis.Client {
	rClient := redis.NewClient(&redis.Options{
		Addr: cfg.Host,
		DB:   cfg.Db, // use default DB
	})
	return rClient
}
