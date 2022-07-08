package cache

import (
	"context"
	"encoding/json"
	"entry-task/internal/tcp-server/model"
	"github.com/go-redis/redis/v8"
	"math/rand"
	"time"
)

const keyPrefix = "user:"

const expiration = time.Hour

type UserCache struct {
	r *redis.Client
}

func NewUserCache(r *redis.Client) *UserCache {
	return &UserCache{
		r: r,
	}
}

func (c *UserCache) Set(ctx context.Context, user *model.User) error {
	key := keyPrefix + user.Name
	value, _ := json.Marshal(user)
	return c.r.Set(ctx, key, value, randomExpiration()).Err()
}

func (c *UserCache) Delete(ctx context.Context, username string) error {
	key := keyPrefix + username
	return c.r.Del(ctx, key).Err()
}

func (c *UserCache) Get(ctx context.Context, username string) (*model.User, error) {
	key := keyPrefix + username
	value, err := c.r.Get(ctx, key).Result()
	if err == nil {
		var user model.User
		err = json.Unmarshal([]byte(value), &user)
		if err != nil {
			return nil, err
		}
		return &user, nil
	}
	return nil, err
}

func randomExpiration() time.Duration {
	return time.Duration(int64(expiration) + rand.Int63n(int64(expiration)))
}
