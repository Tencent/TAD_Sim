package mgr

import (
	"github.com/astaxie/beego/orm"
	_ "github.com/mattn/go-sqlite3" // register sqlite3 driver
	"github.com/sirupsen/logrus"
)

var tableModuleConfig = new(ModuleConfig)

func init() {
	orm.Debug = false
}

// ModuleManager handles all module configuration stuff.
type ModuleManager struct {
	log    *logrus.Logger
	ormer  orm.Ormer
	appDir string
}

// Initialize db stuff
func (mmgr *ModuleManager) Initialize(l *logrus.Logger, appDir string) error {
	var err error
	mmgr.ormer, err = GetDbOrm(appDir)
	if err == nil {
		mmgr.log = l
		mmgr.appDir = appDir
		err = initModuleConfigTableData(mmgr)
	}
	return err
}

// GetAllModules retrives all module configuration records from database.
func (mmgr *ModuleManager) GetAllModules() (modules []*ModuleConfig, err error) {
	_, err = mmgr.ormer.QueryTable(tableModuleConfig).All(&modules)
	return
}
