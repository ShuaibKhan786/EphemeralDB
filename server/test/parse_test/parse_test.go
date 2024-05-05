package parsetest

import (
	"fmt"
	"reflect"
	"testing"

	"github.com/ShuaibKhan786/EphemeralDB-client/internal/ephemeraldb"
)

func TestParse(t *testing.T){
	t.Run("SET UPD",func(t *testing.T) {
		got , _ ,_ := ephemeraldb.Parse("SET car Porche Lamborgini Ferrai")

		key := []byte("car")
		key = append(key,0)
		keySize := len(key)
		value := []byte("Porche Lamborgini Ferrai")
		value = append(value,0)
		valueSize := len(value)

		want := &ephemeraldb.SETUPD{
			OpCode: uint8(2),
			WDs: uint32(7+uint32(keySize)+uint32(valueSize)),
			KS: uint16(keySize),
			K: key,
			VS: uint32(valueSize),
			V: value,
		}

		if !reflect.DeepEqual(got,want) {
			t.Errorf("got : %v and want : %v",got, want)
		}
	})
	t.Run("SET UPD",func(t *testing.T) {
		got , _ , err := ephemeraldb.Parse("GET car")

		if err != nil {
			fmt.Println(err)
		}

		key := []byte("car")
		key = append(key,0)
		keySize := len(key)

		want := &ephemeraldb.GETDEL{
			OpCode: uint8(1),
			WDs: uint16(5+uint16(keySize)),
			KS: uint16(keySize),
			K: key,
		}

		if !reflect.DeepEqual(got,want) {
			t.Errorf("got : %v and want : %v",got, want)
		}
	})
}

