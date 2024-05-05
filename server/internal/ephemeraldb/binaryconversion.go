package ephemeraldb

import (
	"encoding/binary"
	"github.com/ShuaibKhan786/EphemeralDB-client/internal/endian"
)

const (
	GETDELPREFIXSIZE = 5
	SETUPDPREFIXSIZE = 11
)

type Command interface {
	BuildBinaryData() []byte
}

type GETDEL struct {
	OpCode uint8
	WDs    uint16
	KS     uint16
	K      []byte
}

type SETUPD struct {
	OpCode uint8
	WDs    uint32
	KS     uint16
	K      []byte
	VS     uint32
	V      []byte
}

func (gd *GETDEL) BuildBinaryData() []byte {
	rawBinaryData := make([]byte, (gd.WDs))

	rawBinaryData[0] = gd.OpCode
	binary.BigEndian.PutUint16(rawBinaryData[1:3], endian.HtoNs(gd.WDs))
	binary.BigEndian.PutUint16(rawBinaryData[3:5], endian.HtoNs(gd.KS))
	copy(rawBinaryData[5:], gd.K)

	return rawBinaryData
}

func (su *SETUPD) BuildBinaryData() []byte {
	rawBinaryData := make([]byte, su.WDs)

	rawBinaryData[0] = su.OpCode
	binary.BigEndian.PutUint32(rawBinaryData[1:5], endian.HtoNl(su.WDs))
	binary.BigEndian.PutUint16(rawBinaryData[5:7], endian.HtoNs(su.KS))
	copy(rawBinaryData[7:7+su.KS], su.K)
	binary.BigEndian.PutUint32(rawBinaryData[7+su.KS:11+su.KS], endian.HtoNl(su.VS))
	copy(rawBinaryData[11+su.KS:], su.V)

	return rawBinaryData
}
