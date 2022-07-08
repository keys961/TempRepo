package repository

import (
	"entry-task/internal/pkg/cfg"
	"entry-task/internal/tcp-server/datasource"
	"entry-task/internal/tcp-server/model"
	"log"
	"testing"
)

var r *UserRepository

func init() {
	r = NewUserRepository(datasource.NewMySql(
		&cfg.MySqlConfig{
			Host:         "10.100.1.181:3306",
			Username:     "root",
			Password:     "root",
			Db:           "test",
			Charset:      "utf8mb4",
			ParseTime:    true,
			MaxIdleConns: 1,
			MaxOpenConns: 1,
		}))
}

func TestUserRepository_DeleteCreateAndGet(t *testing.T) {
	user := &model.User{
		Name:     "test",
		Password: "test",
		Nickname: "test",
		Avatar:   "test",
		Status:   1,
	}
	_ = r.DeleteByName("test")
	err := r.Create(user)
	if err != nil {
		t.Fatalf("error creating user: %v", err)
	}
	err = r.Create(user)
	if err == nil {
		t.Fatalf("err with duplicate creation")
	}
	u, err := r.GetByName("test")
	if err != nil {
		t.Fatalf("error finding user: %v", err)
	}
	log.Printf("%+v", u)
}

func TestUserRepository_Save(t *testing.T) {
	user := &model.User{
		Name:     "test",
		Password: "test",
		Nickname: "test",
		Avatar:   "test",
		Status:   1,
	}
	_ = r.DeleteByName("test")
	err := r.Create(user)
	if err != nil {
		t.Fatalf("error creating user: %v", err)
	}
	u, err := r.GetByName("test")
	if err != nil {
		t.Fatalf("error finding user: %v", err)
	}
	log.Printf("before: %+v", u)
	u.Nickname = "fuck"
	err = r.Update(u)
	if err != nil {
		t.Fatalf("error updating user: %v", err)
	}
	u, err = r.GetByName("test")
	if err != nil {
		t.Fatalf("error finding user: %v", err)
	}
	log.Printf("after: %+v", u)
}

func TestUserRepository_Update(t *testing.T) {
	user := &model.User{
		Name:     "test",
		Password: "test",
		Nickname: "test",
		Avatar:   "test",
		Status:   1,
	}
	_ = r.DeleteByName("test")
	err := r.Create(user)
	if err != nil {
		t.Fatalf("error creating user: %v", err)
	}
	u, err := r.GetByName("test")
	if err != nil {
		t.Fatalf("error finding user: %v", err)
	}
	log.Printf("before: %+v", u)
	err = r.UpdateNicknameAndAvatar(u.ID, "xxx", "asdfasf")
	if err != nil {
		t.Fatalf("error updating user: %v", err)
	}
	u, err = r.GetByName("test")
	if err != nil {
		t.Fatalf("error finding user: %v", err)
	}
	log.Printf("after: %+v", u)
}
