package repository

import (
    "gorm.io/gorm"
)

type UserRepo struct {
    db *gorm.DB
}

func NewUserRepo(db *gorm.DB) *UserRepo {
    return &UserRepo{db: db}
}

func (r *UserRepo) Create(u *User) error {
    return r.db.Create(u).Error
}

func (r *UserRepo) GetByID(id uint) (*User, error) {
    var u User
    err := r.db.First(&u, id).Error
    if err != nil {
        return nil, err
    }
    return &u, nil
}

func (r *UserRepo) UpdateName(id uint, name string) error {
    return r.db.Model(&User{}).Where("id = ?", id).Update("name", name).Error
}

func (r *UserRepo) Delete(id uint) error {
    return r.db.Delete(&User{}, id).Error
}