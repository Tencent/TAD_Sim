package rest_test

import (
	"crypto/tls"
	"io"
	"log"
	"net/http"
	"os"
	"testing"
	"time"
)

func startHTTPS() {
	http.HandleFunc("/", func(w http.ResponseWriter, req *http.Request) {
		io.WriteString(w, "hello, world!\n")
	})
	if e := http.ListenAndServeTLS("127.0.0.1:443", "/home/brucesz/work/brucesz/git/tad_sim/hadmap_server/resources/server.crt", "/home/brucesz/work/brucesz/git/tad_sim/hadmap_server/resources/server.key", nil); e != nil {
		log.Fatal("ListenAndServe: ", e)
	}
}

func insecreClient() {
	time.Sleep(time.Second * 3)
	c := &http.Client{
		Transport: &http.Transport{
			TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
		}}

	if resp, e := c.Get("https://localhost"); e != nil {
		log.Fatal("http.Client.Get: ", e)
	} else {
		defer resp.Body.Close()
		io.Copy(os.Stdout, resp.Body)
	}
}

func TestHTTPs(t *testing.T) {
	//insecreClient()
	startHTTPS()

}
