package main

import (
	flag "github.com/spf13/pflag"
	"fmt"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"context"
	"path"
	"time"
	"txSim/service/handler"
	"txSim/service/rest"

	"github.com/gorilla/mux"
	"github.com/sirupsen/logrus"
	"github.com/gammazero/workerpool"
)

var log = logrus.New()

var appDir string
var scenarioDir string
var serviceAddress string
var logDir string
var appInstallDir string
var scenarioWorkerPoolSize int

func init() {
	flag.CommandLine.ParseErrorsWhitelist = flag.ParseErrorsWhitelist{ UnknownFlags: true }
	flag.StringVar(&appDir, "service_dir", ".",
	"The root directory of TADSIM back-end service.")
	flag.StringVar(&scenarioDir, "scenario_dir", ".", 
	"The root directory of TADSIM scenario service.")
	flag.StringVar(&serviceAddress, "serv_addr", "127.0.0.1:9000", 
	"The listenning address of this service. Default to 127.0.0.1:9000.")
	flag.StringVar(&logDir, "log_dir", "", 
	"The log directory of this service. Default to the standard output.")
	flag.StringVar(&appInstallDir, "app_dir", ".", 
	"The app directory of this service. Default is .")
	flag.IntVar(&scenarioWorkerPoolSize, "scenario_worker_pool_size", 16, 
	"Limits the concurrency of xosc2sim(rec).")
}

// registerServices registers the service handlers for the model and config endpoints.
func registerServices(router *mux.Router) {
    modelHdlr := handler.NewModelHandler(log, appDir)
    router.HandleFunc("/models", modelHdlr.GetModelHandler).Methods("GET")
    router.HandleFunc("/models", modelHdlr.AddModelHandler).Methods("POST")

    configHdlr := handler.NewConfigHandler(log, appDir)
    router.HandleFunc("/config/modules", configHdlr.GetModuleHandler).Methods("GET")

    router.Use(handler.AccessControlMw) // allow CORS request from TADSim front-end.
}

// createLogFile creates a new log file with a timestamped filename in the specified log directory.
func createLogFile() *os.File {
    logFilePath := path.Join(logDir, fmt.Sprintf("service_%v.log", time.Now().Format("20060102150405")))
    f, err := os.Create(logFilePath)
    if err != nil {
        log.Fatalf("log file %v creation failed: %v
", logFilePath, err)
    }
    return f
}

// main is the entry point of the application, 
// initializing the server and handling signals for graceful shutdown.
func main() {
    flag.Parse()
    if len(logDir) > 0 {
        f := createLogFile()
        defer f.Close()
        log.SetOutput(f)
    }
    log.Infoln("TADSim service starting...")
    router := mux.NewRouter()

    rest.Configure(scenarioDir, appInstallDir)
    rest.Register(router)
    rest.ScenarioOptionsWorkerPool = workerpool.New(scenarioWorkerPoolSize)
    log.Infoln("scenario service handlers registered.")

    // Uncomment the following lines to register the sim service handlers.
    // registerServices(router)
    // log.Infoln("sim service handlers registered.")

    server := &http.Server{Addr: serviceAddress, Handler: router}
    go func() {
        if err := server.ListenAndServe(); err != nil {
            if err != http.ErrServerClosed {
                log.Println(err)
            }
        }
    }()
    log.Infof("sim service starting listening on %v ...", serviceAddress)

    quit := make(chan os.Signal)
    signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
    sig := <-quit
    log.Printf("sim service received signal: %v
", sig)
    log.Println("sim service shutting down.")
    if err := server.Shutdown(context.Background()); err != nil {
        log.Println(err)
    }
    rest.ScenarioOptionsWorkerPool.StopWait()
    log.Println("sim service shutted down.")
}
