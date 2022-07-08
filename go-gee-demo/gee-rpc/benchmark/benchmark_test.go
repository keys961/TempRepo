package benchmark

import (
	"context"
	"geerpc"
	"geerpc/registry"
	"log"
	"net"
	"net/http"
	"runtime"
	"sync"
	"sync/atomic"
	"testing"
	"time"
)

type Foo int

type Args struct{ Num1, Num2 int }

func (f *Foo) Sum(args Args, reply *int) error {
	*reply = args.Num1 + args.Num2
	return nil
}

// start registry
func startRegistry(wg *sync.WaitGroup) {
	l, _ := net.Listen("tcp", ":9999")
	registry.HandleHTTP()
	wg.Done()
	_ = http.Serve(l, nil)
}

// start rpc server
func startServer(addr chan string) {
	var foo Foo
	socket, _ := net.Listen("tcp", ":0")
	server := geerpc.NewServer()
	_ = server.Register(&foo)
	// register the server to the registry
	addr <- socket.Addr().String()
	server.Accept(socket) // blocking
}

func call(c *geerpc.Client, ctx context.Context, serviceMethod string, args *Args) {
	var reply int
	var err error
	err = c.Call(ctx, serviceMethod, args, &reply)
	if err != nil {
		log.Printf("%s error: %v", serviceMethod, err)
	}
}

func BenchmarkCallRpc(b *testing.B) {
	b.SetParallelism(runtime.NumCPU())
	addr := make(chan string)
	go startServer(addr)

	c, _ := geerpc.Dial("tcp", <-addr)
	defer func() { _ = c.Close() }()

	time.Sleep(time.Second / 2)
	b.ResetTimer()
	b.StartTimer()
	var count uint32 = 0
	b.RunParallel(func(pb *testing.PB) {
		for pb.Next() {
			call(c, context.Background(), "Foo.Sum", &Args{
				Num1: int(count),
				Num2: int(count),
			})
			atomic.AddUint32(&count, 1)
		}
	})
	b.StopTimer()
	log.Printf("benchmark call rpc finished. invoke times:%d", count)
}
