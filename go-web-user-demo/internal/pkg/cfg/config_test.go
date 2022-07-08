package cfg

import (
	"log"
	"testing"
)

func TestReadHttpServerConfig(t *testing.T) {
	cfg, err := ReadHttpServerConfig("../../../config/http.yml")
	if err != nil {
		t.Fatalf("Fail to parse http config: %v", err)
	}
	log.Printf("%+v", cfg)
}

func TestReadTcpServerConfig(t *testing.T) {
	cfg, err := ReadTcpServerConfig("../../../config/tcp.yml")
	if err != nil {
		t.Fatalf("Fail to parse http config: %v", err)
	}
	log.Printf("%+v", cfg)
}
