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

// stepCmd represents the step command
var stepCmd = &cobra.Command{
	Use:   "step",
	Short: "take a step on the currently paused scenario.",
	Long: `take a step on the currently paused scenario. it is invalid if there's no scenario setup or one is
currently running.`,
	Run: func(cmd *cobra.Command, args []string) {
		// Create a new HTTP client with a timeout
		client := &http.Client{
			Timeout: time.Second * 3,
		}

		// Make a POST request to step the scenario
		resp, err := client.Post(fmt.Sprintf("http://%s/api/play/step", viper.GetString(cfgKeyEndpoint)), "", nil)
		if err != nil {
			log.Println("step request error: ", err)
			return
		}
		defer resp.Body.Close()

		// Check if the response status is OK
		if resp.StatusCode != http.StatusOK {
			log.Println("step response status error: ", resp.Status)
			return
		}

		// Read the response body
		body, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			log.Println("step response body read error: ", err)
			return
		}

		// Unmarshal the response body into SimGeneralResponseBody struct
		rst := msg.SimGeneralResponseBody{}
		json.Unmarshal(body, &rst)

		// Check the response code and print appropriate message
		if rst.Code == 100 {
			fmt.Println("Scenario step finished.")
		} else {
			fmt.Printf("[ERROR] Scenario stepping failed[%d]: %s\n", rst.Code, rst.Msg)
		}
	},
}

func init() {
	// Add the step command to playCmd
	playCmd.AddCommand(stepCmd)
}
