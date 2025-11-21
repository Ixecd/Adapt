package repository

type User struct {
    ID    uint   `gorm:"primaryKey"`
    Name  string
    Email string `gorm:"uniqueIndex"`
}

func (User) TableName() string { return "users" }