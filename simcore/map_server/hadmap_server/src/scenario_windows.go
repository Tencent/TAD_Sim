package scenario

import "C"

import (
	//	"errors"
	"archive/zip"
	"bytes"
	"compress/gzip"
	"fmt"
	"io"
	"math"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/sirupsen/logrus"

	//"golang.org/x/net/websocket"
	"encoding/json"
	"io/ioutil"
	"os/exec"
	"strconv"
	"sync"
	"syscall"
	"txSim/scenario/handler"
	SW "txSim/scenario/scenewrapper"

	"github.com/gorilla/mux"
	"github.com/tidwall/gjson"
)

var sw = SW.SceneWrapper{}

var wg sync.WaitGroup

var staticfs http.Handler

var log = logrus.New()

var appRootPath string

var appInstallPath string

var startedTime time.Time

var (
	value             int // 前端给的状态
	taskstatu         int // 语义生成进程返回的状态
	nonScenariosCount int
	genScenariosCount int
	genDir            string
	percentFinshed    float32
	valueMux          sync.Mutex
	cond              *sync.Cond
)

// ProcessSingleton 定义一个结构体，用于存储进程信息
type ProcessSingleton struct {
	cmd *exec.Cmd
}

// 定义一个包级别的私有变量，用于存储单例对象
var processInstance *ProcessSingleton
var once sync.Once

// GetProcessInstance 用于获取单例对象
func GetProcessInstance() *ProcessSingleton {
	once.Do(func() {
		processInstance = &ProcessSingleton{}
	})
	return processInstance
}

// StartProcess 启动一个带参数的进程
func (p *ProcessSingleton) StartProcess(command string, args ...string) error {

	if p.cmd != nil && p.cmd.Process != nil {
		log.Info("Process is start")
		return nil // 进程已经启动
	}

	p.cmd = exec.Command(command, args...)
	p.cmd.Stdout = os.Stdout
	p.cmd.Stderr = os.Stderr

	if err := p.cmd.Start(); err != nil {
		return err
	}

	return nil
}

// StopProcess 主动关闭进程
func (p *ProcessSingleton) StopProcess() error {
	if p.cmd == nil || p.cmd.Process == nil {
		log.Info("Process is not start")
		p.cmd = nil
		return nil // 进程未启动
	}

	if err := p.cmd.Process.Signal(syscall.SIGTERM); err != nil {
		p.cmd = nil
		return err
	}

	if _, err := p.cmd.Process.Wait(); err != nil {
		p.cmd = nil
		return err
	}
	p.cmd.Process = nil
	p.cmd = nil
	return nil
}

func init() {
	value = 0
	percentFinshed = 0.0
	cond = sync.NewCond(&valueMux)
	// fmt.Println("init start......")
	// log.SetFormatter(&logrus.TextFormatter{})
	// log.SetLevel(logrus.DebugLevel)
	// log.SetOutput(os.Stdout)
	// sw.Initlize()
	// fmt.Println("init end......")

	// pwd, _ := os.Getwd()
	// fmt.Println(filepath.Join(pwd, "/models"))

	// staticfs = http.FileServer(http.Dir(filepath.Join(pwd, "/models")))

}

func staticFiles(w http.ResponseWriter, r *http.Request) {

	log.Info("staticFiles")
	log.Info(r.URL.Path)
	log.Info(r.URL.Path[1:])
	pwd, _ := os.Getwd()
	log.Info(pwd)
	pa := filepath.Join(pwd, "/static/")
	log.Info(pa)
	pp := filepath.Join(pwd, "/static/", r.URL.Path[1:])
	log.Info(pp)
	//w.Header().Set("Access-Control-Allow-Origin", "*")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	//old := r.URL.Path

	//r.URL.Path = strings.Replace(old, "/statics", "/models", 1)

	fmt.Println(r.URL.Path)
	fmt.Println(r.URL.Path[1:])

	http.ServeFile(w, r, r.URL.Path[1:])
	//staticfs.ServeHTTP(w, r, r.URL.Path[1:])
}

func compressData(s string) (*bytes.Buffer, error) {

	buf := new(bytes.Buffer)
	if false {
		gw := gzip.NewWriter(buf)
		defer gw.Close()

		leng, err := gw.Write([]byte(s))
		if err != nil {
			log.Error("write string to buf error!")
			return buf, err
		}

		log.Info("written data length: ", leng)

		err = gw.Flush()
		if err != nil {
			log.Error("flush to gzip writer error!")
			return buf, err
		}

		gw.Close()

		return buf, nil

	} else {

		buf.WriteString(s)
		return buf, nil
	}
}

func loadSensor() string {

	var sensorFile string
	sensorFile = appRootPath + "/sensor_preset"

	if !Exists(sensorFile) {
		return ""
	}

	file, err := os.Open(sensorFile)
	if err != nil {
		log.Error("open sensor file error!")
		return ""
	}

	defer file.Close()
	content, err := ioutil.ReadAll(file)

	return string(content)
}

func saveSensor(content []byte) bool {

	var sensorFile = appRootPath + "/sensor_preset"
	err := ioutil.WriteFile(sensorFile, content, 0644)
	if err != nil {
		return false
	}

	return true
}

func loadSensorPreset(w http.ResponseWriter, r *http.Request) {
	log.Info("loadSensorPreset start")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	content := loadSensor()
	fmt.Fprintf(w, "%s", content)
	log.Info("loadSensorPreset end")
}

func optionsHandler(w http.ResponseWriter, r *http.Request) {

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	//w.Header().Set("Access-Control-Allow-Methods", "DELETE,POST,GET,OPTIONS")
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func saveSensorPresetOptionsHandler(w http.ResponseWriter, r *http.Request) {

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Methods", "POST")
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	w.Header().Set("Access-Control-Max-Age", "1728000")
}

func saveSensorPreset(w http.ResponseWriter, r *http.Request) {
	log.Info("saveSensorPreset start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("parse sensor preset error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	var ret = saveSensor(body)

	if ret {
		fmt.Fprintf(w, "{'code':0}")
	} else {
		fmt.Fprintf(w, "{'code':-1}")
	}

	log.Info("saveSensorPreset end")
}

func loadSensorGlobalConfig(w http.ResponseWriter, r *http.Request) {
	log.Info(" loadSensorGlobalConfig start")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.LoadGlobalSensor()

	fmt.Fprintln(w, str)

	log.Info("loadSensorGlobalConfig end")
}

func saveSensorGlobalConfigHandler(w http.ResponseWriter, r *http.Request) {

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Methods", "POST")
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	w.Header().Set("Access-Control-Max-Age", "1728000")
}

func saveSensorGlobalConfig(w http.ResponseWriter, r *http.Request) {
	log.Info("saveSensorGlobalConfig start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("parse sensor preset error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	str := sw.SaveGlobalSensor(string(body))

	fmt.Fprintf(w, str)

	log.Info("saveSensorGlobalConfig end")
}

func loadEnvironmentGlobalConfig(w http.ResponseWriter, r *http.Request) {
	log.Info(" loadEnvironmentGlobalConfig start")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.LoadGlobalEnvironment()

	fmt.Fprintln(w, str)

	log.Info("loadEnvironmentGlobalConfig end")
}

func saveEnvironmentGlobalConfigHandler(w http.ResponseWriter, r *http.Request) {

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Methods", "POST")
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	w.Header().Set("Access-Control-Max-Age", "1728000")
}

func saveEnvironmentGlobalConfig(w http.ResponseWriter, r *http.Request) {
	log.Info("saveEnvironmentGlobalConfig start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("parse environment global error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	str := sw.SaveGlobalEnvironment(string(body))

	fmt.Fprintf(w, str)

	log.Info("saveEnvironmentGlobalConfig end")
}

func getConfigurationHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("request configure info")
	origin := r.Header.Get("Origin")

	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetConfiguration()

	fmt.Fprintln(w, str)
}

func getL3StateMachineHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request l3 state machine info")
	origin := r.Header.Get("Origin")

	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetL3StateMachine()

	fmt.Fprintln(w, str)
}

func getCatalogsList(w http.ResponseWriter, r *http.Request) {
	log.Info("get catalog list ")
	origin := r.Header.Get("Origin")
	log.Debug(origin)
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	str := sw.GetCatalogList()
	fmt.Fprintln(w, str)
}
func CatalogsAdd(w http.ResponseWriter, r *http.Request) {
	log.Info("CatalogsAdd start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"uploadGISImagesHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.AddCatalog(string(body))

	fmt.Fprint(w, str)

	log.Info("CatalogsAdd end")

}

func CatalogsUpdate(w http.ResponseWriter, r *http.Request) {

	log.Info("CatalogsUpdate start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"uploadGISImagesHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.UpdateCatalog(string(body))

	fmt.Fprint(w, str)

	log.Info("CatalogsUpdate end")

}

func CatalogsDelete(w http.ResponseWriter, r *http.Request) {

	log.Info("CatalogsDelete start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"uploadGISImagesHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}
	str := sw.DeleteCatalog(string(body))
	fmt.Fprint(w, str)
	log.Info("CatalogsDelete end")

}

func sensorLoad(w http.ResponseWriter, r *http.Request) {
	log.Info("load sensor list ")
	origin := r.Header.Get("Origin")
	log.Debug(origin)
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	str := sw.SensorLoad()
	fmt.Fprintln(w, str)
}
func sensorSave(w http.ResponseWriter, r *http.Request) {

	log.Info("sensorSave start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"sensorSave parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}
	str := sw.SensorSave(string(body))
	fmt.Fprint(w, str)
	log.Info("sensorSave end")

}

func sensorAdd(w http.ResponseWriter, r *http.Request) {

	log.Info("sensorAdd start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"sensorSave parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}
	str := sw.SensorAdd(string(body))
	fmt.Fprint(w, str)
	log.Info("sensorSave end")

}

func sensorUpdate(w http.ResponseWriter, r *http.Request) {

	log.Info("sensorUpdate start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"sensorUpdate parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}
	str := sw.SensorUpdate(string(body))
	fmt.Fprint(w, str)
	log.Info("sensorUpdate end")

}

func sensorDelete(w http.ResponseWriter, r *http.Request) {

	log.Info("sensorDelete start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"sensorSave parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}
	str := sw.SensorDelete(string(body))
	fmt.Fprint(w, str)
	log.Info("sensorDelete end")

}

func sensorGroupLoad(w http.ResponseWriter, r *http.Request) {
	log.Info("load sensor group list ")
	origin := r.Header.Get("Origin")
	log.Debug(origin)
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	str := sw.SensorGroupLoad()
	fmt.Fprintln(w, str)
}

func sensorGroupAdd(w http.ResponseWriter, r *http.Request) {

	log.Info("SensorGroupAdd start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"SensorGroupAdd parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}
	str := sw.SensorGroupAdd(string(body))
	fmt.Fprint(w, str)
	log.Info("SensorGroupAdd end")

}

func sensorGroupDelete(w http.ResponseWriter, r *http.Request) {

	log.Info("sensorGroupDelete start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"sensorGroupDelete parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}
	str := sw.SensorGroupDelete(string(body))
	fmt.Fprint(w, str)
	log.Info("sensorGroupDelete end")

}

func dynamicLoad(w http.ResponseWriter, r *http.Request) {
	log.Info("load dynamic list")
	origin := r.Header.Get("Origin")
	log.Debug(origin)
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	str := sw.DynamicLoad()
	fmt.Fprintln(w, str)
}

func dynamicSave(w http.ResponseWriter, r *http.Request) {

	log.Info("dynamicSave start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"dynamicSave parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}
	str := sw.DynamicSave(string(body))
	fmt.Fprint(w, str)
	log.Info("dynamicSave end")

}
func dynamicDelete(w http.ResponseWriter, r *http.Request) {

	log.Info("dynamicDelete start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"dynamicDelete parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}
	str := sw.DynamicDelete(string(body))
	fmt.Fprint(w, str)
	log.Info("dynamicDelete end")

}

func getHadmapListHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request hadmap list ")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetMapList()

	fmt.Fprintln(w, str)
}

func getHadmapinfoListHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("request hadmap info list")
	origin := r.Header.Get("Origin")

	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetMapinfoList()

	fmt.Fprintln(w, str)
}

func getHadmapsProgressvalHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("request getHadmapsProgressval")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetHadmapsProgress()

	fmt.Fprint(w, str)
}

func stopHadmapsProgressHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("request stopHadmapsProgress")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.StopHadmapsProgress()

	fmt.Fprintln(w, str)
}

func importSceneTemplateHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request import scene template")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	r.ParseForm()
	if len(r.Form["tpl"]) > 0 {
		var tpl = r.Form["tpl"][0]

		str := sw.ImportSceneTemplate(tpl)
		fmt.Fprintln(w, str)

	} else {
		fmt.Fprintln(w, "parameter error!")
		log.Info("import scene template error, no template name")
	}

	log.Info("request import scene template end")
}

func getSceneContentHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request scene content ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	r.ParseForm()
	if len(r.Form["sceneName"]) > 0 {
		var sceneName = r.Form["sceneName"][0]

		str := sw.GetScene(sceneName)

		fmt.Fprintln(w, str)

	} else {
		fmt.Fprintln(w, "parameter error!")
		log.Info("get scene content error, no scene name")
	}
}

func getSceneContentOptionsHandlerV2(w http.ResponseWriter, r *http.Request) {

	log.Info("request get scene options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func getSceneContentHandlerV2(w http.ResponseWriter, r *http.Request) {

	log.Info("request get scene v2. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)

	if err != nil {

		log.Error("read save scene body error!")
		return
	}

	str := sw.GetSceneV2(string(body))

	fmt.Fprintln(w, str)
	log.Info("request get scene v2 end.")
}

func saveSceneContentOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request save scene options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func saveSceneContentHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request save scene. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	//w.Header().Set("Access-Control-Allow-Headers", "Content-Type")

	body, err := ioutil.ReadAll(r.Body)

	if err != nil {

		log.Error("read save scene body error!")
		return
	}
	//strJson := string(body)

	//fmt.Println(strJson)

	var cont map[string]interface{}

	err = json.Unmarshal(body, &cont)
	if err != nil {
		log.Error("save scene unmarshal json data error!")
		return
	}

	var sceneName = cont["name"].(string)
	var sceneContent = cont["content"].(string)

	if len(sceneName) < 1 {
		fmt.Fprintln(w, "save scene param error")
		log.Info("save scene: no scene content")
		return
	}

	if len(sceneContent) < 1 {

		fmt.Fprintln(w, "save scene param error")
		log.Info("save scene: no scene name")
		return
	}

	str := sw.SaveScene(sceneName, sceneContent)

	fmt.Fprintln(w, str)
}

func saveSceneContentHandlerV2(w http.ResponseWriter, r *http.Request) {

	log.Info("request save scene v2. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)

	if err != nil {

		log.Error("read save scene body error!")
		return
	}

	str := sw.SaveSceneV2(string(body))

	fmt.Fprintln(w, str)
	log.Info("request save scene v2 end.")
}

func paramSceneCountOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("paramSceneCountOptionsHandler")

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	w.Header().Set("Access-Control-Allow-Methods", "POST")

	origin := r.Header.Get("Origin")
	log.Info(origin)
	if len(origin) > 0 {
		w.Header().Set("Access-Control-Allow-Origin", origin)
	} else {
		w.Header().Set("Access-Control-Allow-Origin", "*")
	}
}

func paramSceneCountHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("paramSceneCountHander start")

	origin := r.Header.Get("Origin")

	if len(origin) > 0 {
		log.Info("origin : ", origin)
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)

	if err != nil {
		log.Error("parse body error: ", err)
		fmt.Fprintln(w, "paramSceneCountHandler body error")
		return
	}

	var content map[string]interface{}

	log.Info("body: ", string(body))

	err = json.Unmarshal(body, &content)

	if err != nil {

		log.Error("unmarshal body error: ", err)
		fmt.Fprintln(w, "paramSceneHandler unmarshal error")
		return
	}

	log.Info("body parsed")

	var sceneName = content["sceneName"].(string)
	var param = content["param"].(string)

	log.Info("call ParamSceneCount: ", sceneName, ", ", param)

	if len(sceneName) < 1 || len(param) < 1 {
		log.Error("parmSceneCount parameter error!")
		fmt.Fprintln(w, "paramSceneCount parameter error!")
		return
	}

	str := sw.ParamSceneCount(sceneName, param)

	fmt.Fprintln(w, str)
	log.Info("paramSceneCountHandler end")
}

func paramSceneOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("paramSceneOptionsHandler")

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	w.Header().Set("Access-Control-Allow-Methods", "POST")

	origin := r.Header.Get("Origin")
	if len(origin) > 0 {
		w.Header().Set("Access-Control-Allow-Origin", origin)
	} else {
		w.Header().Set("Access-Control-Allow-Origin", "*")
	}
}

func paramSceneHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("paramSceneHander start")

	origin := r.Header.Get("Origin")

	if len(origin) > 0 {
		log.Info("origin : ", origin)
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)

	if err != nil {
		log.Error("parse body error: ", err)
		fmt.Fprintln(w, "paramSceneHandler body error")
		return
	}

	var content map[string]interface{}

	log.Info("body: ", string(body))

	err = json.Unmarshal(body, &content)

	if err != nil {

		log.Error("unmarshal body error: ", err)
		fmt.Fprintln(w, "paramSceneHandler unmarshal error")
		return
	}

	log.Info("body parsed")

	var sceneName = content["sceneName"].(string)
	var param = content["param"].(string)

	log.Info("call ParamScene: ", sceneName, ", ", param)

	if len(sceneName) < 1 || len(param) < 1 {
		log.Error("parmScene parameter error!")
		fmt.Fprintln(w, "paramScene parameter error!")
		return
	}

	str := sw.ParamScene(sceneName, param)

	fmt.Fprintln(w, str)
	log.Info("paramSceneHander end")
}

func saveHadmapSceneContent3Handler(w http.ResponseWriter, r *http.Request) {

	log.Info("request save hadmap scene v3 options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func saveHadmapSceneContent3(w http.ResponseWriter, r *http.Request) {

	log.Info("request save hadmap scene v3. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	//w.Header().Set("Access-Control-Allow-Headers", "Content-Type")

	body, err := ioutil.ReadAll(r.Body)

	if err != nil {

		log.Error("read save hadmap scene body error!")
		return
	}
	strJson := string(body)

	log.Info(strJson)

	if len(strJson) < 1 {
		fmt.Fprintln(w, "save hadmap scene param error")
		log.Info("save hadmap scene: no scene content")
		return
	}

	str := sw.SaveHadmapScene3(strJson)

	fmt.Fprintln(w, str)
}

func saveHadmapSceneContent2Handler(w http.ResponseWriter, r *http.Request) {

	log.Info("request save hadmap scene v2 options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func saveHadmapSceneContent2(w http.ResponseWriter, r *http.Request) {

	log.Info("request save hadmap scene v2. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	//w.Header().Set("Access-Control-Allow-Headers", "Content-Type")

	body, err := ioutil.ReadAll(r.Body)

	if err != nil {

		log.Error("read save hadmap scene body error!")
		return
	}
	strJson := string(body)

	log.Info(strJson)

	if len(strJson) < 1 {
		fmt.Fprintln(w, "save hadmap scene param error")
		log.Info("save hadmap scene: no scene content")
		return
	}

	str := sw.SaveHadmapScene2(strJson)

	fmt.Fprintln(w, str)
}

func saveHadmapSceneContentHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request save hadmap scene options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func saveHadmapSceneContent(w http.ResponseWriter, r *http.Request) {

	log.Info("request save hadmap scene. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	//w.Header().Set("Access-Control-Allow-Headers", "Content-Type")

	body, err := ioutil.ReadAll(r.Body)

	if err != nil {

		log.Error("read save hadmap scene body error!")
		return
	}
	//strJson := string(body)

	//fmt.Println(strJson)

	var cont map[string]interface{}

	err = json.Unmarshal(body, &cont)
	if err != nil {
		log.Error("save hadmap scene unmarshal json data error!")
		return
	}

	var hadmapSceneName = cont["name"].(string)
	var hadmapSceneContent = cont["content"].(string)

	if len(hadmapSceneName) < 1 {
		fmt.Fprintln(w, "save hadmap scene param error")
		log.Info("save hadmap scene: no scene content")
		return
	}

	if len(hadmapSceneContent) < 1 {

		fmt.Fprintln(w, "save hadmap scene param error")
		log.Info("save hadmap scene: no scene name")
		return
	}

	str := sw.SaveHadmapScene(hadmapSceneName, hadmapSceneContent)

	fmt.Fprintln(w, str)
}

func getHadmapSceneListOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request hadmap scene list options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type, x-offset, x-limit")
}

func getHadmapSceneListHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request hadmap scene list ")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	limit, _ := strconv.Atoi(r.Header.Get("x-limit"))
	offset, _ := strconv.Atoi(r.Header.Get("x-offset"))

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetHadmapSceneList(offset, limit)

	fmt.Fprintln(w, str)
}

func getHadmapSceneContentHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request hadmap scene content ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	params := mux.Vars(r)
	fileName := params["name"]
	if len(fileName) > 0 {

		str := sw.GetHadmapScene(fileName)

		fmt.Fprintln(w, str)
	} else {

		fmt.Fprintln(w, "parameter error!")
		log.Info("get hadmap scene content error, no scene name")
	}

}

func getHadmapSceneContent3Handler(w http.ResponseWriter, r *http.Request) {

	log.Info("request hadmap scene content v3")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	params := mux.Vars(r)
	fileName := params["name"]
	if len(fileName) > 0 {

		str := sw.GetHadmapScene3(fileName)

		fmt.Fprintln(w, str)
	} else {

		fmt.Fprintln(w, "parameter error!")
		log.Info("get hadmap scene content error, no scene name")
	}

}

func searchHadmapSceneOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request search hadmap list option")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type, x-offset, x-limit")
}

func searchHadmapSceneHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request search hadmap list ")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, _ := ioutil.ReadAll(r.Body)

	var cont map[string]interface{}

	json.Unmarshal(body, &cont)

	var sceneName = cont["name"].(string)

	if len(sceneName) < 1 {
		fmt.Fprintln(w, "search hadmap scene param error")
		log.Info("search hadmap scene: no scene name")
		return
	}

	log.Info(r.Header.Get("x-limit"))
	log.Info(r.Header.Get("x-offset"))

	limit, err := strconv.Atoi(r.Header.Get("x-limit"))
	if err != nil {
		log.Info(err.Error)
	}
	offset, err := strconv.Atoi(r.Header.Get("x-offset"))

	if err != nil {
		log.Info(err.Error)
	}

	str := sw.SearchHadmapScene(sceneName, offset, limit)

	fmt.Fprintln(w, str)
}

func getHadmapDataHandler(w http.ResponseWriter, r *http.Request) {
	/*
	   {
	       "cmd":"batchqueryinfobypt",
	       "mapname":"d2d_20190726.xodr",
	       "points": [
	           { "startLon":"0.0", "startLat":"0.0" }
	       ]
	   }
	*/
	JsonGetString := func(json string, path string, default_value string) string {
		value := gjson.Get(json, path)
		if value.Exists() {
			return value.String()
		}
		return default_value
	}

	// for POST
	body, _ := ioutil.ReadAll(r.Body)

	r.ParseForm()
	var hadmapCmd = JsonGetString(string(body), "cmd", "")

	if len(r.Form["cmd"]) > 0 {
		hadmapCmd = r.Form["cmd"][0]
	}
	if len(hadmapCmd) <= 0 {
		fmt.Println(w, "parameter error: lack of cmd parameter!")

		log.Info("parameter error: lack of cmd parameter!")

		return
	}

	var hadmapName = JsonGetString(string(body), "mapname", "")

	var lastModifiedTimeStr = ""
	var lastModifiedTime time.Time
	var er error
	if len(r.Header.Get("If-Modified-Since")) > 0 {
		lastModifiedTimeStr = r.Header.Get("If-Modified-Since")
		lastModifiedTime, er = time.Parse(http.TimeFormat, lastModifiedTimeStr)
		if er != nil {
			log.Error("convert time error ", lastModifiedTimeStr)
		}
	}

	//var hadmapSampleDistance = 10;

	if len(r.Form["mapname"]) > 0 {

		hadmapName = r.Form["mapname"][0]

	}
	if len(hadmapName) <= 0 {

		fmt.Println(w, "parameter error: no mapname!")
		log.Info("parameter error: no mapname!")
		//return
	}

	if len(r.Form["sampleDistance"]) > 0 {
		//hadmapSampleDistance, _ = strconv.Atoi(r.Form["sampleDistance"][0])
	}

	var str string

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	log.Info("request hadmap data: ", hadmapName, " ", hadmapCmd)

	if strings.Compare(hadmapCmd, "loadhadmap") == 0 {

		str = sw.LoadHadmap(hadmapName)
	} else if strings.Compare(hadmapCmd, "gettrafficlights") == 0 {

		str = sw.GetTrafficlights(hadmapName)
	} else if strings.Compare(hadmapCmd, "getallroaddata") == 0 {

		w.Header().Set("Last-Modified", startedTime.Format(http.TimeFormat))

		latest := startedTime.After(lastModifiedTime)
		latest = true

		/*
			log.Info("started time: ", startedTime.Format(http.TimeFormat), startedTime)
			log.Info("modified time: ", lastModifiedTime.Format(http.TimeFormat), lastModifiedTime)
			log.Info("time after compare result : ", latest)

			bef := startedTime.Before(lastModifiedTime)

			log.Info("started time: ", startedTime.Format(http.TimeFormat), startedTime)
			log.Info("modified time: ", lastModifiedTime.Format(http.TimeFormat), lastModifiedTime)
			log.Info("time after compare result : ", bef)
		*/

		if latest == true {
			log.Info("road load")
			str := sw.GetRoad(hadmapName)

			buf, er := compressData(str)
			if er != nil {
				w.Write([]byte("compress data error!"))
				w.WriteHeader(http.StatusGone)
				return
			}

			w.Write(buf.Bytes())
			return
		} else {
			w.WriteHeader(http.StatusNotModified)
			log.Info("road 304")
			return
		}

	} else if strings.Compare(hadmapCmd, "getalllanedata") == 0 {

		w.Header().Set("Last-Modified", startedTime.Format(http.TimeFormat))

		latest := startedTime.After(lastModifiedTime)
		latest = true
		if latest == true {
			log.Info("lane load")
			//str = sw.GetLane(hadmapName)
			str := sw.GetLane(hadmapName)

			buf, er := compressData(str)
			if er != nil {
				w.Write([]byte("compressed data error!"))
				w.WriteHeader(http.StatusGone)
				return
			}

			w.Write(buf.Bytes())

			return

		} else {
			w.WriteHeader(http.StatusNotModified)
			log.Info("lane 304")
			return
		}

	} else if strings.Compare(hadmapCmd, "getalllaneboundarydata") == 0 {

		w.Header().Set("Last-Modified", startedTime.Format(http.TimeFormat))

		latest := startedTime.After(lastModifiedTime)
		latest = true
		if latest == true {
			log.Info("lane boundary load")
			strs := sw.GetLaneBoundary(hadmapName)

			buf, er := compressData(strs)
			if er != nil {

				w.Write([]byte("compressed data error!"))
				w.WriteHeader(http.StatusGone)
				return
			}

			w.Write(buf.Bytes())
			return
		} else {
			w.WriteHeader(http.StatusNotModified)
			log.Info("lane boundary 304")
			return
		}

	} else if strings.Compare(hadmapCmd, "getalllanelinkdata") == 0 {

		w.Header().Set("Last-Modified", startedTime.Format(http.TimeFormat))

		latest := startedTime.After(lastModifiedTime)
		latest = true
		if latest == true {
			log.Info("lane link load")
			strs := sw.GetLaneLink(hadmapName)

			buf, er := compressData(strs)
			if er != nil {
				w.Write([]byte("compress data error!"))
				w.WriteHeader(http.StatusGone)
				return
			}

			w.Write(buf.Bytes())
			return
		} else {
			w.WriteHeader(http.StatusNotModified)
			log.Info("lane link 304")
			return
		}

	} else if strings.Compare(hadmapCmd, "getmapobjectdata") == 0 {

		w.Header().Set("Last-Modified", startedTime.Format(http.TimeFormat))
		latest := startedTime.After(lastModifiedTime)
		latest = true
		if latest == true {
			log.Info("map object load")
			strs := sw.GetMapObject(hadmapName)

			buf, er := compressData(strs)
			if er != nil {
				w.Write([]byte("compress data error!"))
				w.WriteHeader(http.StatusGone)
				return
			}

			w.Write(buf.Bytes())
			return
		} else {
			w.WriteHeader(http.StatusNotModified)
			log.Info("map object 304")
			return
		}

	} else if strings.Compare(hadmapCmd, "querynearbyinfo") == 0 {

		var querystring = ""

		if len(r.Form["param"]) > 0 {

			querystring = r.Form["param"][0]

		} else {

			fmt.Println(w, "parameter error: no query string!")
			log.Info("parameter error: no query string!")

			return
		}

		log.Info("querynearbyinfo: ", querystring)

		str = sw.QueryNearbyInfo(querystring)

	} else if strings.Compare(hadmapCmd, "querynearbylane") == 0 {

		var lon = ""
		var lat = ""
		if len(r.Form["startLon"]) > 0 {

			lon = r.Form["startLon"][0]

		} else {

			fmt.Println(w, "parameter error: no startLon!")
			log.Info("parameter error: no startLon!")

			return
		}

		if len(r.Form["startLat"]) > 0 {

			lat = r.Form["startLat"][0]

		} else {

			fmt.Println(w, "parameter error: no startLat!")
			log.Info("parameter error: no startLat!")

			return
		}

		str = sw.QueryNearbyLane(hadmapName, lon, lat)

	} else if strings.Compare(hadmapCmd, "queryinfobypt") == 0 {

		var lon = ""
		var lat = ""
		var searchDistance = ""
		if len(r.Form["startLon"]) > 0 {

			lon = r.Form["startLon"][0]

		} else {

			fmt.Println(w, "parameter error: no startLon!")
			log.Info("parameter error: no startLon!")

			return
		}

		if len(r.Form["startLat"]) > 0 {

			lat = r.Form["startLat"][0]

		} else {

			fmt.Println(w, "parameter error: no startLat!")
			log.Info("parameter error: no startLat!")

			return
		}
		if len(r.Form["searchDistance"]) > 0 {

			searchDistance = r.Form["searchDistance"][0]

		} else {
			searchDistance = "5.0"
		}
		str = sw.QueryInfobyPt(hadmapName, lon, lat, searchDistance)

	} else if strings.Compare(hadmapCmd, "batchqueryinfobypt") == 0 {
		str = sw.BatchQueryInfobyPt(hadmapName, string(body))
	} else if strings.Compare(hadmapCmd, "queryinfobylocalpt") == 0 {

		var x = ""
		var y = ""
		var z = ""
		if len(r.Form["x"]) > 0 {

			x = r.Form["x"][0]

		} else {

			fmt.Println(w, "parameter error: no x!")
			log.Info("parameter error: no x!")

			return
		}

		if len(r.Form["y"]) > 0 {

			y = r.Form["y"][0]

		} else {

			fmt.Println(w, "parameter error: no y!")
			log.Info("parameter error: no y!")

			return
		}

		if len(r.Form["z"]) > 0 {

			z = r.Form["z"][0]

		} else {

			fmt.Println(w, "parameter error: no z!")
			log.Info("parameter error: no z!")

			return
		}

		log.Info("QueryInfobyLocalPt")

		str = sw.QueryInfobyLocalPt(hadmapName, x, y, z)

	} else if strings.Compare(hadmapCmd, "querynextlane") == 0 {

		log.Info("query next lane beginin")
		var roadid = ""
		var secid = ""
		var laneid = ""
		if len(r.Form["roadid"]) > 0 {

			roadid = r.Form["roadid"][0]

		} else {

			fmt.Println(w, "parameter error: no roadid!")
			log.Info("parameter error: no roadid!")

			return
		}

		if len(r.Form["secid"]) > 0 {

			secid = r.Form["secid"][0]

		} else {

			fmt.Println(w, "parameter error: no secid!")
			log.Info("parameter error: no secid!")

			return
		}

		if len(r.Form["laneid"]) > 0 {

			laneid = r.Form["laneid"][0]

		} else {

			fmt.Println(w, "parameter error: no laneid!")
			log.Info("parameter error: no laneid!")

			return
		}
		log.Info("QueryNextLane")
		str = sw.QueryNextLane(hadmapName, roadid, secid, laneid)

	} else if strings.Compare(hadmapCmd, "querylonlat") == 0 {

		log.Info("query lonlat begin")
		var roadid = ""
		var secid = ""
		var laneid = ""
		var shift = ""
		var offset = ""

		if len(r.Form["roadid"]) > 0 {

			roadid = r.Form["roadid"][0]

		} else {

			fmt.Println(w, "parameter error: no roadid!")
			log.Info("parameter error: no roadid!")

			return
		}

		if len(r.Form["secid"]) > 0 {

			secid = r.Form["secid"][0]

		} else {

			fmt.Println(w, "parameter error: no secid!")
			log.Info("parameter error: no secid!")

			return
		}

		if len(r.Form["laneid"]) > 0 {

			laneid = r.Form["laneid"][0]

		} else {

			fmt.Println(w, "parameter error: no laneid!")
			log.Info("parameter error: no laneid!")

			return
		}
		if len(r.Form["shift"]) > 0 {

			shift = r.Form["shift"][0]

		} else {

			fmt.Println(w, "parameter error: no shift!")
			log.Info("parameter error: no shift!")

			return
		}
		if len(r.Form["offset"]) > 0 {

			offset = r.Form["offset"][0]

		} else {

			fmt.Println(w, "parameter error: no offset!")
			log.Info("parameter error: no offset!")

			return
		}

		log.Info("QueryLonLat")
		str = sw.QueryLonLat(hadmapName, roadid, secid, laneid, shift, offset)

	} else if strings.Compare(hadmapCmd, "querylonlatbypoint") == 0 {

		log.Info("query lonlat by point begin")
		var startLon = ""
		var startLat = ""
		var elemType = ""
		var elemId = ""
		var shift = ""
		var offset = ""

		if len(r.Form["startLon"]) > 0 {

			startLon = r.Form["startLon"][0]

		} else {

			fmt.Println(w, "parameter error: no startLon!")
			log.Info("parameter error: no startLat!")

			return
		}

		if len(r.Form["startLat"]) > 0 {

			startLat = r.Form["startLat"][0]

		} else {

			fmt.Println(w, "parameter error: no startLat!")
			log.Info("parameter error: no startLat!")

			return
		}

		if len(r.Form["type"]) > 0 {

			elemType = r.Form["type"][0]

		} else {

			fmt.Println(w, "parameter error: no elemType !")
			log.Info("parameter error: no elemType!")

			return
		}

		if len(r.Form["id"]) > 0 {

			elemId = r.Form["id"][0]

		} else {

			fmt.Println(w, "parameter error: no elemId !")
			log.Info("parameter error: no elemId!")

			return
		}

		if len(r.Form["shift"]) > 0 {

			shift = r.Form["shift"][0]

		} else {

			fmt.Println(w, "parameter error: no shift!")
			log.Info("parameter error: no shift!")

			return
		}
		if len(r.Form["offset"]) > 0 {

			offset = r.Form["offset"][0]

		} else {

			fmt.Println(w, "parameter error: no offset!")
			log.Info("parameter error: no offset!")

			return
		}

		log.Info("QueryLonLatByPoint")
		str = sw.QueryLonLatByPoint(hadmapName, startLon, startLat, elemType, elemId, shift, offset)

	} else if strings.Compare(hadmapCmd, "batchquerylonlatbypoint") == 0 {
		str = sw.BatchQueryLonLatByPoint(hadmapName, string(body))
	} else {

		fmt.Println(w, "parameter error, unknown cmd str!")
		return
	}

	fmt.Fprintln(w, str)

}

func getGISImageListOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request gis image list options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type, x-offset, x-limit")
}

func getGISImageListHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request gis image list ")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	limit, _ := strconv.Atoi(r.Header.Get("x-limit"))
	offset, _ := strconv.Atoi(r.Header.Get("x-offset"))

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetGISImageList(offset, limit)

	fmt.Fprintln(w, str)
}

func renameGISImagesOptionsHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("renameGISImagesOptionsHandler start")

	w.Header().Set("Access-Control-Allow-Credentials", "true")

	origin := r.Header.Get("Origin")

	if len(origin) < 1 {
		origin = "*"
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	w.Header().Set("Access-Control-Allow-Methods", "POST")

	log.Info("renameGISImagesOptionsHandler end")
}

func renameGISImagesHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("renameGISImagesHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"renameGISImagesHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.RenameGISImages(string(body))

	fmt.Fprint(w, str)

	log.Info("renameGISImageHandler end")
}

func deleteGISImagesOptionsHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("deleteGISImagesOptionsHandler start")

	w.Header().Set("Access-Control-Allow-Credentials", "true")

	origin := r.Header.Get("Origin")

	if len(origin) < 1 {
		origin = "*"
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	w.Header().Set("Access-Control-Allow-Methods", "POST")
	//w.Header().Set("Access-Control-Allow-Methods", "OPTIONS, DELETE")

	log.Info("renameGISImagesOptionsHandler end")
}

func deleteGISImagesHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("deleteGISImagesHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"deleteGISImagesHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.DeleteGISImages(string(body))

	fmt.Fprint(w, str)

	log.Info("deleteGISImagesHandler end")
}

func uploadGISImagesOptionsHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("uploadGISImagesOptionsHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	w.Header().Set("Access-Control-Allow-Methods", "POST")

	log.Info("uploadGISImagesOptionsHandler end")
}

func uploadGISImagesHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("uploadGISImagesHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"uploadGISImagesHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.UploadGISImages(string(body))

	fmt.Fprint(w, str)

	log.Info("uploadGISImagesHandler end")
}

func getSceneListOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request scene list options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type, x-offset, x-limit")
}

func getSceneListHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request scene list ")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	limit, _ := strconv.Atoi(r.Header.Get("x-limit"))
	offset, _ := strconv.Atoi(r.Header.Get("x-offset"))

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetSceneList(offset, limit)

	fmt.Fprintln(w, str)
}

func getSceneListHandlerV2(w http.ResponseWriter, r *http.Request) {

	log.Info("request scene list V2 start")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"getSceneListHandlerV2 parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetSceneListV2(string(body))

	fmt.Fprintln(w, str)
	log.Info("request scene list V2 end")
}

func syncScenarioDBFromDisk(w http.ResponseWriter, r *http.Request) {

	log.Info("request synchronize db from disk start")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.SyncScenarioDBFromDisk()

	fmt.Fprintln(w, str)

	log.Info("request synchronize db from disk end")
}

func getSceneInfoHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request scene info ")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	params := mux.Vars(r)
	fileName := params["name"]
	str := sw.GetSceneInfo(fileName)

	fmt.Fprintln(w, str)
}

func getSceneInfoHandlerV2(w http.ResponseWriter, r *http.Request) {

	log.Info("request scene info v2 ")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	params := mux.Vars(r)
	fileName := params["name"]
	str := sw.GetSceneInfoV2(fileName)

	fmt.Fprintln(w, str)
}

func deleteSceneOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("delete scene options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Methods", "DELETE,POST,GET,OPTIONS")
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type, x-offset, x-limit")
}

func deleteSceneHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("deleteSceneHandler start")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	params := mux.Vars(r)
	fileName := params["name"]
	str := sw.DeleteScene(fileName)

	log.Info(fileName)

	if str == "false" {
		http.Error(w, http.StatusText(http.StatusBadRequest), http.StatusBadRequest)
		var errStr string
		errStr = "{\"code\":401, \"message\":\"file not exist!\"}"
		fmt.Println(w, errStr)
	}

	log.Info("deleteSceneHandler end")
}

func deleteScenesOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("deleteScenesOptionsHandler start!")

	w.Header().Set("Access-Control-Allow-Credentials", "true")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	log.Info("deleteScenesOptionsHandler end")
}

func deleteScenesHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("deleteScenesHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"deleteScenesHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.DeleteScenes(string(body))

	fmt.Fprint(w, str)

	log.Info("deleteScenesHandler end")
}

func downloadScenesOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("downloadScenesOptionsHandler start!")

	w.Header().Set("Access-Control-Allow-Credentials", "true")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	log.Info("downloadScenesOptionsHandler end")
}

func downloadScenesHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("downloadScenesHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, "length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"downloadScenesHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.DownloadScenes(string(body))

	fmt.Fprint(w, str)

	log.Info("downloadScenesHandler end")
}

func getGenScenesProgressvalHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("request getGenScenesProgressval")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetGenScenesProgress()

	fmt.Fprint(w, str)
}

func stopGenScenesProgressHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("request stopGenScenesProgress")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.StopGenScenesProgress()

	fmt.Fprintln(w, str)
}

func getScenesProgressvalHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("request getScenesProgressval")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetScenesProgress()

	fmt.Fprint(w, str)
}

func stopScenesProgressHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("request stopScenesProgress")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.StopScenesProgress()

	fmt.Fprintln(w, str)
}

func uploadScenesOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("uploadScenesOptionsHandler start!")

	w.Header().Set("Access-Control-Allow-Credentials", "true")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	log.Info("uploadScenesOptionsHandler end")
}

func uploadScenesHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("uploadScenesHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, "length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"uploadScenesHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.UploadScenes(string(body))

	fmt.Fprint(w, str)

	log.Info("uploadScenesHandler end")
}

func copyScenesOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("copyScenesOptionsHandler start!")

	w.Header().Set("Access-Control-Allow-Credentials", "true")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	log.Info("copyScenesOptionsHandler end")
}

func copyScenesHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("copyScenesHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method

	log.Info("Content type: ", contentType, "length: ", contentLength, " method: ", method)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"copyScenesHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.CopyScenes(string(body))

	fmt.Fprint(w, str)

	log.Info("copyScenesHandler end")
}

func renameSceneOptionsHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("renameSceneOptionsHandler start")

	origin := r.Header.Get("Origin")

	if len(origin) < 1 {
		origin = "*"
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")

	log.Info("renameSceneOptionsHandler end")
}

func renameSceneHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("renameSceneHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"renameSceneHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.RenameScene(string(body))

	fmt.Fprint(w, str)

	log.Info("renameSceneHandler end")
}

func deleteHadmapsOptionsHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("deleteHadmapsOptionsHandler start")

	origin := r.Header.Get("Origin")

	if len(origin) < 1 {
		origin = "*"
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	//w.Header().Set("Access-Control-Allow-Methods", "OPTIONS, DELETE")

	log.Info("renameSceneOptionsHandler end")
}

func deleteHadmapsHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("deleteHadmapsHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"deleteHadmapsHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.DeleteHadmaps(string(body))

	fmt.Fprint(w, str)

	log.Info("deleteHadmapsHandler end")
}

func uploadHadmapsOptionsHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("uploadHadmapsOptionsHandler start")

	origin := r.Header.Get("Origin")

	if len(origin) < 1 {
		origin = "*"
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")

	log.Info("uploadHadmapsOptionsHandler end")
}

func uploadHadmapsHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("uploadHadmapsHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"uploadHadmapsHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.UploadHadmaps(string(body))

	fmt.Fprint(w, str)

	log.Info("uploadHadmapsHandler end")
}

func downloadHadmapsOptionsHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("downloadHadmapsOptionsHandler start")

	origin := r.Header.Get("Origin")

	if len(origin) < 1 {
		origin = "*"
	}

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")

	log.Info("downloadHadmapsOptionsHandler end")
}

func downloadHadmapsHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("downloadHadmapsHandler start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"downloadHadmapsHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.DownloadHadmaps(string(body))

	fmt.Fprint(w, str)

	log.Info("downloadHadmapsHandler end")
}

func searchSceneOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request search list option")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type, x-offset, x-limit")
}

func searchSceneHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request search list ")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, _ := ioutil.ReadAll(r.Body)

	var cont map[string]interface{}

	json.Unmarshal(body, &cont)

	var sceneName = cont["name"].(string)

	if len(sceneName) < 1 {
		fmt.Fprintln(w, "search scene param error")
		log.Info("search scene: no scene name")
		return
	}

	log.Info(r.Header.Get("x-limit"))
	log.Info(r.Header.Get("x-offset"))

	limit, err := strconv.Atoi(r.Header.Get("x-limit"))
	if err != nil {
		log.Info(err.Error)
	}
	offset, err := strconv.Atoi(r.Header.Get("x-offset"))

	if err != nil {
		log.Info(err.Error)
	}

	str := sw.SearchScene(sceneName, offset, limit)

	fmt.Fprintln(w, str)
}

func searchSceneHandlerV2(w http.ResponseWriter, r *http.Request) {
	log.Info("searchSceneHandler V2 start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"renameSceneHandler parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.SearchSceneV2(string(body))

	fmt.Fprint(w, str)

	log.Info("searchSceneHandler V2 end")
}

func getScenesInfoOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request scene info options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	//w.Header().Set("Access-Control-Allow-Methods", "DELETE,POST,GET,OPTIONS")
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func getScenesInfoHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request scenes info ")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, _ := ioutil.ReadAll(r.Body)

	var cont map[string]interface{}

	json.Unmarshal(body, &cont)

	var ids = cont["ids"].(string)

	if len(ids) < 1 {
		fmt.Fprintln(w, "get scenes handler param error")
		log.Info("get scenes handler: no scene name")
		return
	}

	str := sw.GetScenesInfo(ids)

	fmt.Fprintln(w, str)
}

func getScenesInfoHandlerV2(w http.ResponseWriter, r *http.Request) {

	log.Info("request scenes info V2")

	origin := r.Header.Get("Origin")
	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, _ := ioutil.ReadAll(r.Body)

	str := sw.GetScenesInfoV2(string(body))

	fmt.Fprintln(w, str)

	log.Info("request scenes info V2 end")
}

func getScenarioSetListOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request scenario sets options. ")
	optionsHandler(w, r)
}

func getScenarioSetListHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request scenario set list start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"getScenarioSetList parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.GetScenarioSetList(string(body))

	fmt.Fprint(w, str)

	log.Info("request scenario set list end")
}

func createScenarioSetOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request create scenario set options. ")
	optionsHandler(w, r)
}

func createScenarioSetHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request create scenario set start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"createScenarioSet parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.CreateScenarioSet(string(body))

	fmt.Fprint(w, str)

	log.Info("request create scenario set end")
}

func updateScenarioSetOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request update scenario set options. ")
	optionsHandler(w, r)
}

func updateScenarioSetHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request update scenario set start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"createScenarioSet parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.UpdateScenarioSet(string(body))

	fmt.Fprint(w, str)

	log.Info("request update scenario set end")
}

func deleteScenarioSetOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request delete scenario set options. ")
	optionsHandler(w, r)
}

func deleteScenarioSetHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request delete scenario set start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {

		errStr := "{\"code\":-1, \"message\":\"createScenarioSet parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}

	str := sw.DeleteScenarioSet(string(body))

	fmt.Fprint(w, str)

	log.Info("request delete scenario set end")
}

func recordLogOptionsHandler(w http.ResponseWriter, r *http.Request) {
	//log.Info("request record log options handler")
	optionsHandler(w, r)
}

func recordLogHandler(w http.ResponseWriter, r *http.Request) {

	//log.Info("request record log handler start")
	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	body, err := ioutil.ReadAll(r.Body)

	if err != nil {
		errStr := "{\"code\": -1, \"message\": \" recordLog parse body error\"}"
		fmt.Fprint(w, errStr)
	}

	//log.Info("request record log handler 2")
	sw.RecordLog(string(body))
	fmt.Fprint(w, "{\"code\":0}")

	//log.Info("request record log handler end")
}

//func upgradeWS(w http.ResponseWriter, r *http.Request) {
//	log.Info("upgrade to web socket")
//	conn, err := websocket.Upgrade(w, r, w.Header(), 1024, 1024)
//	if err != nil {
//		fmt.Fprintln(w, "errrrrrr")
//		http.Error(w, "upgrade error", 402)
//		return
//	}
//
//	wsProcess(conn)
//}
//
//func wsProcess(conn *websocket.Conn) {
//	defer conn.Close()
//
//	for {
//		time.Sleep(time.Second)
//		conn.WriteJSON("{\"type\": \"TEST\", \"data\": { \"debug\": \"hello world!\"}}")
//		//conn.Write([]byte("{\"type\": \"TEST\", \"data\": { \"debug\": \"hello world!\"}}"))
//	}
//}

func testAPI(w http.ResponseWriter, r *http.Request) {

	log.Info("testAPI ")

	origin := r.Header.Get("Origin")
	log.Debug(origin)
	log.Debug(r.URL.Path)

	//w.Header().Set("Access-Control-Allow-Credentials", "true")
	//w.Header().Set("Access-Control-Allow-Origin", "*")

	fmt.Fprintln(w, "test API OK")
}

func IsDir(path string) bool {
	s, err := os.Stat(path)
	if err != nil {
		return false
	}

	return s.IsDir()
}

func IsFile(path string) bool {
	s, err := os.Stat(path)
	if err != nil {
		return false
	}

	return !s.IsDir()
}

func Exists(path string) bool {
	_, err := os.Stat(path)
	if err == nil {
		return true
	}

	if os.IsExist(err) {
		return true
	}

	return false
}

