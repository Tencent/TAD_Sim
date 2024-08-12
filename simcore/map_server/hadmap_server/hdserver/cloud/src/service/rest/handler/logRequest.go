package handler

import (
	"net/http"
	"strconv"
	"strings"
	"time"

	"github.com/felixge/httpsnoop"
	"github.com/sirupsen/logrus"
)

// Global logger instance for logging HTTP request information
var log *logrus.Logger

// HttpReqInfo struct to store HTTP request information
type HttpReqInfo struct {
	method    string
	uri       string
	referer   string
	ipaddr    string
	code      int
	size      int64
	duration  time.Duration
	userAgent string
}

// SetRequestInfoLogger sets the global logger instance for logging HTTP request information
func SetRequestInfoLogger(l *logrus.Logger) {
	log = l
}

// ExtractInfo extracts HTTP request information from the given http.Request
func (_this *HttpReqInfo) ExtractInfo(r *http.Request) {
	_this.method = r.Method
	_this.uri = r.URL.String()
	_this.referer = r.Header.Get("Referer")
	_this.userAgent = r.Header.Get("User-Agent")
	_this.ipaddr = _this.ExtractRemoteAddr(r)
}

// ipAddrFromRemoteAddr extracts the IP address from the given remote address string
func (_this *HttpReqInfo) ipAddrFromRemoteAddr(s string) string {
	idx := strings.LastIndex(s, ":")

	if idx == -1 {
		return s
	}

	return s[:idx]
}

// ExtractRemoteAddr extracts the remote IP address from the given http.Request
func (_this *HttpReqInfo) ExtractRemoteAddr(r *http.Request) string {
	hdr := r.Header
	hdrRealIP := hdr.Get("X-Real-IP")
	hdrForwardedFor := hdr.Get("X-Forwarded-For")

	if hdrRealIP == "" && hdrForwardedFor == "" {
		return _this.ipAddrFromRemoteAddr(r.RemoteAddr)
	}

	if hdrForwardedFor != "" {
		parts := strings.Split(hdrForwardedFor, ",")

		for i, p := range parts {
			parts[i] = strings.TrimSpace(p)
		}

		return parts[0]
	}

	return hdrRealIP
}

// RecordInfo logs the HTTP request information
func (_this *HttpReqInfo) RecordInfo() {
	log.Info("user agent: ", _this.userAgent)
	log.Info(_this.method, " ", _this.uri, " ip from:", _this.ipaddr, " referer: ", _this.referer)
	durMs := _this.duration / time.Millisecond
	log.Info("status code : ",
		strconv.Itoa(_this.code), " size: ", strconv.FormatInt(_this.size, 10),
		" request duration: ", strconv.FormatInt(int64(durMs), 10))
}

// LogRequestInfo wraps the given http.Handler to log HTTP request information
func LogRequestInfo(h http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		info := HttpReqInfo{}
		info.ExtractInfo(r)

		m := httpsnoop.CaptureMetrics(h, w, r)

		info.code = m.Code
		info.size = m.Written
		info.duration = m.Duration

		info.RecordInfo()
	})
}
