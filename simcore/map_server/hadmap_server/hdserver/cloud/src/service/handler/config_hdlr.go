package handler

import (
	"encoding/json"
	"fmt"
	"github.com/sirupsen/logrus"
	"net/http"
	"time"
	"txSim/service/mgr"
)

const (
	moduleTypeSystem = iota
	moduleTypeUser
)

// ConfigHandler holds a logger used in handlers.
type ConfigHandler struct {
	log       *logrus.Logger
	moduleMgr *mgr.ModuleManager
}

// NewConfigHandler create and return a ConfigHandler.
func NewConfigHandler(l *logrus.Logger, appDir string) *ConfigHandler {
	moduleMgr := mgr.ModuleManager{}
	err := moduleMgr.Initialize(l, appDir)
	if err == nil {
		l.Infoln("Module manager initialized.")
	} else {
		l.Fatalf("Module manager Initialization failed: %v\n", err)
	}
	return &ConfigHandler{l, &moduleMgr}
}

// GetModuleHandler handles GET request with query parameters:
// (currently no parameter, return all module configurations in the database.)
func (mh *ConfigHandler) GetModuleHandler(w http.ResponseWriter, r *http.Request) {
	modules, err := mh.moduleMgr.GetAllModules()
	if err != nil {
		mh.log.Errorf("config handler(%q %v) get all module error: %v\n", r.URL.Path, r.Method, err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}
	jsonStr, err := getModuleJSONOutput(modules)
	if err != nil {
		mh.log.Errorf("config handler(%q %v) marshal json error: %v\n", r.URL.Path, r.Method, err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}
	w.Header().Set("Content-Type", "application/json")
	fmt.Fprint(w, jsonStr)
}

// ModuleConfigOutput represents the json format accepted by front-end.
// for now we need this temporary format conversion. will be removed in TADSim v2.0.
type ModuleConfigOutput struct {
	ID              uint32     `json:"id"`
	Name            string     `json:"name"`
	StepTime        float64    `json:"stepTime"`
	Timeout         uint32     `json:"timeout"`
	ServiceEndpoint string     `json:"serviceEndpoint"`
	LibPath         string     `json:"libPath"`
	Path            string     `json:"path"`
	Args            string     `json:"args"`
	InitArgs        string     `json:"initArgs"`
	From            uint8      `json:"from"`
	Editable        bool       `json:"editable"`
	Deletable       bool       `json:"deletable"`
	UserID          int32      `json:"userId"`
	CreatedAt       time.Time  `json:"createdAt"`
	UpdatedAt       time.Time  `json:"updatedAt"`
	DeletedAt       *time.Time `json:"deletedAt"`
}

// ModuleConfigListOutput represents a list of ModuleConfigOutput.
type ModuleConfigListOutput struct {
	List  []*ModuleConfigOutput `json:"list"`
	Count int                   `json:"count"`
}

func formatOutputModuleConfig(o *mgr.ModuleConfig) *ModuleConfigOutput {
	out := ModuleConfigOutput{
		ID:              o.ID,
		Name:            o.Name,
		StepTime:        o.StepTime,
		Timeout:         o.Timeout,
		ServiceEndpoint: o.ServiceEndpoint,
		LibPath:         "",
		InitArgs:        o.InitParams,
		From:            o.Type,
		UserID:          o.UserID,
		CreatedAt:       o.Created,
		UpdatedAt:       o.Updated,
		DeletedAt:       nil}

	if len(o.LibPath) == 0 {
		out.Path = GetModuleLaunchCommand(o.EnvironmentPrefix, o.LaunchCommand)
		out.Args = o.CommandArgs
	} else {
		out.Path = GetModuleLaunchCommand(o.EnvironmentPrefix, "")
		out.Args = fmt.Sprintf("%s %s", o.ServiceEndpoint, o.LibPath)
	}

	if out.From == moduleTypeSystem {
		out.Editable = false
		out.Deletable = false
	} else {
		out.Editable = true
		out.Deletable = true
	}

	return &out
}

func getModuleJSONOutput(modules []*mgr.ModuleConfig) (string, error) {
	outModules := make([]*ModuleConfigOutput, len(modules))
	for i, m := range modules {
		outModules[i] = formatOutputModuleConfig(m)
	}
	output := ModuleConfigListOutput{List: outModules, Count: len(modules)}
	jsonBytes, err := json.Marshal(output)
	return string(jsonBytes), err
}
