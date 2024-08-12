package cmd

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"time"
	"txsim/cli/msg"

	"github.com/spf13/cobra"
	"github.com/spf13/viper"
)

// flagPauseOnStart indicates whether to pause the scenario at the start
var flagPauseOnStart bool

// RunParams holds the parameters for the run command
type RunParams struct {
	Scenario string `json:"scenarioPath"`
	Pause    bool   `json:"pauseOnStart"`
}

// runCmd represents the run command
var runCmd = &cobra.Command{
	Use:   "run",
	Short: "setup and start running a specified scenario.",
	Long: `request the TAD Sim to setup and start running a new specified scenario. optionally use --pause to indicates
it to pause at the beginning of the scenario simulation. For example:

txsim-cli play run --pause /absolute/path/to/my/scenario.sim

the command is asynchronous and returned immediately. using data commands to get the status/messages info of the
running scenario.`,
	Args: func(cmd *cobra.Command, args []string) error {
		// Validate that a scenario file path is provided
		if len(args) < 1 {
			return errors.New("must specify the path of the scenario file to be run")
		}

		// Check if the specified scenario file exists
		if _, err := os.Stat(args[0]); err == nil {
			return nil
		} else if errors.Is(err, os.ErrNotExist) {
			return errors.New("the scenario file specified not exist")
		} else {
			return err
		}
	},
	Run: func(cmd *cobra.Command, args []string) {
		// Create run parameters
		runParams := RunParams{
			Scenario: args[0],
			Pause:    flagPauseOnStart,
		}

		// Marshal run parameters to JSON
		runBodyJson, _ := json.Marshal(runParams)
		client := &http.Client{
			Timeout: time.Second * 3,
		}
		endpoint := viper.GetString(cfgKeyEndpoint)
		url := fmt.Sprintf("http://%s/api/play/run", endpoint)

		// Make a POST request to start the scenario
		resp, err := client.Post(url, "application/json", bytes.NewBuffer(runBodyJson))
		if err != nil {
			log.Println("run request error: ", err)
			return
		}
		defer resp.Body.Close()

		// Check if the response status is OK
		if resp.StatusCode != http.StatusOK {
			log.Println("run response status error: ", resp.Status)
			return
		}

		// Read the response body
		body, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			log.Println("run response body read error: ", err)
			return
		}

		// Unmarshal the response body into SimGeneralResponseBody struct
		runRst := msg.SimGeneralResponseBody{}
		json.Unmarshal(body, &runRst)

		// Check the response code and print appropriate message
		if runRst.Code == 0 {
			if flagPauseOnStart {
				fmt.Println("Scenario started running.")
			} else {
				fmt.Println("Scenario is ready to run.")
			}
		} else {
			fmt.Printf("[ERROR] Scenario run failed[%d]: %s\n", runRst.Code, runRst.Msg)
		}
	},
}

func init() {
	// Add the run command to playCmd
	playCmd.AddCommand(runCmd)
	// Add the pause flag to the run command
	runCmd.Flags().BoolVarP(&flagPauseOnStart, "pause", "p", false, "immediately pause on scenario beginning.")
}
