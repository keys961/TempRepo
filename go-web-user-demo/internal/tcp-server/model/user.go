package model

type User struct {
	ID       uint
	Name     string
	Password string
	Nickname string
	Avatar   string
	Status   uint8
}

func (*User) TableName() string {
	return "user"
}
