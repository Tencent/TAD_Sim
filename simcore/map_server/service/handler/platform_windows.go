package handler

import (
	"path/filepath"
	"txSim/service/utils"
)

// GetModuleLaunchCommand on windows, do NOT support env variable currently.
func GetModuleLaunchCommand(userEnv, modulePath string) string {
	execDir := utils.GetCurrentExecutableDir()
	var execPath string
	if len(modulePath) == 0 {
		execPath = filepath.Join(execDir, "..", "txSimModuleService.exe")
	} else {
		execPath = filepath.Join(execDir, "..", modulePath)
	}
	return execPath
}
