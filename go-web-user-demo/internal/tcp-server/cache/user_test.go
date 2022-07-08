package cache

import (
	"context"
	"entry-task/internal/pkg/cfg"
	"entry-task/internal/tcp-server/datasource"
	"entry-task/internal/tcp-server/model"
	"reflect"
	"testing"
)

var r *UserCache

func init() {
	r = &UserCache{
		r: datasource.NewRedis(
			&cfg.RedisConfig{
				Host: "10.100.1.181:6379",
				Db:   0,
			})}
}

func TestUserCache(t *testing.T) {
	user := &model.User{
		ID:       1,
		Name:     "test",
		Password: "test",
		Nickname: "test",
		Avatar:   "test",
		Status:   1,
	}
	ctx := context.Background()
	err := r.Set(ctx, user)
	if err != nil {
		t.Fatalf("%v", err)
	}
	u, err := r.Get(ctx, user.Name)
	if err != nil || !reflect.DeepEqual(u, user) {
		t.Fatalf("%v, %v %v", err, user, u)
	}
	err = r.Delete(ctx, user.Name)
	if err != nil {
		t.Fatalf("%v", err)
	}
	u, _ = r.Get(ctx, user.Name)
	if u != nil {
		t.Fatalf("not deleted")
	}
}
