package repository

import (
    "database/sql"
    "regexp"
    "testing"

    sqlmock "github.com/DATA-DOG/go-sqlmock"
    "gorm.io/driver/mysql"
    "gorm.io/gorm"
)

func newMockGorm(t *testing.T) (*gorm.DB, sqlmock.Sqlmock, *sql.DB) {
    db, mock, err := sqlmock.New()
    if err != nil {
        t.Fatalf("sqlmock.New error: %v", err)
    }
    gdb, err := gorm.Open(mysql.New(mysql.Config{Conn: db, SkipInitializeWithVersion: true}), &gorm.Config{})
    if err != nil {
        t.Fatalf("gorm.Open error: %v", err)
    }
    return gdb, mock, db
}

func TestUserRepo_Create(t *testing.T) {
    gdb, mock, sqlDB := newMockGorm(t)
    defer sqlDB.Close()

    repo := NewUserRepo(gdb)

    mock.ExpectBegin()
    mock.ExpectExec(regexp.QuoteMeta("INSERT INTO `users` (`name`,`email`) VALUES (?,?)")).
        WithArgs("Alice", "alice@example.com").
        WillReturnResult(sqlmock.NewResult(1, 1))
    mock.ExpectCommit()

    u := &User{Name: "Alice", Email: "alice@example.com"}
    if err := repo.Create(u); err != nil {
        t.Fatalf("Create error: %v", err)
    }
    if err := mock.ExpectationsWereMet(); err != nil {
        t.Fatalf("unmet expectations: %v", err)
    }
}

func TestUserRepo_GetByID(t *testing.T) {
    gdb, mock, sqlDB := newMockGorm(t)
    defer sqlDB.Close()

    repo := NewUserRepo(gdb)

    rows := sqlmock.NewRows([]string{"id", "name", "email"}).AddRow(1, "Alice", "alice@example.com")
    mock.ExpectQuery(regexp.QuoteMeta("SELECT * FROM `users` WHERE `users`.`id` = ? ORDER BY `users`.`id` LIMIT 1")).
        WithArgs(1).
        WillReturnRows(rows)

    got, err := repo.GetByID(1)
    if err != nil {
        t.Fatalf("GetByID error: %v", err)
    }
    if got.ID != 1 || got.Name != "Alice" || got.Email != "alice@example.com" {
        t.Fatalf("unexpected result: %+v", got)
    }
    if err := mock.ExpectationsWereMet(); err != nil {
        t.Fatalf("unmet expectations: %v", err)
    }
}

func TestUserRepo_UpdateName(t *testing.T) {
    gdb, mock, sqlDB := newMockGorm(t)
    defer sqlDB.Close()

    repo := NewUserRepo(gdb)

    mock.ExpectBegin()
    mock.ExpectExec(regexp.QuoteMeta("UPDATE `users` SET `name`=? WHERE id = ?")).
        WithArgs("Bob", 1).
        WillReturnResult(sqlmock.NewResult(0, 1))
    mock.ExpectCommit()

    if err := repo.UpdateName(1, "Bob"); err != nil {
        t.Fatalf("UpdateName error: %v", err)
    }
    if err := mock.ExpectationsWereMet(); err != nil {
        t.Fatalf("unmet expectations: %v", err)
    }
}

func TestUserRepo_Delete(t *testing.T) {
    gdb, mock, sqlDB := newMockGorm(t)
    defer sqlDB.Close()

    repo := NewUserRepo(gdb)

    mock.ExpectBegin()
    mock.ExpectExec(regexp.QuoteMeta("DELETE FROM `users` WHERE `users`.`id` = ?")).
        WithArgs(1).
        WillReturnResult(sqlmock.NewResult(0, 1))
    mock.ExpectCommit()

    if err := repo.Delete(1); err != nil {
        t.Fatalf("Delete error: %v", err)
    }
    if err := mock.ExpectationsWereMet(); err != nil {
        t.Fatalf("unmet expectations: %v", err)
    }
}