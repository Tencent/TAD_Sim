/*
 Copyright (c) 2020 Tencent.com, Inc. All Rights Reserved

	Author: sbrucezhang(sbrucezhang@tencent.com)
	Date: 2020/10/14 20:24:11

*/
package utils

/*import (
	"fmt"
	"io"
	"os"
	"path"
	"runtime"
	"sync"

	"metric/config"
	//"filepath"
	//"metric/monitor"

	nested "github.com/antonfisher/nested-logrus-formatter"
	"github.com/sirupsen/logrus"
	formatter "github.com/zput/zxcTool/ztLog/zt_formatter"
	lumberjack "gopkg.in/natefinch/lumberjack.v2"
)

var (
	logLevels = map[string]logrus.Level{
		"TRACE": logrus.TraceLevel,
		"DEBUG": logrus.DebugLevel,
		"INFO":  logrus.InfoLevel,
		"WARN":  logrus.WarnLevel,
	}

	myFormatter *formatter.ZtFormatter = &formatter.ZtFormatter{
		CallerPrettyfier: func(f *runtime.Frame) (string, string) {
			filename := path.Base(f.File)
			return f.Function, fmt.Sprintf("%s:%d", filename, f.Line)
		},
		Formatter: nested.Formatter{
			HideKeys:     false,
			TrimMessages: true,
			FieldsOrder: []string{
				"component", "category", "filename",
				"network-type", "role", "target", "prefix", "address", "timeout-ms",
				"method", "url", "from", "request", "data", "statusCode", "response",
				"job-type", "job-id", "job-user", "job-stage", "job-is-retry",
				"error",
			},
		},
	}

	// Log is the logger object to be used
	Logger *logrus.Logger = func(f logrus.Formatter) *logrus.Logger {
		l := logrus.New()
		l.SetReportCaller(true)
		l.SetFormatter(f)
		l.SetLevel(logrus.DebugLevel)
		return l
	}(myFormatter)

	logger  *logrus.Entry //= Logger.WithField("Service", "daan_metric")
	logOnce sync.Once
)

func GetLogger() *logrus.Entry {

	if logger == nil {
		logOnce.Do(func() {
			logger = Logger.WithField("Service", "daan_metric")
		})
	}
	return logger
}

func InitLog(conf *config.Yaml) {
	_ = GetLogger()

	logRotate := &lumberjack.Logger{
		Filename:   conf.Log.FileName,
		MaxSize:    conf.Log.MaxSize, // MB
		MaxBackups: 0,
		MaxAge:     conf.Log.MaxAge, //days
		Compress:   false,
	}

	if conf.Log.AlsoToStderr {
		logWriter := io.MultiWriter(os.Stderr, logRotate)
		Logger.SetOutput(logWriter)
	} else {
		Logger.SetOutput(logRotate)
	}
	if level, ok := logLevels[conf.Log.Level]; ok {
		Logger.SetLevel(level)
	} else {
		fmt.Println("unknown log level")
		logger.Fatalf("Unrecognized log level %v", conf.Log.Level)
	}

	//LogRotateMonitor(conf)
}

*/
