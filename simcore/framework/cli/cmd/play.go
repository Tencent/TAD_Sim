package cmd

import (
	"github.com/spf13/cobra"
)

// playCmd is the root command for scenario playing related sub-commands
var playCmd = &cobra.Command{
	Use:   "play",
	Short: "root command for scenario playing relative sub-commands",
	Long:  `root command for scenario playing relative sub-commands. including run, step, stop, pause, resume.`,
}

func init() {
	// Add playCmd as a sub-command to the root command
	rootCmd.AddCommand(playCmd)
}
