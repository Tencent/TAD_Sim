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

// resumeCmd represents the resume command
var resumeCmd = &cobra.Command{
	Use:   "resume",
	Short: "resume the currently paused scenario.",
	Long: `resume the currently paused scenario. ignored if there's no scenario setup or one is already running.

the command is asynchronous and returned immediately. using data commands to get the status/messages info of
the running scenario.`,
	Run: func(cmd *cobra.Command, args []string) {
		// Create a new HTTP client with a timeout
		client := &http.Client{
			Timeout: time.Second * 3,
		}

		// Make a POST request to resume the scenario
		resp, err := client.Post(fmt.Sprintf("http://%s/api/play/resume", viper.GetString(cfgKeyEndpoint)), "", nil)
		if err != nil {
			log.Println("resume request error: ", err)
			return
		}
		defer resp.Body.Close()

		// Check if the response status is OK
		if resp.StatusCode != http.StatusOK {
			log.Println("resume response status error: ", resp.Status)
			return
		}

		// Read the response body
		body, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			log.Println("resume response body read error: ", err)
			return
		}

		// Unmarshal the response body into SimGeneralResponseBody struct
		rst := msg.SimGeneralResponseBody{}
		json.Unmarshal(body, &rst)

		// Check the response code and print appropriate message
		if rst.Code == 0 {
			fmt.Println("Scenario resumed.")
		} else {
			fmt.Printf("[ERROR] Scenario resuming failed[%d]: %s\n", rst.Code, rst.Msg)
		}
	},
}

func init() {
	// Add the resume command to playCmd
	playCmd.AddCommand(resumeCmd)
}
