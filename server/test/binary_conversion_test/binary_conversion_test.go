package binary_conversion_test

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"testing"
	"github.com/ShuaibKhan786/EphemeralDB-client/internal/endian"
	"github.com/ShuaibKhan786/EphemeralDB-client/internal/ephemeraldb"
)

func TestBinaryConversion(t *testing.T) {
	t.Run("get and del testing", func(t *testing.T) {
		gd := &ephemeraldb.GETDEL{
			OpCode: 1,
			WDs:    8,
			KS:     3,
			K:      []byte("car"),
		}
		got := gd.BuildBinaryData()
		want := make([]byte, 8)
		want[0] = 1
		binary.BigEndian.PutUint16(want[1:3], endian.HtoNs(8))
		binary.BigEndian.PutUint16(want[3:5], endian.HtoNs(3))
		copy(want[5:], []byte("car"))

		fmt.Println(got)
		fmt.Println(want)

		if !bytes.Equal(got, want) {
			t.Errorf("got : %v and want %v", string(got), string(want))
		}
	})
}
