package mgr

import (
	"encoding/json"
	"fmt"
	"os"
	"path"
	"txSim/service/utils"
)

const modelDataFileName = "model_data.json"
const moduleDataFileName = "module_data.json"

// ModelDataConfig is a json representation of "config" field in ModelData.
type ModelDataConfig struct {
	TypeIdx map[string]uint32 `json:"index"`
}

// ModelData is a json representation from model_data.json file, used for initiation.
type ModelData struct {
	Data   []*ScenarioModel `json:"data"`
	Config ModelDataConfig  `json:"config"`
}

// ModuleData is a json representation from module_data.json file, used for initiation.
type ModuleData struct {
	Data []*ModuleConfig `json:"data"`
}

// getModelsFromFile reads model data from a JSON file and returns a slice of ScenarioModel and a type index map.
func getModelsFromFile(fp string) ([]*ScenarioModel, map[string]uint32, error) {
	// Open the file
	f, err := os.Open(fp)
	if err != nil {
		return nil, nil, err
	}
	// Create a JSON decoder and decode the data into ModelData struct
	d := json.NewDecoder(f)
	var data ModelData
	if err = d.Decode(&data); err != nil {
		return nil, nil, err
	}
	// Check if the type index map is empty
	if len(data.Config.TypeIdx) == 0 {
		return nil, nil, fmt.Errorf("no index config provided in %v", modelDataFileName)
	}
	models := make([]*ScenarioModel, 0, 16) // 16 is reasonable for the initial guess of TADSim scenario model count.
	for _, m := range data.Data {
		_, ok := data.Config.TypeIdx[m.Type]
		if !ok {
			return nil, nil, fmt.Errorf("no %v type index defined in %v", m.Type, modelDataFileName)
		}
		models = append(models, m)
	}
	// Normalize the type index map
	utils.NormalizeIndexMap(data.Config.TypeIdx)
	return models, data.Config.TypeIdx, nil
}

// getModulesFromFile reads module configuration data from a JSON file and returns a slice of ModuleConfig.
func getModulesFromFile(fp string) ([]*ModuleConfig, error) {
	// Open the file
	f, err := os.Open(fp)
	if err != nil {
		return nil, err
	}
	// Create a JSON decoder and decode the data into ModuleData struct
	d := json.NewDecoder(f)
	var data ModuleData
	if err = d.Decode(&data); err != nil {
		return nil, err
	}
	// Initialize the modules slice and append the module configurations to it
	modules := make([]*ModuleConfig, 0)
	for _, m := range data.Data {
		modules = append(modules, m)
	}
	return modules, nil
}

// initScenarioModelTableData initializes the scenario model data in the database.
func initScenarioModelTableData(mmgr *ModelManager) error {
	// Create a new ScenarioModel instance and query table
	table := new(ScenarioModel)
	qt := mmgr.ormer.QueryTable(table)
	cnt, err := qt.Count()
	if err != nil {
		return err
	}
	// Check if the model data file exists
	fp := path.Join(mmgr.appDir, modelDataFileName)
	if isExists, err := utils.IsFileExist(fp); !isExists {
		mmgr.log.Infoln("model data file not exist, skip scenario model data initialization.")
		if err != nil {
			mmgr.log.Errorf("find %v file error: %v", moduleDataFileName, err)
		}
		return nil
	}
	// Get the models and type index map from the file
	models, typeIdx, err := getModelsFromFile(fp)
	if err != nil {
		return err
	}
	mmgr.TypeIdx = typeIdx
	// If the table is empty, insert the model data into the database
	if cnt == 0 {
		mmgr.log.Infoln("initializing scenario model data...")
		inserter, _ := qt.PrepareInsert()
		for _, model := range models {
			id, err := inserter.Insert(model)
			if err != nil {
				return err
			}
			mmgr.log.Infof("model(%v %v %v) inserted.", id, model.Name, model.Type)
		}
		inserter.Close()
	}
	return nil
}

// initModuleConfigTableData initializes the module configuration data in the database.
func initModuleConfigTableData(mmgr *ModuleManager) error {
	// Create a new ModuleConfig instance and query table
	table := new(ModuleConfig)
	qt := mmgr.ormer.QueryTable(table)
	cnt, err := qt.Count()
	if err != nil {
		return err
	}
	// Check if the module data file exists
	if cnt == 0 {
		fp := path.Join(mmgr.appDir, moduleDataFileName)
		if isExists, err := utils.IsFileExist(fp); !isExists {
			mmgr.log.Infoln("module data file not exist, skip module config data initialization.")
			if err != nil {
				mmgr.log.Errorf("find %v file error: %v", moduleDataFileName, err)
			}
			return nil
		}
		// Get the module configurations from the file
		configs, err := getModulesFromFile(fp)
		if err != nil {
			return err
		}
		mmgr.log.Infoln("initializing module configuration data...")
		inserter, _ := qt.PrepareInsert()
		for _, c := range configs {
			id, err := inserter.Insert(c)
			if err != nil {
				return err
			}
			mmgr.log.Infof("module(%v %v %v) inserted.", id, c.Name, c.ServiceEndpoint)
		}
		inserter.Close()
	}
	return nil
}
