package utils

import (
	"bytes"
	"entry-task/internal/pkg/hash"
	"io"
	"io/ioutil"
	"mime/multipart"
	"os"
	"path"
	"strings"
)

func HashedFileName(name string, username string) string {
	ext := FileExt(name)
	fileName := strings.TrimSuffix(name, ext)
	fileName = hash.MD5(fileName)
	return username + fileName + ext
}

func FileExt(name string) string {
	return path.Ext(name)
}

func CheckPath(dst string) bool {
	_, err := os.Stat(dst)
	return !os.IsNotExist(err)
}

func CheckPermission(dst string) bool {
	_, err := os.Stat(dst)
	return !os.IsPermission(err)
}

func CreatePath(dst string, perm os.FileMode) error {
	err := os.MkdirAll(dst, perm)
	if err != nil {
		return err
	}
	return nil
}

func SaveFile(file *multipart.FileHeader, dst string) error {
	src, err := file.Open()
	if err != nil {
		return err
	}
	defer closeSilently(src)

	out, err := os.Create(dst)
	if err != nil {
		return err
	}
	defer closeSilently(out)

	_, err = io.Copy(out, src)
	return err
}

func GetFileBytes(file *multipart.FileHeader) ([]byte, error) {
	src, err := file.Open()
	if err != nil {
		return nil, err
	}
	defer closeSilently(src)

	buf := bytes.NewBuffer(nil)
	if _, err := io.Copy(buf, src); err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func SaveFileBytes(file *[]byte, dst string) error {
	err := ioutil.WriteFile(dst, *file, 0644)
	return err
}

func closeSilently(f multipart.File) {
	_ = f.Close()
}
