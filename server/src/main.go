package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"net/url"
	"os"
	"strings"
	"github.com/ShuaibKhan786/EphemeralDB-client/internal/ephemeraldb"
	loadenv "github.com/ShuaibKhan786/EphemeralDB-client/internal/loadEnv"
	"github.com/rs/cors"
)

var (
	dbAddress string = ""
)

type ServerResponse struct {
	Response string `json:"response"`
}

func main() {

	
	err := loadenv.Load("../")
	if err != nil {
		log.Fatal(err)
	}

	dbAddress = os.Getenv("EPHEMERALDB")
	serverAddress := ":"+os.Getenv("PORT")

	httpAddr := serverAddress

	fmt.Println(httpAddr)

	mux := http.NewServeMux()
	mux.Handle("GET /",http.FileServer(http.Dir("../dist")))
	mux.HandleFunc("GET /v1/ephemeraldb",executeCommand)

	handler := cors.Default().Handler(mux)

	err = http.ListenAndServe(httpAddr,handler)
	if err != nil {
		log.Fatal(err)
	}
}


func executeCommand(w http.ResponseWriter,r *http.Request) {
	var res ServerResponse

	queryValue, _ := url.ParseQuery(r.URL.RawQuery)
	
	if !queryValue.Has("command") {
		res.Response = "command param not found"
		jsonData, _ := json.Marshal(res)
		sendErrorResponse(jsonData,w)
		return
	}

	formattedCommand := strings.Trim((queryValue.Get("command")),`"`)

	cmd , operation ,err := ephemeraldb.Parse(formattedCommand)
	if err != nil {
		res.Response = err.Error()
		jsonData , _ := json.Marshal(res)
		sendSuccessResponse(jsonData,w)
		return
	}

	rawBinaryData := cmd.BuildBinaryData()
	rawBinaryDataSize := uint32(len(rawBinaryData))

	conn, err := ephemeraldb.ConnectToServer(&dbAddress)
	if err != nil {
		res.Response = "ephemeraldb server is down"
		jsonData , _ := json.Marshal(res)
		sendErrorResponse(jsonData,w)
		return
	}
	defer ephemeraldb.CloseServerConnection(conn)

	servr, err := ephemeraldb.SendBinaryData(conn, rawBinaryData, rawBinaryDataSize)
	if err != nil {
		ephemeraldb.CloseServerConnection(conn)
		log.Fatal(err)
	}

	if servr == ephemeraldb.EPHEMERALDB_SUCCESS && operation == 1 {
		rawBinaryDataBack, err := ephemeraldb.RecvBinaryData(conn)
		if err != nil {
			ephemeraldb.CloseServerConnection(conn)
			log.Fatal(err)
		}

		res.Response = string(rawBinaryDataBack)
		
		jsonRes , _ := json.Marshal(res)
		sendSuccessResponse(jsonRes,w)
	} else {
		res := ServerResponse{
			Response: ephemeraldb.ParseResponse(servr),
		}
		jsonRes, _ := json.Marshal(res)
		switch servr {
		case ephemeraldb.EPHEMERALDB_SUCCESS,ephemeraldb.EPHEMERALDB_KEY_FOUND:
			sendSuccessResponse(jsonRes,w)
		default:
			w.WriteHeader(http.StatusNotFound)
			w.Header().Add("Content-Type","application/json")
			w.Write(jsonRes)
			return
		}
	}
}


func sendErrorResponse(res []byte, w http.ResponseWriter) {
	w.WriteHeader(http.StatusBadRequest)
	w.Header().Add("Content-Type","application/json")
	w.Write(res)	
}

func sendSuccessResponse(res []byte, w http.ResponseWriter) {
	w.WriteHeader(http.StatusOK)
	w.Header().Add("Content-Type","application/json")
	w.Write(res)
}
