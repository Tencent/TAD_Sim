package cmd

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"time"
	"txsim/cli/msg"

	"github.com/spf13/cobra"
	"github.com/spf13/viper"
)

// stopCmd represents the stop command
var stopCmd = &cobra.Command{
	Use:   "stop",
	Short: "stop the currently running scenario.",
	Long: `stop the currently running scenario. after this operation the command pause, resume, step will no longer
valid until the next run command issued.`,
	Run: func(cmd *cobra.Command, args []string) {
		// Create a new HTTP client with a timeout
		client := &http.Client{
			Timeout: time.Second * 3,
		}

		// Make a POST request to stop the scenario
		resp, err := client.Post(fmt.Sprintf("http://%s/api/play/stop", viper.GetString(cfgKeyEndpoint)), "", nil)
		if err != nil {
			log.Println("stop request error: ", err)
			return
		}
		defer resp.Body.Close()

		// Check if the response status is OK
		if resp.StatusCode != http.StatusOK {
			log.Println("stop response status error: ", resp.Status)
			return
		}

		// Read the response body
		body, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			log.Println("stop response body read error: ", err)
			return
		}

		// Unmarshal the response body into SimGeneralResponseBody struct
		rst := msg.SimGeneralResponseBody{}
		json.Unmarshal(body, &rst)

		// Check the response code and print appropriate message
		if rst.Code == 100 {
			fmt.Println("Scenario Stopped.")
		} else {
			fmt.Printf("[ERROR] Scenario stop failed[%d]: %s\n", rst.Code, rst.Msg)
		}
	},
}

func init() {
	// Add the stop command to playCmd
	playCmd.AddCommand(stopCmd)
}
