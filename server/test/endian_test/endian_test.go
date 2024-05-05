package endian_testing

import (
	"testing"
	"unsafe"

	"github.com/ShuaibKhan786/EphemeralDB-client/internal/endian"
)

func isLE() bool {
    var i int32 = 1
    return *(*uint8)(unsafe.Pointer(&i)) == 1
}

func TestEndian (t *testing.T) {
    var inp16 uint16 = 0x1234
    var out16 uint16 = 0x3412

    var inp32 uint32 = 0x00000001
    var out32 uint32 = 0x01000000

    t.Run("Host to Network 2 bytes",func(t *testing.T){
        if !isLE() {
            checkerUint16(t,endian.HtoNs(inp16),inp16)
        }
        checkerUint16(t,endian.HtoNs(inp16),out16)
    })
 
    t.Run("Network to Host 2 bytes",func(t *testing.T){
         if !isLE() {
            checkerUint16(t,endian.NtoHs(inp16),inp16)
        }
        checkerUint16(t,endian.NtoHs(inp16),out16)        
    })   

    t.Run("Host to Network 4 bytes",func(t *testing.T){
        if !isLE() {
            checkerUint32(t,endian.HtoNl(inp32),inp32)
        }
        checkerUint32(t,endian.HtoNl(inp32),out32)
    })

    t.Run("Network to Host 4 bytes",func(t *testing.T){
         if !isLE() {
            checkerUint32(t,endian.NtoHl(inp32),inp32)
        }
        checkerUint32(t,endian.NtoHl(inp32),out32)       
    })
}

func checkerUint16(t testing.TB,got,want uint16){
    t.Helper()
    
    if got != want {
        t.Errorf("got : %v and want : %v",got,want)
    }
}
func checkerUint32(t testing.TB,got,want uint32){
    t.Helper()
    
    if got != want {
        t.Errorf("got : %v and want : %v",got,want)
    }
}
