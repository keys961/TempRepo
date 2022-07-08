package session

import (
	"gee-orm/log"
	"reflect"
)

// Hooks constants
// The signature of hooks is: func (obj *Obj) hook(s *Session) error
const (
	BeforeQuery  = "BeforeQuery"
	AfterQuery   = "AfterQuery"
	BeforeUpdate = "BeforeUpdate"
	AfterUpdate  = "AfterUpdate"
	BeforeDelete = "BeforeDelete"
	AfterDelete  = "AfterDelete"
	BeforeInsert = "BeforeInsert"
	AfterInsert  = "AfterInsert"
)

// CallMethod calls the value's method with specified method name
func (s *Session) CallMethod(method string, value interface{}) {
	m := reflect.ValueOf(s.RefTable().Model).MethodByName(method)
	if value != nil {
		// use value as instance to invoke its method if value is not nil
		m = reflect.ValueOf(value).MethodByName(method)
	}

	param := []reflect.Value{reflect.ValueOf(s)}
	if m.IsValid() {
		if ret := m.Call(param); len(ret) > 0 {
			if err, ok := ret[0].Interface().(error); ok {
				log.Error(err)
			}
		}
	}
}
