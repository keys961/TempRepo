package session

import (
	"gee-orm/log"
	"testing"
)

type Account struct {
	ID       int `gee-orm:"PRIMARY KEY"`
	Password string
}

func (account *Account) BeforeInsert(_ *Session) error {
	log.Info("before insert: ", account)
	account.ID += 1000
	return nil
}

func (account *Account) AfterQuery(_ *Session) error {
	log.Info("after query: ", account)
	account.Password = "******"
	return nil
}

func TestSession_CallMethod(t *testing.T) {
	s := NewSession().Model(&Account{})
	_ = s.DropTable()
	_ = s.CreateTable()
	_, _ = s.Insert(&Account{1, "123456"}, &Account{2, "qwerty"})

	u := &Account{}

	err := s.First(u)
	if err != nil || u.ID != 1001 || u.Password != "******" {
		t.Fatal("Failed to call hooks after query, got: ", u)
	}
}
