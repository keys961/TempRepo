package rpc

import (
	"github.com/silenceper/pool"
	"time"
)

func NewClientPool(network, address string, max, maxIdle, idleTimeout int, opt *Option) (pool.Pool, error) {
	factory := func() (interface{}, error) {
		return Dial(network, address, opt)
	}
	release := func(v interface{}) error { return v.(*Client).Close() }

	poolConfig := &pool.Config{
		InitialCap:  0,
		MaxIdle:     maxIdle,
		MaxCap:      max,
		Factory:     factory,
		Close:       release,
		IdleTimeout: time.Duration(idleTimeout) * time.Second,
	}

	p, e := pool.NewChannelPool(poolConfig)
	if e != nil {
		return nil, e
	}
	return p, nil
}
