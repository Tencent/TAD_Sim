package handler

import (
	"fmt"
	"path/filepath"
	"txSim/service/utils"
)

// GetModuleLaunchCommand on linux, add LD_LIBRARY_PATH env variable to support comand launching.
func GetModuleLaunchCommand(userEnv, modulePath string) string {
	execDir := utils.GetCurrentExecutableDir()
	var execPath string
	if len(modulePath) == 0 {
		execPath = filepath.Join(execDir, "txSimModuleService")
	} else {
		execPath = filepath.Join(execDir, modulePath)
	}
	depPath := filepath.Join(execDir, "simdeps")
	if len(userEnv) > 0 {
		return fmt.Sprintf("LD_LIBRARY_PATH=%s:%s %s", depPath, userEnv, execPath)
	}
	return fmt.Sprintf("LD_LIBRARY_PATH=%s %s", depPath, execPath)
}
