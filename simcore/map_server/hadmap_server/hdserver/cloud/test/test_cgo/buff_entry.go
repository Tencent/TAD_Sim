package main

//#include <stdio.h>
import "C"
import "unsafe"

func main() {
	// Create a new instance of MyBuffer with 1024 bytes of space
	buf := NewMyBuffer(1024)
	// Ensure the buffer memory is released when the function returns
	defer buf.Delete()

	// Copy the string "hello\x00" into the buffer's Data() slice
	copy(buf.Data(), []byte("hello\x00"))
	// Use the C language's puts function to output the data in the buffer
	// Convert the Go []byte type to C *char type using unsafe package for pointer conversion
	C.puts((*C.char)(unsafe.Pointer(&(buf.Data()[0]))))
}
