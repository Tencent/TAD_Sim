package main

import (
	"log"
	"fmt"
	"github.com/golang/protobuf/proto"
)

func main() {
	test := &CloudUploadHadmapCallback{
		MapNames :[]string{"map1","map2","使用"},
	}
	data, err := proto.Marshal(test)
	if err != nil {
		log.Fatal("marshaling error: ", err)
	}
	newTest := &CloudUploadHadmapCallback{}
	err = proto.Unmarshal(data, newTest)
	if err != nil {
		log.Fatal("unmarshaling error: ", err)
	}
	// Now test and newTest contain the same data.

	for _, v := range test.GetMapNames() {
		fmt.Println("name is: ", v)
	}
	fmt.Println("new test names: ")

	for _, v := range newTest.GetMapNames() {
		fmt.Println("name is: ", v)
	}
	//if test.GetMapNames() != newTest.GetMapNames() {
	//	panic("map names not equal")
		//log.Fatalf("data mismatch %q != %q", test.GetName(), newTest.GetName())
	//}
}