func mapCreateSession(w http.ResponseWriter, r *http.Request) {
	log.Info("create session")
	origin := r.Header.Get("Origin")

	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.MapEditCreateSession()

	fmt.Fprintln(w, str)
}
func mapReleaseSessionOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request release session options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}
func mapReleaseSession(w http.ResponseWriter, r *http.Request) {
	log.Info("release session")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("parse environment global error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	str := sw.MapEditReleaseSession(string(body))

	fmt.Fprintf(w, str)

	log.Info("release session end")
}
func mapOpenMapOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request open map options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}
func mapOpenMap(w http.ResponseWriter, r *http.Request) {
	log.Info("open map start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("parse environment global error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	str := sw.MapEditOpenHadmap(string(body))

	fmt.Fprintf(w, str)

	log.Info("open map end")
}
func mapCreateStringOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request open map options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}
func mapCreateString(w http.ResponseWriter, r *http.Request) {
	log.Info("create string start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("parse environment global error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	str := sw.MapEditCreateString(string(body))

	fmt.Fprintf(w, str)

	log.Info("create string end")
}
func mapCreateMapOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request open map options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}
func mapCreateMap(w http.ResponseWriter, r *http.Request) {
	log.Info("create map start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("parse environment global error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	str := sw.MapEditCreateMap(string(body))

	fmt.Fprintf(w, str)

	log.Info("create map end")
}
func mapModifyMapOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request open map options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}
func mapModifyMap(w http.ResponseWriter, r *http.Request) {
	log.Info("modify map start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("parse environment global error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	str := sw.MapEditModifyMap(string(body))

	fmt.Fprintf(w, str)

	log.Info("modify map end")
}

func mapSaveMapOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request open map options. ")

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func mapSaveMap(w http.ResponseWriter, r *http.Request) {
	log.Info("save map start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("parse environment global error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	str := sw.MapEditSaveMap(string(body))

	fmt.Fprintf(w, str)

	log.Info("save map end")
}

func extractFile(file *zip.File, destinationDir string) error {
	//如果文件是目录，则返回
	if file.FileInfo().IsDir() {
		return nil
	}
	// 打开文件
	srcFile, err := file.Open()
	if err != nil {
		return err
	}
	defer srcFile.Close()

	// 创建目标文件路径
	destFilePath := filepath.Join(destinationDir, file.FileInfo().Name())

	// 创建目标文件所在的目录
	err = os.MkdirAll(filepath.Dir(destFilePath), os.ModePerm)
	if err != nil {
		return err
	}

	// 创建目标文件
	destFile, err := os.Create(destFilePath)
	if err != nil {
		return err
	}
	defer destFile.Close()

	// 将文件内容从源文件复制到目标文件
	_, err = io.Copy(destFile, srcFile)
	if err != nil {
		return err
	}

	return nil
}

func unzip(zipFilePath string, outputDirectory string) error {
	zipFile, err := zip.OpenReader(zipFilePath)
	if err != nil {
		return err
	}
	defer zipFile.Close()

	for _, file := range zipFile.File {
		err := extractFile(file, outputDirectory)
		if err != nil {
			return err
		}
	}
	return nil
}

type ZipInfo struct {
	Path string `json:"model_path"`
}

func mapModelUploadOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request map model upload options. ")
	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func mapModelUpload(w http.ResponseWriter, r *http.Request) {
	log.Info("ModelUpload start")
	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("parse ModelUpload global error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}
	var zipinfo ZipInfo
	err_json := json.Unmarshal(body, &zipinfo)
	if err_json != nil {
		fmt.Println("Error parsing JSON data:", err)
		return
	}
	id := time.Now().Unix()
	path := string(appRootPath + "\\..\\..\\data\\scenario\\cache\\" + strconv.FormatInt(id, 10))

	ext := filepath.Ext(zipinfo.Path)
	if ext == ".zip" {
		err_zip := unzip(string(zipinfo.Path), path)
		if err_zip != nil {
			log.Info(err_zip)
			fmt.Fprintf(w, "{\"id\":\"-1\",\"filepath\":[]}")
			return
		}
	}
	str := sw.ModelUpload(path, strconv.FormatInt(id, 10))
	fmt.Fprintf(w, str)
	log.Info("ModelUpload end")
	return
}

type DstFile struct {
	Path string `json:"dstpath"`
}

func uploadHandler(w http.ResponseWriter, r *http.Request) {
	// 获取上传的文件
	file, header, err := r.FormFile("file")
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		log.Info(w, "Error retrieving the file:", err)
		return
	}
	defer file.Close()

	// 获取并验证model_dir
	model_dir := r.FormValue("model_dir")
	if model_dir == "" {
		w.WriteHeader(http.StatusBadRequest)
		log.Info(w, "Error retrieving the key value")
		return
	}

	// 清理和验证model_dir
	cleanModelDir := filepath.Clean(model_dir)
	if !isValidDirectory(cleanModelDir) {
		w.WriteHeader(http.StatusBadRequest)
		log.Info(w, "Invalid directory path")
		return
	}

	files, err := filepath.Glob(filepath.Join(cleanModelDir, "*.fbx"))
	if err != nil || len(files) == 0 {
		w.WriteHeader(http.StatusInternalServerError)
		log.Info("Error finding files:", err)
		return
	}

	firstFile := files[0]
	fileName := filepath.Base(firstFile)
	ext := filepath.Ext(fileName)
	newFileName := "thumbnail_" + strings.TrimSuffix(fileName, ext) + filepath.Ext(header.Filename)
	destFilePath := filepath.Join(cleanModelDir, newFileName)

	// 使用os.OpenFile和syscall.O_CREAT|syscall.O_EXCL来安全创建文件
	out, err := os.OpenFile(destFilePath, os.O_WRONLY|os.O_CREATE|os.O_EXCL, 0666)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		log.Info(w, "Error creating the file:", err)
		return
	}
	defer out.Close()

	// 将上传的文件数据写入新文件
	_, err = io.Copy(out, file)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		log.Info(w, "Error writing the file:", err)
		return
	}

	ret := DstFile{
		Path: destFilePath,
	}
	jsonData, err := json.Marshal(ret)
	if err != nil {
		log.Info("Error converting to JSON:", err)
		return
	}

	// 文件上传成功，返回成功信息
	fmt.Fprintf(w, string(jsonData))
}

// 验证目录路径是否合法
func isValidDirectory(dir string) bool {
	// 检查目录是否存在并且是一个目录
	info, err := os.Stat(dir)
	if err != nil || !info.IsDir() {
		return false
	}
	return true
}

func uploadOptionsHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("request map model upload options. ")
	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func mapModelSaveHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("map model save start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("map model save error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	str := sw.MapModelSave(string(body))

	fmt.Fprintf(w, str)

	log.Info("map model save end")
}

func mapModelSaveOptionsHandler(w http.ResponseWriter, r *http.Request) {

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func mapModelDeleteHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("map model delete start")

	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("map model save error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	str := sw.MapModelDelete(string(body))

	fmt.Fprintf(w, str)

	log.Info("map model delete end")

}

func sceneModelSaveHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("scene model save start")
	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("scene model save error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}

	str := sw.SceneModelSave(string(body))

	fmt.Fprintf(w, str)

	log.Info("scene model save end")

}

func sceneModelDeleteHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("scene model save start")
	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("scene model delete error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}
	str := sw.SceneModelDelete(string(body))

	fmt.Fprintf(w, str)

	log.Info("scene model delete end")

}

func mapModelDeleteOptionsHandler(w http.ResponseWriter, r *http.Request) {

	origin := r.Header.Get("Origin")
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
}

func getMapModelListHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("create session")
	origin := r.Header.Get("Origin")

	log.Debug(origin)

	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	str := sw.GetMapModelList()

	fmt.Fprintln(w, str)
}
func insertMapHandler(w http.ResponseWriter, r *http.Request) {

	log.Info("insert one map start")
	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method
	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)
	body, err := ioutil.ReadAll(r.Body)
	log.Info("Content : ", string(body))
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"insert map parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}
	str := sw.InsertMap(string(body))
	fmt.Fprint(w, str)
	log.Info("insert one map end")

}

func insertScenesHandler(w http.ResponseWriter, r *http.Request) {
	log.Info("insert scnens start")
	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	contentType := r.Header.Get("Content-Type")
	contentLength := r.ContentLength
	method := r.Method
	log.Info("Content type: ", contentType, " length: ", contentLength, " method: ", method)
	body, err := ioutil.ReadAll(r.Body)
	log.Info("Content : ", string(body))
	if err != nil {
		errStr := "{\"code\":-1, \"message\":\"insert map parse body eror!!\"}"
		fmt.Fprint(w, errStr)
	}
	str := sw.InsertScene(string(body))
	fmt.Fprint(w, str)
	log.Info("insert scnens end")

}

type RetStruct struct {
	Code              int    `json:"code"`
	Message           string `json:"message"`
	NonScenariosCount int    `json:"nonScenariosCount"`
	GenScenariosCount int    `json:"genScenariosCount"`
}

func printStrings(args ...string) {
	for _, str := range args {
		log.Info(str)
	}
}

