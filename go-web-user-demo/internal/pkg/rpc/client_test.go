package rpc

import (
	"context"
	"encoding/gob"
	"fmt"
	"net"
	"strings"
	"testing"
	"time"
)

type Bar int

func (b *Bar) Timeout(_ int, reply *int) error {
	time.Sleep(time.Second * 2)
	*reply = 1
	return nil
}

type Arg struct {
	A int
	B string
}

type Ret struct {
	A int
	B string
}

func (b *Bar) Echo(argv Arg, reply *Ret) error {
	reply.A = argv.A
	reply.B = argv.B
	return nil
}

func startServer(addr chan string) {
	var b Bar
	_ = Register(&b)
	// pick a free port
	l, _ := net.Listen("tcp", ":0")
	addr <- l.Addr().String()
	Accept(l)
}

func TestClient_Call(t *testing.T) {
	t.Parallel()
	addrCh := make(chan string)
	go startServer(addrCh)
	addr := <-addrCh
	time.Sleep(time.Second)
	t.Run("client timeout", func(t *testing.T) {
		client, _ := Dial("tcp", addr)
		ctx, _ := context.WithTimeout(context.Background(), time.Second)
		var reply int
		err := client.Call(ctx, "Bar.Timeout", 1, &reply)
		_assert(err != nil && strings.Contains(err.Error(), ctx.Err().Error()), "expect a timeout error")
	})
	t.Run("server handle timeout", func(t *testing.T) {
		client, _ := Dial("tcp", addr, &Option{
			HandleTimeout: time.Second,
		})
		var reply int
		err := client.Call(context.Background(), "Bar.Timeout", 1, &reply)
		_assert(err != nil && strings.Contains(err.Error(), "handle timeout"), "expect a timeout error")
	})
	gob.Register(Arg{})
	gob.Register(Ret{})
	t.Run("client echo", func(t *testing.T) {
		client, _ := Dial("tcp", addr)
		ctx := context.Background()
		var reply Ret
		err := client.Call(ctx, "Bar.Echo", Arg{
			A: 14,
			B: "14",
		}, &reply)
		fmt.Printf("%v\n", reply)
		_assert(err == nil && reply.A == 14, "expect a reply = 14")
	})
}
