package util

import (
	"log"
	"os"
	"os/signal"
	"syscall"
)

// MonitorInterruptSignal listens for interrupt signals and notifies the provided channel when an interrupt is received.
func MonitorInterruptSignal(c chan<- bool) {
	// Create a channel to receive OS signals
	s := make(chan os.Signal, 1)

	// Notify the channel on receiving an interrupt signal or SIGTERM
	signal.Notify(s, os.Interrupt, syscall.SIGTERM)

	// Wait for a signal to be received
	<-s

	// Log that an interrupt signal has been received
	log.Printf("received interrupt signal.")

	// Send a notification to the provided channel
	c <- true
}
