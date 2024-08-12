package mgr

import (
	"github.com/astaxie/beego/orm"
	_ "github.com/mattn/go-sqlite3" // register sqlite3 driver
	"github.com/sirupsen/logrus"
)

var tableScenarioModel = new(ScenarioModel)

func init() {
	orm.Debug = false
}

// ModelManager handles all models stuff in scenario editor and player.
type ModelManager struct {
	log     *logrus.Logger
	ormer   orm.Ormer
	appDir  string
	TypeIdx map[string]uint32
}

// Initialize db stuff
func (mmgr *ModelManager) Initialize(l *logrus.Logger, appDir string) error {
	var err error
	mmgr.ormer, err = GetDbOrm(appDir)
	if err == nil {
		mmgr.log = l
		mmgr.appDir = appDir
		err = initScenarioModelTableData(mmgr)
	}
	return err
}

// AddModel add a model record into data base.
func (mmgr *ModelManager) AddModel(m *ScenarioModel) error {
	_, err := mmgr.ormer.Insert(m)
	return err
}

// GetAllModels retrives all model records from database.
func (mmgr *ModelManager) GetAllModels() (models []*ScenarioModel, err error) {
	_, err = mmgr.ormer.QueryTable(tableScenarioModel).All(&models)
	return
}
