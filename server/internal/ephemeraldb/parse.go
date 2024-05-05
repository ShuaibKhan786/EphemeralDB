package ephemeraldb

import (
	"errors"
	"strings"
)

func extractCmd(cmd string) uint8 {
	switch strings.ToUpper(cmd) {
	case "GET":
		return 1
	case "SET":
		return 2
	case "UPD":
		return 3
	case "DEL":
		return 4
	case "EXIST":
		return 5
	default:
		return 0
	}
}

func Parse(data string) (Command, uint8, error) {
	dataParts := strings.SplitN(data, " ", 3)
	if len(dataParts) < 2 {
		return nil, 0, errors.New("(syntax error) :insufficient data")
	}

	cmd := extractCmd(dataParts[0])
	if cmd == 0 {
		return nil, 0, errors.New("(syntax error) :invalid command")
	}

	key := []byte(dataParts[1])
	keySize := uint16(len(key))

	switch cmd {
	case 1, 4, 5:
		if len(dataParts) >= 3 {
			return nil, 0, errors.New("(syntax error) :value must not used for GET/DEL command")
		}
		return &GETDEL{
			OpCode: cmd,
			WDs:    uint16(GETDELPREFIXSIZE + keySize),
			KS:     keySize,
			K:      key,
		}, cmd, nil
	case 2, 3:
		if len(dataParts) < 3 {
			return nil, 0, errors.New("(syntax error) :value missing for SET/UPD command")
		}
		value := []byte(dataParts[2])
		valueSize := uint32(len(value))
		return &SETUPD{
			OpCode: cmd,
			WDs:    uint32(SETUPDPREFIXSIZE + uint32(keySize) + valueSize),
			KS:     keySize,
			K:      key,
			VS:     valueSize,
			V:      value,
		}, cmd, nil
	default:
		return nil, 0, errors.New("(error) unknown command")
	}
}

func ParseResponse(resNo uint8) string {
	switch resNo {
	case EPHEMERALDB_FAIL:
		return "Fail"
	case EPHEMERALDB_SUCCESS:
		return "Ok"
	case EPHEMERALDB_KEY_NOT_FOUND:
		return "key does not exist"
	case EPHEMERALDB_KEY_FOUND:
		return "key exist"
	case EPHEMERALDB_RESEND:
		return "resend the data"
	case EPHEMERALDB_DATA_LIMIT:
		return "data size exceed"
	default:
		return "Fail"
	}
}
