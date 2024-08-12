module txSim/service

go 1.18

require (
	github.com/astaxie/beego v1.12.0
	//	txSim/scenario v0.0.0
	github.com/felixge/httpsnoop v1.0.1
	github.com/gammazero/workerpool v1.1.3
	github.com/gorilla/mux v1.7.3
	github.com/mattn/go-sqlite3 v1.10.0
	github.com/sirupsen/logrus v1.4.2
	github.com/spf13/pflag v1.0.5
	github.com/tidwall/gjson v1.14.4
	gopkg.in/resty.v1 v1.12.0
)

require (
	github.com/gammazero/deque v0.2.0 // indirect
	github.com/konsorten/go-windows-terminal-sequences v1.0.1 // indirect
	github.com/tidwall/match v1.1.1 // indirect
	github.com/tidwall/pretty v1.2.0 // indirect
	golang.org/x/net v0.0.0-20220722155237-a158d28d115b // indirect
	golang.org/x/sys v0.0.0-20220722155257-8c9f86f7a55f // indirect
	google.golang.org/appengine v1.6.8 // indirect
)

//replace txSim/scenario v0.0.0 => ./src
//replace txSim/rest v0.0.0 => ./rest
