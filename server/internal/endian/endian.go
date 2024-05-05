package endian

import (
    "encoding/binary"
)

func HtoNs(h uint16) uint16 {
    if isLE() {
        return swap16(h)
    }
    return h
}

func NtoHs(n uint16) uint16 {
    return HtoNs(n)
}

func HtoNl(h uint32) uint32 {
    if isLE() {
        return swap32(h)
    }
    return h
}

func NtoHl (n uint32) uint32 {
    return HtoNl(n)
}

func isLE() bool {
    var num uint16 = 0x0102
    var b [2]byte
    binary.BigEndian.PutUint16(b[:], num)
    return b[0] == 2 
}

func swap16(val uint16) uint16 {
    return (val>>8)&0xFF | (val<<8)&0xFF00
}

func swap32(val uint32) uint32 {
    return (val>>24)&0xFF | (val<<24)&0xFF000000 |
        (val>>8)&0xFF00 | (val<<8)&0xFF0000
}
