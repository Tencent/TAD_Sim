package handler

import (
	"encoding/json"
	"fmt"
	"github.com/sirupsen/logrus"
	"net/http"
	"txSim/service/mgr"
)

// ModelHandler holds a logger used in handlers.
type ModelHandler struct {
	log     *logrus.Logger
	manager *mgr.ModelManager
}

// NewModelHandler create and return a modelHandler.
func NewModelHandler(l *logrus.Logger, appDir string) *ModelHandler {
	modelMgr := mgr.ModelManager{}
	go modelMgr.Initialize(l, appDir)
	//err := modelMgr.Initialize(l, appDir)
	//if err == nil {
	//	l.Infoln("Model manager initialized.")
	//} else {
	//	l.Fatalf("Model manager Initialization failed: %v\n", err)
	//}
	return &ModelHandler{l, &modelMgr}
}

// AddModelHandler handles POST request with json string body content:
// {"name": "foo", "type": "bar", "image": "xxx", "modelIamge": "yyy"}
func (mh *ModelHandler) AddModelHandler(w http.ResponseWriter, r *http.Request) {
	d := json.NewDecoder(r.Body)
	var m mgr.ScenarioModel
	if err := d.Decode(&m); err != nil {
		mh.log.Errorf("model handler(%q %v) parse json body error: %v\n", r.URL.Path, r.Method, err)
		w.WriteHeader(http.StatusBadRequest)
		return
	}
	if err := mh.manager.AddModel(&m); err != nil {
		mh.log.Errorf("model handler(%q %v) add model error: %v\n", r.URL.Path, r.Method, err)
		w.WriteHeader(http.StatusInternalServerError)
	}
	w.WriteHeader(http.StatusCreated)
}

// GetModelHandler handles GET request with query parameters:
// (currently no parameter, return all models in the database.)
func (mh *ModelHandler) GetModelHandler(w http.ResponseWriter, r *http.Request) {
	models, err := mh.manager.GetAllModels()
	if err != nil {
		mh.log.Errorf("model handler(%q %v) get all model error: %v\n", r.URL.Path, r.Method, err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}
	jsonStr, err := getModelJSONOutput(models, mh.manager.TypeIdx)
	if err != nil {
		mh.log.Errorf("model handler(%q %v) marshal json error: %v\n", r.URL.Path, r.Method, err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}
	w.Header().Set("Content-Type", "application/json")
	fmt.Fprint(w, jsonStr)
}

func getModelJSONOutput(models []*mgr.ScenarioModel, idxs map[string]uint32) (string, error) {
	outModels := make([]map[string][]*mgr.ScenarioModel, len(idxs))
	for _, m := range models {
		mType := m.Type
		m.Type = ""
		i := idxs[mType]
		if outModels[i] == nil {
			outModels[i] = make(map[string][]*mgr.ScenarioModel)
		}
		outModels[i][mType] = append(outModels[i][mType], m)
	}
	jsonBytes, err := json.Marshal(outModels)
	return string(jsonBytes), err
}
