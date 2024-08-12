package mgr

import (
	"time"
)

// ScenarioModel is the object mapping to data base scenario_model table.
type ScenarioModel struct {
	ID      uint32    `orm:"auto;pk;column(id)" json:"id"`
	Name    string    `json:"name"`
	Type    string    `json:"type,omitempty"`
	Created time.Time `orm:"auto_now_add;type(datetime)" json:"-"`
	Updated time.Time `orm:"auto_now;type(datetime)" json:"-"`
	// Image2D string    `orm:"column(image_2D)" json:"image"`
	// Image3D string    `orm:"column(image_3D)" json:"modelImage"`
}

// TableUnique defines an unique key in database.
func (m *ScenarioModel) TableUnique() [][]string {
	return [][]string{
		[]string{"Name", "Type"},
	}
}

// ModuleConfig is the object mapping to database module_config table.
type ModuleConfig struct {
	ID                uint32    `orm:"auto;pk;column(id)" json:"-"`
	Name              string    `json:"name"`
	StepTime          float64   `json:"stepTime"`
	ServiceEndpoint   string    `json:"endpoint"`
	Timeout           uint32    `orm:"null" json:"timeout"`
	LibPath           string    `orm:"null" json:"libPath"`
	EnvironmentPrefix string    `orm:"null" json:"envPrefix"`
	LaunchCommand     string    `orm:"null" json:"command"`
	CommandArgs       string    `orm:"null" json:"commandArgs"`
	InitParams        string    `orm:"null" json:"initParams"`
	Type              uint8     `json:"type"`
	UserID            int32     `orm:"column(user_id)" json:"userID"`
	Created           time.Time `orm:"auto_now_add;type(datetime)" json:"-"`
	Updated           time.Time `orm:"auto_now;type(datetime)" json:"-"`
}
