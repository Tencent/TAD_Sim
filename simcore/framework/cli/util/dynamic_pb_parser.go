package util

import (
	"log"

	"github.com/jhump/protoreflect/desc/protoparse"
	"github.com/jhump/protoreflect/dynamic"
)

// GetProtoMsg creates a new dynamic.Message instance based
// on the given message name, proto file path, and proto directories.
func GetProtoMsg(msgName string, protoPath string, protoDir []string) *dynamic.Message {
	// Create a new proto parser and add the provided proto directories to its import paths
	var parser protoparse.Parser
	parser.ImportPaths = append(parser.ImportPaths, protoDir...)

	// Parse the proto file and retrieve the file descriptors
	file_descriptors, err := parser.ParseFiles(protoPath)
	if err != nil {
		log.Println("proto file parsing error: ", err)
		return nil
	}

	// Get the first file descriptor (assuming there's only one file being parsed)
	file_descriptor := file_descriptors[0]

	// Find the message descriptor with the given name in the file descriptor
	msg_descriptor := file_descriptor.FindMessage(msgName)
	if msg_descriptor == nil {
		log.Printf("message %s not found in %s.", msgName, protoPath)
		return nil
	}

	// Create and return a new dynamic.Message instance based on the message descriptor
	return dynamic.NewMessage(msg_descriptor)
}
