package utils

import (
	"fmt"
	"os"
	"path/filepath"
	"sort"
)

// NormalizeIndexMap normalize an index map so that values of keys start from 0,1,2,3...
// it also checks same value of multiple keys and, if any, panic.
// e.g. an ixdex map like {"foo":4, "bar":2, "zox":99} will be normalized to {"foo":1, "bar":0, "zox":2}
// and {"foo": 4, "bar": 4} will panic.
func NormalizeIndexMap(m map[string]uint32) {
	type pair struct {
		s string
		i uint32
	}
	pairSlice := make([]*pair, 0)
	for k, v := range m {
		pairSlice = append(pairSlice, &pair{k, v})
	}
	sort.Slice(pairSlice, func(i, j int) bool { return pairSlice[i].i < pairSlice[j].i })
	last := pairSlice[0].i
	for i, p := range pairSlice {
		if i > 0 && p.i == last {
			panic(fmt.Errorf("NormalizeIndexMap duplicate index found"))
		}
		m[p.s] = uint32(i)
		last = p.i
	}
}

// GetCurrentExecutableDir get the absolute path of the directory of current running executable.
func GetCurrentExecutableDir() string {
	dir, err := filepath.Abs(filepath.Dir(os.Args[0]))
	if err != nil {
		panic(fmt.Errorf("cannot get current executable path"))
	}
	return dir
}

// IsFileExist check if the path provided by f is exist.
// return true if exists, false otherwise.
func IsFileExist(f string) (bool, error) {
	if _, err := os.Stat(f); err == nil {
		return true, nil
	} else if os.IsNotExist(err) {
		return false, nil
	} else {
		return false, err
	}
}
