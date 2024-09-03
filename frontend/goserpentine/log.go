package main

import (
	"github.com/op/go-logging"
	"os"
)

var log = logging.MustGetLogger("goserpentine")

func initLogger() {
	backend := logging.NewLogBackend(os.Stdout, "", 0)
	formatter := logging.NewBackendFormatter(backend, logging.MustStringFormatter(
		`%{color}%{time:15:04:05.000} %{shortfunc} ▶ %{level:.4s} %{id:03x}%{color:reset} %{message}`,
	))
	logging.SetBackend(formatter)
}
