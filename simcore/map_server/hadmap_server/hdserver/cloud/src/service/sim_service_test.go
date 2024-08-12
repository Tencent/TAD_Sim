/*
 Copyright (c) 2020 Tencent.com, Inc. All Rights Reserved

	Author: sbrucezhang(sbrucezhang@tencent.com)
	Date: 2020/10/14 20:24:11
*/

package main_test

import (
	"encoding/json"
	"fmt"
	"io/ioutil"

	//"service/utils"
	"net/http"
	"testing"
	"time"
	"txSim/service/rest"
	"txSim/service/utils"

	"github.com/gorilla/mux"
	//"github.com/gorilla/mux"
	"gopkg.in/resty.v1"
)

var port = "9001"

var wait_for_rest_start_secs = time.Duration(1)

func startRestfulAPI() {

	utils.Print("Start restful api")

	router := mux.NewRouter()

	rest.Configure("./data/scenario", "./data/app")
	rest.Register(router)

	utils.Print("sim service handle registered.")

	//log.Infof("sim service starting listening on %v ...", serviceAddress)
	err := http.ListenAndServe("127.0.0.1:"+port, router)

	utils.Print("listen error:  ", err.Error())
	utils.Assert(err == nil, "list shoule not return error")

	utils.Print("start rest return? .")
}

func get_hadmap_scene_third_api_2() {
	time.Sleep(wait_for_rest_start_secs * time.Second)
	utils.Print("after sleep , do get hadmap scene test.")
	client := resty.New()

	testCB := func() {

		resp, _ := client.R().
			SetHeader("Content-Type", "application/x-www-form-urlencoded").
			//SetHeader(rest.AuthTokenKey, rest.TestToken).
			Get("http://127.0.0.1:" + port + "/api/hadmaps/open/v3/d2d_20190726.xodr?version=1632973527&bucket=simulation-dev-1251316161&mapFileKey=test/map/d2d_20190726.xodr&mapJsonKey=")

		utils.Assert(resp.StatusCode() == 200, fmt.Sprintf("Unexpected status code, expected  %d got %d, instead , reason %s", 200, resp.StatusCode(), string(resp.Body())))

		utils.Print("resp is: ", len(resp.Body()))

		fmt.Println(resp.Header().Get("Content-Type"))

	}

	utils.Print("xxxxxxxxxxxx")
	testCB()
}

func get_hadmap_scene_third_api() {
	time.Sleep(wait_for_rest_start_secs * time.Second)
	utils.Print("after sleep , do get hadmap scene test.")
	client := resty.New()

	testCB := func() {

		resp, _ := client.R().
			SetHeader("Content-Type", "application/x-www-form-urlencoded").
			//SetHeader(rest.AuthTokenKey, rest.TestToken).
			Get("http://127.0.0.1:" + port + "/api/hadmaps/open/v3/NASTway.xodr?version=1632973527&bucket=simulation-dev-1251316161&mapFileKey=maps/144115205301725161/20043-61318699da5bae0007806990/NASTway.xodr&mapJsonKey=")

		utils.Assert(resp.StatusCode() == 200, fmt.Sprintf("Unexpected status code, expected  %d got %d, instead , reason %s", 200, resp.StatusCode(), string(resp.Body())))

		utils.Print("resp is: ", string(resp.Body()))

		fmt.Println(resp.Header().Get("Content-Type"))

	}

	utils.Print("xxxxxxxxxxxx")
	testCB()
}

func get_hadmap_scene_api() {
	time.Sleep(wait_for_rest_start_secs * time.Second)
	utils.Print("after sleep , do get hadmap scene test.")
	client := resty.New()

	testCB := func() {

		resp, _ := client.R().
			SetHeader("Content-Type", "application/x-www-form-urlencoded").
			//SetHeader(rest.AuthTokenKey, rest.TestToken).
			Get("http://127.0.0.1:" + port + "/api/hadmaps/open/v3/23445.xodr?version=1634635360&bucket=simulation-dev-1251316161&mapFileKey=/maps/editor/cdb0272a9bc748c485063a7934fe37da/1573eb59ab914d6f99fdb80e4f3767db/23445.xodr&mapJsonKey=maps/editor/cdb0272a9bc748c485063a7934fe37da/1573eb59ab914d6f99fdb80e4f3767db/23445.xodr_json")

		utils.Assert(resp.StatusCode() == 200, fmt.Sprintf("Unexpected status code, expected  %d got %d, instead , reason %s", 200, resp.StatusCode(), string(resp.Body())))

		utils.Print("resp is: ", string(resp.Body()))

		fmt.Println(resp.Header().Get("Content-Type"))

	}

	utils.Print("xxxxxxxxxxxx")
	testCB()
}

func get_hadmap_scene_save_api() {
	time.Sleep(wait_for_rest_start_secs * time.Second)
	utils.Print("after sleep , do get hadmap scene test.")

	b, err := ioutil.ReadFile("/data/hadmap_server/hdserver/test/data/self_made.json") // just pass the file name
	utils.Assert(err == nil, " read file error")

	client := resty.New()

	testCB := func() {

		resp, _ := client.R().
			SetHeader("Content-Type", "application/json").
			SetBody(b).
			//SetHeader(rest.AuthTokenKey, rest.TestToken).
			Post("http://127.0.0.1:" + port + "/api/hadmap/save/v3")

		utils.Assert(resp.StatusCode() == 200, fmt.Sprintf("Unexpected status code, expected  %d got %d, instead , reason %s", 200, resp.StatusCode(), string(resp.Body())))

		tmp := make(map[string]string)
		json.Unmarshal([]byte(resp.Body()), &tmp)
		utils.Print("resp is: ", tmp)

		fmt.Println(resp.Header().Get("Content-Type"))

	}

	utils.Print("xxxxxxxxxxxx")
	testCB()
}

func hadmap_callback_api() {

	time.Sleep(wait_for_rest_start_secs * time.Second)
	utils.Print("after sleep , do hadmap_upload_cb test.")
	client := resty.New()

	pay_detail := make(map[string]interface{})
	pay_detail["name"] = "sihuan.sqlite"
	var detail []map[string]interface{}

	detail = append(detail, pay_detail)
	payload := make(map[string]interface{})
	payload["hadmaps"] = detail

	testCB := func() {

		b, _ := json.Marshal(payload)

		utils.Print("payload:", string(b))
		resp, _ := client.R().
			SetHeader("Content-Type", "application/json").
			SetBody(payload).
			//SetHeader(rest.AuthTokenKey, rest.TestToken).
			Post("http://127.0.0.1:9000/api/cloud/hadmaps/upload/callback")

		utils.Assert(resp.StatusCode() == 200, fmt.Sprintf("Unexpected status code, expected  %d got %d, instead , reason %s", 200, resp.StatusCode(), string(resp.Body())))

		utils.Print("resp is: ", string(resp.Body()))

	}

	testCB()

}

// GormProxyTest test for form client.
func TestRestAPI(t *testing.T) {
	//flags.sim_host = "simci.autocloud.map.qq.com"
	//conf_path := os.Args[len(os.Args)-2]
	//flags.YamlConfig = conf_path
	//tars.ServerConfigPath = os.Args[len(os.Args)-1]

	go get_hadmap_scene_third_api_2()
	//go hadmap_callback_api()
	//go get_hadmap_scene_save_api()
	utils.Print("after go get hadmap scene api.")
	startRestfulAPI()

}
