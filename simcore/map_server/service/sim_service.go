package main

import (
	"flag"
	"fmt"
	"net/http"
	"os"
	"path"
	"time"
	"txSim/scenario"
	"txSim/service/handler"
	"github.com/gorilla/mux"
	"github.com/sirupsen/logrus"
)

// log is the global logger instance for the application.
var log = logrus.New()

// Global variables for the application configuration.
var appDir string
var scenarioDir string
var serviceAddress string
var logDir string
var appInstallDir string

// init initializes the command-line flags for the application.
func init() {
	flag.StringVar(&appDir, "service_dir", ".", "The root directory of TADSIM back-end service.")
	flag.StringVar(&scenarioDir, "scenario_dir", ".", "The root directory of TADSIM scenario service.")
	flag.StringVar(&serviceAddress, "serv_addr", "127.0.0.1:9000",
		"The listenning address of this service. Default to 127.0.0.1:9000.")
	flag.StringVar(&logDir, "log_dir", "", "The log directory of this service. Default to the standard output.")
	flag.StringVar(&appInstallDir, "app_dir", ".", "The app directory of this service. Default is .")
}

// registerServices registers the service handlers for the model and config endpoints.
func registerServices(router *mux.Router) {
	modelHdlr := handler.NewModelHandler(log, appDir)
	router.HandleFunc("/models", modelHdlr.GetModelHandler).Methods("GET")
	router.HandleFunc("/models", modelHdlr.AddModelHandler).Methods("POST")

	configHdlr := handler.NewConfigHandler(log, appDir)
	router.HandleFunc("/config/modules", configHdlr.GetModuleHandler).Methods("GET")
	// allow CORS request from TADSim front-end.
	router.Use(handler.AccessControlMw)
}

// createLogFile creates a new log file with a timestamped filename in the specified log directory.
func createLogFile() *os.File {
	logFilePath := path.Join(logDir, fmt.Sprintf("service_%v.log", time.Now().Format("20060102150405")))
	f, err := os.Create(logFilePath)
	if err != nil {
		log.Fatalf("log file %v creation failed: %v", logFilePath, err)
	}
	return f
}

// main is the entry point of the application, initializing the server and handling signals for graceful shutdown.
func main() {
	flag.Parse()
	if len(logDir) > 0 {
		f := createLogFile()
		defer f.Close()
		log.SetOutput(f)
	}
	log.Infoln("TADSim service starting...")
	router := mux.NewRouter()

	scenario.Configure(scenarioDir, appInstallDir)
	scenario.Register(router)
	log.Infoln("scenario service handlers registered.")
	log.Infof("sim service starting listening on %v ...", serviceAddress)
	http.ListenAndServe(serviceAddress, router)
}
