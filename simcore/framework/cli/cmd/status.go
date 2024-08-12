package cmd

import (
	"context"
	"encoding/json"
	"fmt"
	"log"
	"strings"
	"txsim/cli/util"

	"github.com/spf13/cobra"
	"github.com/spf13/viper"
	"nhooyr.io/websocket"
)

// PublishedShmemTopic represents a published shared memory topic
type PublishedShmemTopic struct {
	Name      string `json:"name"`
	SizeBytes string `json:"size"`
}

// TopicInfo holds information about topic subscriptions and publications
type TopicInfo struct {
	Subs      []string              `json:"subs,omitempty"`
	Pubs      []string              `json:"pubs,omitempty"`
	SubShmems []string              `json:"shmemSubs,omitempty"`
	PubShmems []PublishedShmemTopic `json:"shmemPubs,omitempty"`
}

// ModuleInitStatus contains initialization status of a module
type ModuleInitStatus struct {
	Name          string    `json:"name"`
	State         int16     `json:"state"`
	Topics        TopicInfo `json:"topicInfo,omitempty"`
	LogFileName   string    `json:"logPath,omitempty"`
	BinaryUpdated bool      `json:"binaryUpdated"`
}

// ModulePlayStatus contains runtime status of a module
type ModulePlayStatus struct {
	Name            string `json:"name"`
	ErrCode         int16  `json:"ec"`
	WallTimeElapsed uint32 `json:"elapsedTime"`
	MemoryUsed      uint32 `json:"memoryUsed"`
	CpuTimeUs       uint32 `json:"cpuTimeUser"`
	CpuTimeSy       uint32 `json:"cpuTimeSys"`
	Fps             uint16 `json:"fps"`
	Msg             string `json:"msg,omitempty"`
	Feedback        string `json:"feedback,omitempty"`
}

// TopicMessage represents a message in a topic
type TopicMessage struct {
	Topic     string `json:"topic"`
	SizeBytes uint64 `json:"size"`
}

// ModuleStepMessage contains information about a simulation step
type ModuleStepMessage struct {
	SimTimestamp uint64         `json:"timestamp"`
	Messages     []TopicMessage `json:"msgInfo,omitempty"`
}

// SimStatus contains the entire simulation status
type SimStatus struct {
	ErrCode              int16              `json:"ec"`
	WallTimeElapsed      uint32             `json:"totalTimeCost"`
	CpuTimeUs            float64            `json:"totalCpuTimeUser"`
	CpuTimeSy            float64            `json:"totalCpuTimeSys"`
	ModulePlayStatusInfo []ModulePlayStatus `json:"moduleStatus,omitempty"`
	ModuleInitStatusInfo []ModuleInitStatus `json:"initStatus,omitempty"`
	SimMessages          ModuleStepMessage  `json:"stepMessage,omitempty"`
}

