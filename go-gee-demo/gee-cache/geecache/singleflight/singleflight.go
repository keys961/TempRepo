package singleflight

import "sync"

type call struct {
	wg sync.WaitGroup
	val interface{} // return val
	err error // return err
}

type Group struct {
	mu sync.Mutex // mutex to protect the call map
	m  map[string]*call // call with key -> call
}

// Do is the entry to execute fn binding with key without duplicate requests
func (g *Group) Do(key string, fn func()(interface{}, error)) (interface{}, error) {
	g.mu.Lock()

	if g.m == nil {
		g.m = make(map[string]*call)
	}

	if c, ok := g.m[key]; ok {
		// wait if request in progress
		// to avoid duplicate requests
		g.mu.Unlock()
		c.wg.Wait()
		return c.val, c.err
	}

	// create a call
	c := new(call)
	c.wg.Add(1)
	g.m[key] = c
	g.mu.Unlock()

	// execute the call
	c.val, c.err = fn()
	c.wg.Done()

	// remove in-progress request
	g.mu.Lock()
	delete(g.m, key)
	g.mu.Unlock()

	return c.val, c.err
}
