// my_buffer.go

package main

import "unsafe"

// MyBuffer struct to represent a buffer allocated through cgo
type MyBuffer struct {
	cptr *cgo_MyBuffer_T
}

// NewMyBuffer creates a new MyBuffer instance with the specified size
func NewMyBuffer(size int) *MyBuffer {
	return &MyBuffer{
		cptr: cgo_NewMyBuffer(size),
	}
}

// Delete deallocates the memory associated with the MyBuffer instance
func (p *MyBuffer) Delete() {
	cgo_DeleteMyBuffer(p.cptr)
}

// Data returns a slice of the underlying buffer data
func (p *MyBuffer) Data() []byte {
	data := cgo_MyBuffer_Data(p.cptr)
	size := cgo_MyBuffer_Size(p.cptr)
	return ((*[1 << 31]byte)(unsafe.Pointer(data)))[0:int(size):int(size)]
}
