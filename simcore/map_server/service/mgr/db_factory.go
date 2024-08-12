package mgr

import (
	"path"
	"sync"

	"github.com/astaxie/beego/orm"
	_ "github.com/mattn/go-sqlite3" // register sqlite3 driver
)

var onceOrmer sync.Once
var ormerSingleton orm.Ormer
var ormerSingletonError error

func init() {
	orm.Debug = false
}

// GetDbOrm initialize and return an database object-relative-mapping object which used to operate db tables.
func GetDbOrm(appDir string) (orm.Ormer, error) {
	onceOrmer.Do(func() {
		ormerSingletonError = orm.RegisterDataBase("default", "sqlite3", path.Join(appDir, "sim.sqlite"))
		if ormerSingletonError == nil {
			orm.RegisterModel(new(ScenarioModel))
			orm.RegisterModel(new(ModuleConfig))
			ormerSingletonError = orm.RunSyncdb("default", false, true)
			if ormerSingletonError == nil {
				ormerSingleton = orm.NewOrm()
			}
		}
	})
	return ormerSingleton, ormerSingletonError
}
