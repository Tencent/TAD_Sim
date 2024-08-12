package handler

import(
	"net/http"
)

func CrossAccessControlHandler(h http.Handler) http.Handler {

	fn := func(w http.ResponseWriter, r* http.Request) {

		origin := r.Header.Get("Origin")

		if len(origin) < 1 {
			origin = "*"
		}
		w.Header().Set("Access-Control-Allow-Credentials", "true")
		w.Header().Set("Access-Control-Allow-Origin", origin)
		/*
		w.Header().Set("Access-Control-Allow-Methods", "POST")
		w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
		w.Header().Set("Access-Control-Max-Age", "1728000")
		*/

		h.ServeHTTP(w, r)

	}

	return http.HandlerFunc(fn)
}
