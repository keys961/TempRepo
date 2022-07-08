package cfg

import (
	"io/ioutil"
	"time"
)
import "gopkg.in/yaml.v2"

type ServerConfig struct {
	Host         string        `yaml:"host"`
	Port         int           `yaml:"port"`
	ReadTimeout  time.Duration `yaml:"read_timeout"`
	WriteTimeout time.Duration `yaml:"write_timeout"`
}

type AppConfig struct {
	LogFile              string   `yaml:"log_file"`
	UploadSavePath       string   `yaml:"upload_save_path"`
	UploadServerUrl      string   `yaml:"upload_server_url"`
	UploadImageMaxSize   int      `yaml:"upload_image_max_size"`
	UploadImageAllowExts []string `yaml:"upload_image_allow_exts"`
}

type MySqlConfig struct {
	Host         string `yaml:"host"`
	Username     string `yaml:"username"`
	Password     string `yaml:"password"`
	Db           string `yaml:"db"`
	Charset      string `yaml:"charset"`
	ParseTime    bool   `yaml:"parse_time"`
	MaxIdleConns int    `yaml:"max_idle_conns"`
	MaxOpenConns int    `yaml:"max_open_conns"`
}

type RedisConfig struct {
	Host string `yaml:"host"`
	Db   int    `yaml:"db"`
}

type RpcClientConfig struct {
	Host        string `yaml:"host"`
	MaxConn     int    `yaml:"maxConn"`
	MaxIdle     int    `yaml:"maxIdle"`
	MaxIdleTime int    `yaml:"maxIdleTime"`
}

type HttpServerConfig struct {
	Server ServerConfig    `yaml:"server"`
	App    AppConfig       `yaml:"app"`
	Redis  RedisConfig     `yaml:"redis"`
	Rpc    RpcClientConfig `yaml:"rpc"`
}

type TcpServerConfig struct {
	Server ServerConfig `yaml:"server"`
	App    AppConfig    `yaml:"app"`
	Mysql  MySqlConfig  `yaml:"mysql"`
	Redis  RedisConfig  `yaml:"redis"`
}

func ReadHttpServerConfig(path string) (*HttpServerConfig, error) {
	conf := new(HttpServerConfig)
	f, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}
	err = yaml.Unmarshal(f, conf)
	if err != nil {
		return nil, err
	}
	return conf, nil
}

func ReadTcpServerConfig(path string) (*TcpServerConfig, error) {
	conf := new(TcpServerConfig)
	f, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}
	err = yaml.Unmarshal(f, conf)
	if err != nil {
		return nil, err
	}
	return conf, nil
}
