package utils_test

import (
	"reflect"
	"testing"
)

func TestIndexMapNormalization(t *testing.T) {
	m1 := map[string]uint32{"foo": 4, "bar": 2, "zox": 99}
	m1Expected := map[string]uint32{"foo": 1, "bar": 0, "zox": 2}
	NormalizeIndexMap(m1)
	if !reflect.DeepEqual(m1, m1Expected) {
		t.Fail()
	}
	m2 := map[string]uint32{"foo": 4, "bar": 4, "zox": 99}
	defer func() {
		if r := recover(); r == nil {
			t.Fail()
		}
	}()
	NormalizeIndexMap(m2)
}
