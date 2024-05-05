package ephemeraldb

import (
	"encoding/binary"
	"net"
	"github.com/ShuaibKhan786/EphemeralDB-client/internal/endian"
)

func ConnectToServer(serverAddress *string) (*net.TCPConn, error) {
	raddr, _ := net.ResolveTCPAddr("tcp",*serverAddress)

	conn, err := net.DialTCP(raddr.Network(),nil,raddr)

	if err != nil {
		return nil , err
	}

	return conn , nil
}

func SendBinaryData(conn *net.TCPConn, rawBinaryData []byte, rawBinaryDataSize uint32) (uint8, error) {
	writeTracker := 0

	for {
		n , err := conn.Write(rawBinaryData[writeTracker:])
		if err != nil {
			return 0 ,err
		}

		writeTracker += n

		if writeTracker >= int(rawBinaryDataSize) {
			break
		}
	}

	var buffer [1]byte

	_ , err := conn.Read(buffer[:])
	if err != nil {
		return 0 ,err
	}
	
	return uint8(buffer[0]) , nil
}

func RecvBinaryData(conn *net.TCPConn) ([]byte,error) {
	var rawBinaryVS [4]byte

	_ , err := conn.Read(rawBinaryVS[:])
	if err != nil {
		return nil , err
	}
	
	actualVS := endian.NtoHl(binary.BigEndian.Uint32(rawBinaryVS[:]))

	value := make([]byte,actualVS)

	readTracker := 0

	for {
		n , err := conn.Read(value)
		if err != nil {
			return nil , err
		}
		readTracker += n

		if readTracker >= int(actualVS) {
			break;
		}
	}

	return value , nil
}

func CloseServerConnection(conn *net.TCPConn) {
	conn.Close()
}