// PrintFormattedStatus prints the formatted simulation status
func PrintFormattedStatus(status *SimStatus) {
	fmt.Println("========================================")
	fmt.Println("Error code: ", status.ErrCode)
	fmt.Printf(
		"Total: wall time elapsed -> %dms, "+
		"cpu time used -> (%fms user, %fms sys)\n",
		status.WallTimeElapsed,
		status.CpuTimeUs,
		status.CpuTimeSy,
	)

	// Print module initialization status
	if len(status.ModuleInitStatusInfo) > 0 {
		fmt.Println("Module Init Status:")
		sb := new(strings.Builder)
		for _, m := range status.ModuleInitStatusInfo {
			fmt.Fprintf(sb, "  %s: state -> %d", m.Name, m.State)
			if len(m.LogFileName) > 0 {
				fmt.Fprintf(sb, ", log file name -> %s", m.LogFileName)
			}
			if len(m.Topics.Pubs) > 0 {
				fmt.Fprintf(sb, ", pubs -> [%s]", strings.Join(m.Topics.Pubs, ", "))
			}
			if len(m.Topics.PubShmems) > 0 {
				t := make([]string, len(m.Topics.PubShmems))
				for i, v := range m.Topics.PubShmems {
					t[i] = "(" + v.Name + "," + v.SizeBytes + ")"
				}
				fmt.Fprintf(sb, ", shmem pubs -> [%s]", strings.Join(t, ", "))
			}
			if len(m.Topics.Subs) > 0 {
				fmt.Fprintf(sb, ", subs -> [%s]", strings.Join(m.Topics.Subs, ", "))
			}
			if len(m.Topics.SubShmems) > 0 {
				fmt.Fprintf(sb, ", shmem subs -> [%s]", strings.Join(m.Topics.SubShmems, ", "))
			}
			if m.BinaryUpdated {
				fmt.Fprint(sb, ", module binary updated.")
			}
			fmt.Fprintln(sb)
		}
		fmt.Print(sb.String())
	}

	// Print module play status
	if len(status.ModulePlayStatusInfo) > 0 {
		fmt.Println("Module play Status:")
		sb := new(strings.Builder)
		for _, m := range status.ModulePlayStatusInfo {
			fmt.Fprintf(
				sb,
				"  %s: ec -> %d, wall time elapsed -> %dms, fps -> %d, "+
				"cpu time used -> (%dus user, %dus sys), memory used -> %dkb",
				m.Name,
				m.ErrCode,
				m.WallTimeElapsed,
				m.Fps,
				m.CpuTimeUs,
				m.CpuTimeSy,
				m.MemoryUsed,
			)
			if len(m.Msg) > 0 {
				fmt.Fprintf(sb, ", msg -> %s", m.Msg)
			}
			if len(m.Feedback) > 0 {
				fmt.Fprintf(sb, ", feedback -> %s", m.Feedback)
			}
			fmt.Fprintln(sb)
		}
		fmt.Print(sb.String())
	}

	// Print simulation messages
	{
		sb := new(strings.Builder)
		fmt.Fprintf(sb, "%d msgs @ step %dms:", len(status.SimMessages.Messages), status.SimMessages.SimTimestamp)
		for _, m := range status.SimMessages.Messages {
			fmt.Fprintf(sb, " <%s, %d>", m.Topic, m.SizeBytes)
		}
		fmt.Println(sb.String())
	}
}

// receiveStatus listens for simulation status messages over a WebSocket connection
func receiveStatus(ctx *context.Context, conn *websocket.Conn, c chan<- bool) {
	for {
		// Read message from WebSocket connection
		msg_type, payload, err := conn.Read(*ctx)
		if err != nil {
			log.Printf("ws client read error: %s", err)
			break
		}

		//log.Printf("===== ws read: msg type -> %d msg length -> %d", msg_type, len(payload))
		// If message type is text, unmarshal and print status
		if msg_type == websocket.MessageText {
			status := SimStatus{}
			err := json.Unmarshal(payload, &status)
			if err != nil {
				log.Println("error parsing status json payload: ", err)
			} else {
				PrintFormattedStatus(&status)
			}
		} else {
			log.Println("un-expected binary payload received.")
		}
	}

	// Notify that the status receiving has ended
	c <- true
}

// msgCmd represents the msg command
var statusCmd = &cobra.Command{
	Use:   "status",
	Short: "monitoring the scenario playing status of the TAD Sim.",
	Long:  `monitoring all kinds of the status(elapsed wall/cpu time, error code, topic/message meta info, etc.) of the scenario playing in the TAD Sim.`,
	Run: func(cmd *cobra.Command, args []string) {
		// Notify that the status receiving has ended
		ctx, cancel := context.WithCancel(context.Background())
		defer cancel()

		// Establish WebSocket connection
		conn, resp, err := websocket.Dial(ctx, fmt.Sprintf("ws://%s/api/data/status", viper.GetString(cfgKeyEndpoint)), nil)
		if err != nil {
			if resp != nil {
				log.Printf("ws client dial resp status: %s", resp.Status)
			}
			log.Printf("ws client dial error: %s.", err)
			if conn != nil {
				conn.Close(websocket.StatusInternalError, "ws client error")
			}
			return
		}
		defer conn.Close(websocket.StatusNormalClosure, "")

		// Channel to signal the end of status receiving
		c := make(chan bool)
		go receiveStatus(&ctx, conn, c)
		go util.MonitorInterruptSignal(c)
		<-c
	},
}

func init() {
	// Add the status command to dataCmd
	dataCmd.AddCommand(statusCmd)
}
