package handler

import (
	"net/http"
	"net/url"
	"strings"
)

// allowedOrigins 白名单：仅允许本地来源访问
var allowedOrigins = map[string]bool{
	"http://localhost":      true,
	"https://localhost":     true,
	"http://127.0.0.1":     true,
	"https://127.0.0.1":    true,
	"http://localhost:9000": true,
	"http://127.0.0.1:9000": true,
}

// isAllowedOrigin 检查 Origin 是否在白名单中
// 允许 localhost / 127.0.0.1 的任意端口
func isAllowedOrigin(origin string) bool {
	if allowedOrigins[origin] {
		return true
	}
	// 解析 Origin，允许 localhost / 127.0.0.1 的任意端口
	u, err := url.Parse(origin)
	if err != nil {
		return false
	}
	host := strings.Split(u.Host, ":")[0]
	return host == "localhost" || host == "127.0.0.1"
}

func CrossAccessControlHandler(h http.Handler) http.Handler {

	fn := func(w http.ResponseWriter, r *http.Request) {

		origin := r.Header.Get("Origin")

		if isAllowedOrigin(origin) {
			w.Header().Set("Access-Control-Allow-Credentials", "true")
			w.Header().Set("Access-Control-Allow-Origin", origin)
			w.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
			w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
			w.Header().Set("Access-Control-Max-Age", "1728000")
		}
		// 不在白名单中的 Origin 不设置 CORS 头，浏览器将拒绝跨域请求

		h.ServeHTTP(w, r)

	}

	return http.HandlerFunc(fn)
}
