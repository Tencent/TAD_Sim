package cmd

import (
	"fmt"
	"os"

	"github.com/spf13/cobra"

	"github.com/spf13/viper"
)

var cfgFile string

const cfgKeyEndpoint = "server.endpoint"

// rootCmd represents the base command when called without any subcommands
var rootCmd = &cobra.Command{
	Use:   "txsim-cli",
	Short: "an CLI(command line interface) tool for the TAD Sim",
	Long:  `an CLI(command line interface) tool for the TAD Sim.`,
}

// Execute adds all child commands to the root command and sets flags appropriately.
// This is called by main.main(). It only needs to happen once to the rootCmd.
func Execute() {
	cobra.CheckErr(rootCmd.Execute())
}

func init() {
	cobra.OnInitialize(initConfig)

	rootCmd.PersistentFlags().StringVar(&cfgFile, "config", "", "config file (default is $HOME/txsim-cli.config)")
	rootCmd.PersistentFlags().String("endpoint", "127.0.0.1:8302", "the endpoint of the TAD Sim backend service")
	viper.BindPFlag(cfgKeyEndpoint, rootCmd.PersistentFlags().Lookup("endpoint"))
}

// initConfig reads in config file and ENV variables if set.
func initConfig() {
	if cfgFile != "" {
		// Use config file from the flag.
		viper.SetConfigFile(cfgFile)
	} else {
		// Find home directory.
		home, err := os.UserHomeDir()
		cobra.CheckErr(err)

		// Search config in home directory with name "txsim-cli.config" (without extension).
		viper.AddConfigPath(home)
		viper.SetConfigName("txsim-cli.config")
		viper.SetConfigType("ini")
	}

	viper.AutomaticEnv() // read in environment variables that match

	// If a config file is found, read it in.
	err := viper.ReadInConfig()
	if err == nil {
		fmt.Fprintln(os.Stderr, "Using config file:", viper.ConfigFileUsed())
	} else {
		if _, ok := err.(viper.ConfigFileNotFoundError); !ok {
			fmt.Fprintf(os.Stderr, "Reading config file %s error: %s", viper.ConfigFileUsed(), err)
		}
	}
}
