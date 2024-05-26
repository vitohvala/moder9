package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/url"

	"github.com/gorilla/websocket"
)

var (
	socket        *websocket.Conn
	history       []string
	allWords      []string
	filteredWords []string
	fileStatus    []FileStatus
	letters       = map[string][]string{
		"1": {" "},
		"2": {"a", "b", "c", "č", "ć"},
		"3": {"d", "e", "f", "đ"},
		"4": {"g", "h", "i"},
		"5": {"j", "k", "l"},
		"6": {"m", "n", "o"},
		"7": {"p", "r", "s", "š"},
		"8": {"t", "u", "v"},
		"9": {"z", "ž"},
	}
	// Settings as individual boolean flags
	includeFullDictionary = true
	includeEnteredWords   = true
	includePastedWords    = true
)

type FileStatus struct {
	Name     string
	Finished bool
}

func main() {
	setupFiles()
	setupWebSocket()
	for {
		handleMessage(readMessage())
	}
}

func setupFiles() {
	if includeFullDictionary {
		fileStatus = append(fileStatus, FileStatus{Name: "dict.txt", Finished: false})
	}
	if includeEnteredWords {
		fileStatus = append(fileStatus, FileStatus{Name: "entered-words.txt", Finished: false})
	}
	if includePastedWords {
		fileStatus = append(fileStatus, FileStatus{Name: "pasted-words.txt", Finished: false})
	}
}

func setupWebSocket() {
	u := url.URL{Scheme: "ws", Host: "localhost:8765", Path: "/"}
	var err error
	socket, _, err = websocket.DefaultDialer.Dial(u.String(), nil)
	if err != nil {
		log.Fatal("Error connecting to WebSocket:", err)
	}
	defer socket.Close()
	fmt.Println("WebSocket connection established.")

	// Send read signal for each file
	for _, file := range fileStatus {
		sendReadSignal(file.Name)
	}
}

func sendReadSignal(fileName string) {
	message := fmt.Sprintf(`["get_words_from_file", "%s"]`, fileName)
	socket.WriteMessage(websocket.TextMessage, []byte(message))
}

func readMessage() (string, []byte) {
	_, message, err := socket.ReadMessage()
	if err != nil {
		log.Println("read:", err)
		return "", nil
	}
	return parseEvent(message), message
}

func parseEvent(message []byte) string {
	var data []interface{}
	json.Unmarshal(message, &data)
	return data[0].(string)
}

func handleMessage(event string, message []byte) {
	switch event {
	case "press":
		handleKeyPress(event)
	case "release":
		handleKeyRelease(event)
	case "got_words":
		updateFilesStatusAndProcessWords(message)
	}
}

func updateFilesStatusAndProcessWords(message []byte) {
	var data []interface{}
	json.Unmarshal(message, &data)
	words := data[1].([]interface{})
	fileName := data[2].(string)

	for _, word := range words {
		allWords = append(allWords, word.(string))
	}

	for i, file := range fileStatus {
		if file.Name == fileName {
			fileStatus[i].Finished = true
			break
		}
	}

	if allFilesFinished() {
		sortWordsByFrequency()
		removeDuplicates()
	}
}

func allFilesFinished() bool {
	for _, file := range fileStatus {
		if !file.Finished {
			return false
		}
	}
	return true
}

func handleKeyPress(key string) {
	fmt.Println("keypressed", key)
	// Implement key press logic
}

func handleKeyRelease(key string) {
	fmt.Println("key released", key)
}

func sortWordsByFrequency() {
	// Implementation remains the same
}

func removeDuplicates() {
	// Implementation remains the same
}
