import WebSocket from "ws"
import fs from 'fs'

// Global variables

let socket;

// Settings 
const phoneLayout = true;
const pasteInstadOfTyping = false;
const fullDictionary = false;

// statuses 
let ctrlPressed = false;


// Variables
let history = []
let allWords = []
let filteredwords = []
let one = "", two = "", three = "", four = "", five = "";

const files = ["pasted-words.txt", "entered-words.txt"]
fullDictionary && files.push("dict.txt")

//možda da na 1 budu naša slova, verovatno će biti preciznije predviđanje
const letters = {
    "1" : " ",
    "2" : ["a", "b", "c", "č", "ć"],
    "3" : ["d", "e", "f", "đ"],
    "4" : ["g", "h", "i"],
    "5" : ["j", "k", "l"],
    "6" : ["m", "n", "o"],
    "7" : ["p", "r", "s", "š"],
    "8" : ["t", "u", "v"],
    "9" : ["z", "ž"],
}

files.forEach(file => {

    const text = fs.readFileSync(file, {
        encoding: "utf8",
    })

    allWords = [...allWords, ...extractWords(text)];
})


allWords = sortWordsByFrequency(allWords);
allWords = [...new Set(allWords)];

//functions

function extractWords(text) {
    let words = text.match(/\b[^\d\W]+\b/g);
    words = words || [];
    return words.map(word => word.toLowerCase())
}

function sortWordsByFrequency(words) {
    const wordCounts = {};
    for (const word of words) {
      wordCounts[word] = (wordCounts[word] || 0) + 1;
    }
  
    const sortedWords = Object.keys(wordCounts).sort((a, b) => wordCounts[b] - wordCounts[a]);
  
    return sortedWords;
}
  
function setupSocket(){

    socket = new WebSocket("ws://localhost:8765");

    socket.onopen = function(event) {
        console.log("WebSocket connection established.");
    };

    socket.onmessage = function(event) {

        let [eventName, key] = JSON.parse(event.data)

        if(key.startsWith("'") && key.endsWith("'"))
            key = key.substring(1, key.length - 1);


        if(eventName === "press"){
            handleKeyPress(key)
        }
        
        if(eventName === "release")
            handleKeyRelease(key)
    
    };
}


function handleKeyPress(key){

    if(key.includes("ctrl")) ctrlPressed = true

    if(key.includes("backspace")){

        if(ctrlPressed) return reset()

        //optimizovati da samo koristi prethodni niz prosto
        history.pop()
        checkWords(history.join(""));
        return;

    }

    // Do not check first because "backspace" includes "space".
    if(key.includes("space")){
        return restart()
    }

    let finalKey = ""

    switch(key){

        case "/":
            return sendPasteSignal(two);
        
        case "*":
            return sendPasteSignal(three);

        case "-":
            return sendPasteSignal(four)

        case "+":
            return sendPasteSignal(five);

        case "7":
            if(phoneLayout) return sendPasteSignal(one)
            finalKey = phoneLayout ? "1" : "7" 
            break;

        case "8":
            finalKey = phoneLayout ? "2" : "8" 
            break;

        case "9":
            finalKey = phoneLayout ? "3" : "9" 
            break;

        case "4":
            finalKey = 4;
            break;

        case "5":
        case "<65437>":
            finalKey = 5;
            break;
        
        case "6":
            finalKey = 6;
            break;
        
        case "1":
            if(!phoneLayout) return sendPasteSignal(one)
            finalKey = phoneLayout ? "7" : "1" 
            break;

        case "2":
            finalKey = phoneLayout ? "8" : "2" 
            break;
        
        case "3":
            finalKey = phoneLayout ? "9" : "3"
            break;
    }

    if("123456789".includes(finalKey)){
        history.push(finalKey)
    }

    checkWords(history.join(""))
}

function handleKeyRelease(key){
    if(key.includes("ctrl")) ctrlPressed = false
}

function sendMessageToPython(message) {
    socket.send(message);
}

function sendPasteSignal(word){
    let message = ["paste", word, pasteInstadOfTyping, history.length]
    message = JSON.stringify(message);
    sendMessageToPython(message)
    restart();
}

function restart(){
    one = "";
    two = "";
    three = "";
    four = "";
    five = "";
    history = [];
    filteredwords = [...allWords];
}

function checkWords(text){

    filteredwords = allWords.filter(word => word.length >= text.length);

    for (let index = 0; index < text.length; index++) {
        const number = text[index];
        const allowedLetters = letters[number];

        filteredwords = filteredwords.filter(word => {
            return allowedLetters.some(letter => word[index] === letter);
        });
    }


    filteredwords = [...filteredwords.filter(word => word.length === text.length), ...filteredwords.filter(word => word.length !== text.length)]
    console.clear();

    one = filteredwords.length ? filteredwords[0] : " "
    two = filteredwords.length > 1 ? filteredwords[1] : " "
    three = filteredwords.length > 2 ? filteredwords[2] : " "
    four = filteredwords.length > 3 ? filteredwords[3] : " "
    five = filteredwords.length > 4 ? filteredwords[4] : " "

    console.log(one, two, three, four, five)
}

setupSocket();