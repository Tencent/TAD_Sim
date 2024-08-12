module txSim/service

go 1.12

require (
	github.com/astaxie/beego v1.12.0
	github.com/gorilla/mux v1.7.3
	github.com/mattn/go-sqlite3 v1.10.0
	github.com/sirupsen/logrus v1.4.2
	google.golang.org/appengine v1.6.8 // indirect
	txSim/scenario v0.0.0
)

replace txSim/scenario v0.0.0 => ../hadmap_server/src
