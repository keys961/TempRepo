package repository

import (
	"entry-task/internal/tcp-server/model"
	"gorm.io/gorm"
)

type UserRepository struct {
	engine *gorm.DB
}

func NewUserRepository(engine *gorm.DB) *UserRepository {
	return &UserRepository{
		engine: engine,
	}
}

func (r *UserRepository) Create(user *model.User) error {
	return r.engine.Create(user).Error
}

func (r *UserRepository) Update(user *model.User) error {
	return r.engine.Save(user).Error
}

func (r *UserRepository) UpdateNicknameAndAvatar(id uint, nickname string, avatar string) error {
	values := map[string]interface{}{}
	if nickname != "" {
		values["nickname"] = nickname
	}
	if avatar != "" {
		values["avatar"] = avatar
	}
	return r.engine.Model(&model.User{ID: id}).Updates(values).Error
}

func (r *UserRepository) DeleteByName(name string) error {
	return r.engine.Where("name = ?", name).Delete(&model.User{}).Error
}

func (r *UserRepository) GetByName(name string) (*model.User, error) {
	var user model.User
	err := r.engine.Where("name = ?", name).First(&user).Error
	if err != nil {
		return nil, err
	}
	return &user, nil
}
