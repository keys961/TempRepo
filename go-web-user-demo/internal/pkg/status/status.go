package status

import (
	"fmt"
	"net/http"
)

type Status struct {
	Code int    `json:"code"`
	Msg  string `json:"msg"`
}

var status = map[int]string{}

var (
	Ok = NewStatus(0, "OK")

	ErrServer          = NewStatus(1, "Server Error")
	ErrInvalidParams   = NewStatus(2, "Invalid Params")
	ErrNotFound        = NewStatus(3, "Not Found")
	ErrSessionNotFound = NewStatus(4, "User Session Required")

	ErrLogin         = NewStatus(11, "User Login Failed")
	ErrRegister      = NewStatus(12, "User Register Failed")
	ErrGetProfile    = NewStatus(13, "User Get Profile Failed")
	ErrUpdateProfile = NewStatus(14, "User Update Profile Failed")
	ErrUploadAvatar  = NewStatus(15, "Upload Picture Failed")
)

func NewStatus(code int, msg string) Status {
	if _, ok := status[code]; ok {
		panic(fmt.Sprintf("Errcode %d existed.", code))
	}
	status[code] = msg
	return Status{Code: code, Msg: msg}
}

func (e *Status) String() string {
	return fmt.Sprintf("code: %v, msg: %v", e.Code, e.Msg)
}

func (e *Status) HttpStatusCode() int {
	switch c := e.Code; {
	case c == Ok.Code:
		return http.StatusOK
	case c == ErrServer.Code:
		return http.StatusInternalServerError
	case c == ErrInvalidParams.Code:
		return http.StatusBadRequest
	case c == ErrNotFound.Code:
		return http.StatusNotFound
	case c == ErrSessionNotFound.Code:
		return http.StatusUnauthorized
	case c >= 10:
		// Business Error will be represented by retcode, not HTTP status code
		return http.StatusOK
	}
	return http.StatusInternalServerError
}