func semanticStartExe(w http.ResponseWriter, r *http.Request) {
	log.Info("semantic start exe")
	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	log.Info(string(body))
	if err != nil {
		log.Error("parse semantic strat exe error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}
	str := sw.StartGenerateSemantic(string(body))
	separator := ","

	// 使用 strings.Split() 函数分割字符串
	parts := strings.Split(str, separator)

	log.Info("分割后的字符串：", parts)
	// 初始化状态值
	value = 0
	printStrings(parts...)
	// 启动可执行程序
	excel2asam_path := string(appInstallPath) + "\\service\\excel2asam\\excel2asam.exe"
	log.Info("excel2asam_path : ", excel2asam_path)
	GetProcessInstance().StopProcess()

	if err := GetProcessInstance().StartProcess(excel2asam_path, parts...); err != nil {
		log.Info("start process failure")
		fmt.Fprintf(w, "{\"code\":-1,\"message\":\"Semantic generation failed\"}")
		return
	}
	log.Info("start process finshed")
	wg.Add(1)
	defer wg.Done()
	// 等待可执行程序返回语义生成信息
	valueMux.Lock()
	log.Info("lock")
	// 创建一个 20 秒的定时器
	timeout := time.After(20 * time.Second)
	// 等待条件成立或定时器到期
	select {
	case <-timeout:
		log.Info("timeout")
		fmt.Fprintf(w, "{\"code\":-1,\"message\":\"Semantic generation failed\"}")
		valueMux.Unlock()
		return
	default:
		cond.Wait() // 等待条件成立
		log.Info("Condition is satisfied")
	}
	valueMux.Unlock()
	log.Info("Unlock")
	ret := RetStruct{
		Code:              0,
		Message:           "ok",
		NonScenariosCount: nonScenariosCount,
		GenScenariosCount: genScenariosCount,
	}
	jsonData, err := json.Marshal(ret)
	if err != nil {
		log.Info("Error marshalling JSON:", err)
		fmt.Fprintf(w, "{'code':-1,'message':'Semantic generation failed'}")
		return
	}
	fmt.Fprintf(w, string(jsonData))
	log.Info("semantic start exe finished")
}

type SemanticStatuStruct struct {
	TaskGenStatus     int    `json:"taskGenStatus"`
	TaskAllCount      int    `json:"taskAllCount"`
	TaskNonCount      int    `json:"taskNonCount"`
	TaskFinishedCount int    `json:"taskFinishedCount"`
	Dir               string `json:"dir"`
}

type SemanticCodeStruct struct {
	Code    int    `json:"code"`
	Message string `json:"message"`
}

type SemanticProcessStruct struct {
	Code    int    `json:"code"`
	Message string `json:"message"`
	Dir     string `json:"dir"`
}

func semanticGenerate(w http.ResponseWriter, r *http.Request) {
	log.Info("semantic generate")
	origin := r.Header.Get("Origin")
	log.Debug(origin)
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	// 准备状态，等待进入开始状态
	valueMux.Lock()
	value = 1
	valueMux.Unlock()
	// 创建数据集
	sw.GenerateSceneSet()
	var ret SemanticProcessStruct
	ret.Code = 0
	ret.Message = "ok"
	ret.Dir = genDir
	jsonData, err3 := json.Marshal(ret)
	if err3 != nil {
		log.Info("Error marshalling JSON:", err3)
		fmt.Fprintf(w, "{'code':-1,'message':'Semantic generation failed'}")
		return
	}
	fmt.Fprintf(w, string(jsonData))
	log.Info("semantic generate end")
}

func semanticStatu(w http.ResponseWriter, r *http.Request) {
	log.Info("semantic statu")
	origin := r.Header.Get("Origin")
	if len(origin) < 1 {
		origin = "*"
	}
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	contentLength := r.ContentLength
	contentType := r.Header.Get("Content-Type")
	log.Info("Content type: ", contentType, " length: ", contentLength)
	body, err := ioutil.ReadAll(r.Body)
	log.Info(string(body))
	if err != nil {
		log.Error("semantic statu error!")
		fmt.Fprintf(w, "{'code':-1}")
		return
	}
	var semanticStatu SemanticStatuStruct
	err2 := json.Unmarshal([]byte(body), &semanticStatu)
	if err2 != nil {
		fmt.Println("Error:", err2)
		fmt.Fprintf(w, "{'code':-1}")
		return
	}
	nonScenariosCount = semanticStatu.TaskNonCount
	genScenariosCount = semanticStatu.TaskAllCount
	genDir = semanticStatu.Dir
	if genScenariosCount == 0 {
		percentFinshed = 1.0
	} else {
		formattedNumber := float64(semanticStatu.TaskFinishedCount) / float64(genScenariosCount)
		percentFinshed = float32(math.Round(formattedNumber*10) / 10)
	}

	var SemanticCode SemanticCodeStruct
	SemanticCode.Code = value
	SemanticCode.Message = "ok"
	if semanticStatu.TaskGenStatus == 2 {
		SemanticCode.Code = -1
		SemanticCode.Message = "Semantic generation failed"
	}
	jsonData, err3 := json.Marshal(SemanticCode)
	if err3 != nil {
		log.Info("Error marshalling JSON:", err3)
		fmt.Fprintf(w, "{'code':-1,'message':'Semantic generation failed'}")
		return
	}
	fmt.Fprintf(w, string(jsonData))
	valueMux.Lock()
	if semanticStatu.TaskGenStatus == 2 {
		value = semanticStatu.TaskGenStatus
	}
	cond.Signal()
	valueMux.Unlock()
	log.Info("Code value ", SemanticCode.Code)
	log.Info("semantic statu finished")
}

func semanticGetProcess(w http.ResponseWriter, r *http.Request) {
	log.Info("semantic getgenprogressval")
	origin := r.Header.Get("Origin")
	log.Debug(origin)
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)

	var ret SemanticCodeStruct
	ret.Code = 0
	if value == 2 {
		ret.Message = strconv.FormatFloat(1.000, 'f', 3, 64)
	} else {
		ret.Message = strconv.FormatFloat(float64(percentFinshed), 'f', 3, 64)
	}
	jsonData, err3 := json.Marshal(ret)
	if err3 != nil {
		log.Info("Error marshalling JSON:", err3)
		fmt.Fprintf(w, "{'code':-1,'message':'Semantic generation failed'}")
		return
	}
	fmt.Fprintf(w, string(jsonData))

	log.Info("end getgenprogressval")
}

func semanticStopProcess(w http.ResponseWriter, r *http.Request) {
	log.Info("semantic getgenprogressval")
	origin := r.Header.Get("Origin")
	log.Debug(origin)
	w.Header().Set("Access-Control-Allow-Credentials", "true")
	w.Header().Set("Access-Control-Allow-Origin", origin)
	valueMux.Lock()
	value = 2
	valueMux.Unlock()
	// 等待语义生成进程状态变为非ing
	var ret SemanticProcessStruct
	ret.Code = 0
	ret.Message = "ok"
	ret.Dir = genDir
	jsonData, err3 := json.Marshal(ret)
	if err3 != nil {
		log.Info("Error marshalling JSON:", err3)
		fmt.Fprintf(w, "{'code':-1,'message':'Semantic generation failed'}")
		return
	}
	fmt.Fprintf(w, string(jsonData))
	log.Info("end getgenprogressval")
}

func Register(route *mux.Router) {

	pwd, _ := os.Getwd()

	fmt.Println(pwd)

	// log.Info("pwd", pwd)
	// fmt.Println(filepath.Join(pwd, "/models"));

	// route := mux.NewRouter()
	// rosbag
	route.HandleFunc("/convertrosbag", saveSceneContentOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/convertrosbag", saveSceneContentHandler).Methods("POST")

	// static file server
	route.PathPrefix("/models/").HandlerFunc(staticFiles).Methods("GET")
	route.PathPrefix("/data/").HandlerFunc(staticFiles).Methods("GET")

	// configuration
	route.HandleFunc("/configurationinfo", getConfigurationHandler).Methods("GET")
	route.HandleFunc("/l3statemachineinfo", getL3StateMachineHandler).Methods("GET")

	// single scenario
	route.HandleFunc("/importscenetpl", importSceneTemplateHandler).Methods("GET")
	route.HandleFunc("/openscene", getSceneContentHandler).Methods("GET")
	route.HandleFunc("/openscene/v2", getSceneContentOptionsHandlerV2).Methods("OPTIONS")
	route.HandleFunc("/openscene/v2", getSceneContentHandlerV2).Methods("POST")
	route.HandleFunc("/savescene", saveSceneContentOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/savescene", saveSceneContentHandler).Methods("POST")
	route.HandleFunc("/savescene/v2", saveSceneContentOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/savescene/v2", saveSceneContentHandlerV2).Methods("POST")
	route.HandleFunc("/paramscene", paramSceneOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/paramscene", paramSceneHandler).Methods("POST")
	route.HandleFunc("/paramscenecount", paramSceneCountOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/paramscenecount", paramSceneCountHandler).Methods("POST")

	// scenarios
	route.HandleFunc("/api/scenarios", getSceneListOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarios", getSceneListHandler).Methods("GET")
	route.HandleFunc("/api/scenarios/v2", getSceneListHandlerV2).Methods("POST")
	route.HandleFunc("/api/scenarios/sync", syncScenarioDBFromDisk).Methods("POST")
	route.HandleFunc("/api/scenarios/{name}", getSceneInfoHandler).Methods("GET")
	route.HandleFunc("/api/scenarios/v2/{name}", getSceneInfoHandlerV2).Methods("GET")
	route.HandleFunc("/api/scenarios/{name}", deleteSceneOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarios/{name}", deleteSceneHandler).Methods("DELETE")
	route.HandleFunc("/api/scenarios/delete", deleteScenesOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarios/delete", deleteScenesHandler).Methods("POST")
	route.HandleFunc("/api/scenarios/download", downloadScenesOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarios/download", downloadScenesHandler).Methods("POST")
	route.HandleFunc("/api/scenarios/upload", uploadScenesOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarios/upload", uploadScenesHandler).Methods("POST")
	route.HandleFunc("/api/scenarios/copy", copyScenesOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarios/copy", copyScenesHandler).Methods("POST")
	route.HandleFunc("/api/scenarios/rename", renameSceneOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarios/rename", renameSceneHandler).Methods("POST")
	route.HandleFunc("/api/scenarios/searches", searchSceneOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarios/searches", searchSceneHandler).Methods("POST")
	route.HandleFunc("/api/scenarios/searches/v2", searchSceneOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarios/searches/v2", searchSceneHandlerV2).Methods("POST")
	route.HandleFunc("/api/scenarios/ids", getScenesInfoOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarios/ids", getScenesInfoHandler).Methods("POST")
	route.HandleFunc("/api/scenarios/ids/v2", getScenesInfoOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarios/ids/v2", getScenesInfoHandlerV2).Methods("POST")

	route.HandleFunc("/api/scenarios/getprogressval", getScenesProgressvalHandler).Methods("GET")
	route.HandleFunc("/api/scenarios/getprogressval", getScenesProgressvalHandler).Methods("POST")

	route.HandleFunc("/api/scenarios/stopprogress", stopScenesProgressHandler).Methods("GET")
	route.HandleFunc("/api/scenarios/stopprogress", stopScenesProgressHandler).Methods("POST")

	route.HandleFunc("/api/scenarios/getgenprogressval", getGenScenesProgressvalHandler).Methods("GET")
	route.HandleFunc("/api/scenarios/getgenprogressval", getGenScenesProgressvalHandler).Methods("POST")
	route.HandleFunc("/api/scenarios/stopgenprogress", stopGenScenesProgressHandler).Methods("GET")
	route.HandleFunc("/api/scenarios/stopgenprogress", stopGenScenesProgressHandler).Methods("POST")

	// scene sets
	route.HandleFunc("/api/scenarioset", getScenarioSetListOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarioset", getScenarioSetListHandler).Methods("POST")
	route.HandleFunc("/api/scenarioset/add", createScenarioSetOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarioset/add", createScenarioSetHandler).Methods("POST")
	route.HandleFunc("/api/scenarioset/delete", deleteScenarioSetOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarioset/delete", deleteScenarioSetHandler).Methods("POST")
	route.HandleFunc("/api/scenarioset/update", updateScenarioSetOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/scenarioset/update", updateScenarioSetHandler).Methods("POST")

	// gis images
	route.HandleFunc("/api/gisimages", getGISImageListOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/gisimages", getGISImageListHandler).Methods("GET")
	// single gis image
	route.HandleFunc("/api/gisimage/delete", deleteGISImagesOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/gisimage/delete", deleteGISImagesHandler).Methods("POST")
	route.HandleFunc("/api/gisimage/upload", uploadGISImagesOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/gisimage/upload", uploadGISImagesHandler).Methods("POST")
	route.HandleFunc("/api/gisimage/rename", renameGISImagesOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/gisimage/rename", renameGISImagesHandler).Methods("POST")

	// single hadmap scene
	route.HandleFunc("/api/hadmaps/open/{name}", getHadmapSceneContentHandler).Methods("GET")
	route.HandleFunc("/api/hadmap/save", saveHadmapSceneContent2Handler).Methods("OPTIONS")
	route.HandleFunc("/api/hadmap/save", saveHadmapSceneContent2).Methods("POST")
	route.HandleFunc("/api/hadmaps/open/v3/{name}", getHadmapSceneContent3Handler).Methods("GET")
	route.HandleFunc("/api/hadmap/save/v3", saveHadmapSceneContent3Handler).Methods("OPTIONS")
	route.HandleFunc("/api/hadmap/save/v3", saveHadmapSceneContent3).Methods("POST")

	// hadmap scenes
	route.HandleFunc("/api/hadmaps", getHadmapSceneListOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/hadmaps", getHadmapSceneListHandler).Methods("GET")
	route.HandleFunc("/api/hadmaps/searches", searchHadmapSceneOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/hadmaps/searches", searchHadmapSceneHandler).Methods("POST")

	// single hadmap operation
	route.HandleFunc("/hadmapdata", getHadmapDataHandler).Methods("GET")
	route.HandleFunc("/hadmapdata", getHadmapDataHandler).Methods("POST")

	// hadmaps
	route.HandleFunc("/hadmaplist", getHadmapListHandler).Methods("GET")
	route.HandleFunc("/hadmaplist", getHadmapListHandler).Methods("POST")
	route.HandleFunc("/hadmapinfolist", getHadmapinfoListHandler).Methods("GET")
	route.HandleFunc("/hadmapinfolist", getHadmapinfoListHandler).Methods("POST")
	route.HandleFunc("/hadmaps/delete", deleteHadmapsOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/hadmaps/delete", deleteHadmapsHandler).Methods("POST")
	route.HandleFunc("/hadmaps/download", downloadHadmapsOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/hadmaps/download", downloadHadmapsHandler).Methods("POST")
	route.HandleFunc("/hadmaps/upload", uploadHadmapsOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/hadmaps/upload", uploadHadmapsHandler).Methods("POST")

	route.HandleFunc("/hadmaps/getprogressval", getHadmapsProgressvalHandler).Methods("GET")
	route.HandleFunc("/hadmaps/getprogressval", getHadmapsProgressvalHandler).Methods("POST")

	route.HandleFunc("/hadmaps/stopprogress", stopHadmapsProgressHandler).Methods("GET")
	route.HandleFunc("/hadmaps/stopprogress", stopHadmapsProgressHandler).Methods("POST")
	//catalog
	route.HandleFunc("/catalogs/getcatalogslist", getCatalogsList).Methods("GET")
	route.HandleFunc("/catalogs/add", CatalogsAdd).Methods("POST")
	route.HandleFunc("/catalogs/update", CatalogsUpdate).Methods("POST")
	route.HandleFunc("/catalogs/delete", CatalogsDelete).Methods("POST")

	//sensorV2
	route.HandleFunc("/sensors/load", sensorLoad).Methods("GET")
	route.HandleFunc("/sensors/save", sensorSave).Methods("POST")
	route.HandleFunc("/sensors/add", sensorAdd).Methods("POST")
	route.HandleFunc("/sensors/delete", sensorDelete).Methods("POST")
	route.HandleFunc("/sensors/update", sensorUpdate).Methods("POST")

	//sensorgroup
	route.HandleFunc("/sensorGroup/load", sensorGroupLoad).Methods("GET")
	route.HandleFunc("/sensorGroup/save", sensorGroupAdd).Methods("POST")
	route.HandleFunc("/sensorGroup/delete", sensorGroupDelete).Methods("POST")

	//dynamic
	route.HandleFunc("/dynamics/load", dynamicLoad).Methods("GET")
	route.HandleFunc("/dynamics/save", dynamicSave).Methods("POST")
	route.HandleFunc("/dynamics/delete", dynamicDelete).Methods("POST")

	//sensor
	route.HandleFunc("/sensors/loadpreset", loadSensorPreset).Methods("GET")
	route.HandleFunc("/sensors/savepreset", saveSensorPresetOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/sensors/savepreset", saveSensorPreset).Methods("POST")
	route.HandleFunc("/sensors/loadglobal", loadSensorGlobalConfig).Methods("GET")
	route.HandleFunc("/sensors/saveglobal", saveSensorGlobalConfigHandler).Methods("OPTIONS")
	route.HandleFunc("/sensors/saveglobal", saveSensorGlobalConfig).Methods("POST")
	route.HandleFunc("/sensors/loadglobalenvironment", loadEnvironmentGlobalConfig).Methods("GET")
	route.HandleFunc("/sensors/saveglobalenvironment", saveEnvironmentGlobalConfigHandler).Methods("OPTIONS")
	route.HandleFunc("/sensors/saveglobalenvironment", saveEnvironmentGlobalConfig).Methods("POST")
	//

	// apis
	route.HandleFunc("/city_monitor/pause/{uuid}", testAPI)
	route.HandleFunc("/city_monitor/resume/{uuid}", testAPI)

	// tools
	route.HandleFunc("/api/record/log/info", recordLogOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/api/record/log/info", recordLogHandler).Methods("POST")

	handler.SetRequestInfoLogger(log)
	route.Use(handler.CrossAccessControlHandler)
	route.Use(handler.LogRequestInfo)

	//
	route.HandleFunc("/mapedit/create_session", mapCreateSession).Methods("GET")
	route.HandleFunc("/mapedit/release_session", mapReleaseSessionOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/mapedit/release_session", mapReleaseSession).Methods("POST")
	route.HandleFunc("/mapedit/open_hadmap", mapOpenMapOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/mapedit/open_hadmap", mapOpenMap).Methods("POST")
	route.HandleFunc("/mapedit/get_hadmap", mapCreateStringOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/mapedit/get_hadmap", mapCreateString).Methods("POST")
	route.HandleFunc("/mapedit/create_hadmap", mapCreateMapOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/mapedit/create_hadmap", mapCreateMap).Methods("POST")
	route.HandleFunc("/mapedit/modify_hadmap", mapModifyMapOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/mapedit/modify_hadmap", mapModifyMap).Methods("POST")
	route.HandleFunc("/mapedit/save_hadmap", mapSaveMapOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/mapedit/save_hadmap", mapSaveMap).Methods("POST")

	// models
	route.HandleFunc("/mapmodel/upload", mapModelUpload).Methods("POST")
	route.HandleFunc("/mapmodel/upload", mapModelUploadOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/mapmodel/upload_thumbnail", uploadHandler).Methods("POST")
	route.HandleFunc("/mapmodel/upload_thumbnail", uploadOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/mapmodel/save", mapModelSaveHandler).Methods("POST")
	route.HandleFunc("/mapmodel/save", mapModelSaveOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/mapmodel/delete", mapModelDeleteHandler).Methods("POST")
	route.HandleFunc("/mapmodel/delete", mapModelDeleteOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/mapmodel/getmodellist", getMapModelListHandler).Methods("GET")
	route.HandleFunc("/scenemodel/upload", mapModelUpload).Methods("POST")
	route.HandleFunc("/scenemodel/upload", mapModelUploadOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/scenemodel/upload_thumbnail", uploadHandler).Methods("POST")
	route.HandleFunc("/scenemodel/upload_thumbnail", uploadOptionsHandler).Methods("OPTIONS")
	route.HandleFunc("/scenemodel/save", sceneModelSaveHandler).Methods("POST")
	route.HandleFunc("/scenemodel/delete", sceneModelDeleteHandler).Methods("POST")

	//semantic
	route.HandleFunc("/api/semantic/startexe", semanticStartExe).Methods("POST")
	route.HandleFunc("/api/semantic/generate", semanticGenerate).Methods("GET")
	route.HandleFunc("/semantic/statu", semanticStatu).Methods("POST")
	route.HandleFunc("/api/semantic/getgenprogressval", semanticGetProcess).Methods("GET")
	route.HandleFunc("/api/semantic/stopgenprogressval", semanticStopProcess).Methods("GET")

	route.HandleFunc("/maps/insert", insertMapHandler).Methods("POST")
	route.HandleFunc("/scenarios/insert", insertScenesHandler).Methods("POST")
	/*
		route.HandleFunc("/api/record/log/error", recordLogOptionsHandler).Methods("OPTIONS")
		route.HandleFunc("/api/record/log/error", recordLogHandler).Methods("POST")
	*/

	// route.HandleFunc("/ws", upgradeWS)
	// route.Handle("/ws", websocket.Handler(wsProcess))
	// log.Info("dir: ", filepath.Join(pwd, "/statics/"))

	// fmt.Println(filepath.Join(pwd, "/statics/"))

	// http.Handle("/aaa/", http.StripPrefix("/aaa/", http.FileServer(http.Dir(filepath.Join(pwd, "/statics/")))))
	// http.HandleFunc("/statics/", staticFiles)
	// http.HandleFunc("/hadmapdata", getHadmapDataHandler)
	// http.HandleFunc("/hadmaplist", getHadmapListHandler)
	// http.HandleFunc("/openscene", getSceneContentHandler)
	// http.HandleFunc("/savescene", saveSceneContentHandler)
	// http.HandleFunc("/api/scenarios", getSceneListHandler)
	// http.HandleFunc("/api/scenarios/:name", getSceneInfoHandler)
	// http.Handle("/ws", websocket.Handler(wsProcess))
	// http.ListenAndServe("127.0.0.1:9000", route)

}

func Configure(path string, appInstallDir string) bool {

	fmt.Println("starting configuring scenario service...")

	fmt.Printf("scenario directory is: %s, app install directory is %s. \n", path, appInstallDir)
	if Exists(path) == false || IsDir(path) == false {
		fmt.Printf("scenario directory %s is not exist", path)
		return false
	}

	appRootPath = path
	appInstallPath = appInstallDir
	var logDir string = path + "/log"

	// 创建日志目录
	if !Exists(logDir) {
		err := os.MkdirAll(logDir, os.ModePerm)
		if err != nil {
			fmt.Println("creating scenario log directory failed!")
			return false
		}
	}

	t := time.Now()
	date := t.Format("20060102150405")
	var s string = fmt.Sprintf("main_%s.log", date)

	startedTime = t
	startedTimeStr := startedTime.Format(http.TimeFormat)
	startedTime, _ = time.Parse(http.TimeFormat, startedTimeStr)

	var logFileName string = logDir + "/" + s
	file, err := os.OpenFile(logFileName, os.O_CREATE|os.O_WRONLY, 0666)
	if err == nil {
		log.SetOutput(file)
	} else {
		log.SetOutput(os.Stdout)
	}
	// log
	Formatter := new(logrus.TextFormatter)
	// Formatter.TimestampFormat = "2006-01-02T15:04:05.999999999Z07:00"
	Formatter.TimestampFormat = "2006-01-02T15:04:05.999999999"
	// Formatter.FullTimestamp = true
	// Formatter.ForceColors = true
	log.SetFormatter(Formatter)
	// log.SetFormatter(&logrus.TextFormatter{})
	log.SetLevel(logrus.DebugLevel)

	log.Info("init scene wrapper, resource path : ", path)
	log.Info("app install directory is ", appInstallDir)
	sw.Initlize(path, appInstallDir)
	log.Info("init scene wrapper finished")

	fmt.Println("finished configuring scenario service.")

	return true
}
