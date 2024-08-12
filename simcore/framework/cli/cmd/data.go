package cmd

import (
	"github.com/spf13/cobra"
)

// dataCmd represents the data command
var dataCmd = &cobra.Command{
	Use:   "data",
	Short: "root command for data relative sub-commands",
	Long:  `root command for data relative sub-commands. currently including msg and status.`,
}

func init() {
	// Add the pause command to dataCmd
	rootCmd.AddCommand(dataCmd)
}
