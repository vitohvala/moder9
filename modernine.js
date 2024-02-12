import WebSocket from "ws"

const socket = new WebSocket("ws://localhost:8765");

socket.onopen = function(event) {
    console.log("WebSocket connection established.");
    sendMessageToGo("hello from js")
};

socket.onmessage = function(event) {
    console.log("Message from Python:", event.data);
};

// Example of sending a message to Python
function sendMessageToGo(message) {
    socket.send(message);
}
