import {spawn} from "child_process"
import fs from 'fs'


// Variables

const phoneLayout = false;
const pasteInsteadOfTyping = true;
const useExistingJSON = false;

let allWords = []
let filteredWords = []
let history = []
let one = "", two = "", three = "", four = "", five = "";

let ctrlPressed = false;
let pasteInProgress = false;

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

function handleErrorAndClose(process){
    process.stderr.on('data', (data) => {
        console.error(`stderr: ${data}`);
    });

    process.on('close', (code) => {

        // Do not log if everything is OK
        if(code == 0) return

        console.log(`Child process exited with code ${code}`);
    });
}

function handleKeylogger(data){

    if(pasteInProgress) return;

    data = data.toString();

    if(!data) return;

    let [eventName, key ] = JSON.parse(data);

    if(eventName === "press") handleKeyPress(key);
    if(eventName === "release") handleKeyRelease(key);
}

function handleKeyPress(key){

    if(key.includes("ctrl")) ctrlPressed = true

    if(key.includes("backspace")){

        if(ctrlPressed) return restart()

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
        case "<103>":
            if(phoneLayout) return sendPasteSignal(one)
            finalKey = phoneLayout ? "1" : "7"
            break;

        case "8":
        case "<104>":
            finalKey = phoneLayout ? "2" : "8"
            break;

        case "9":
        case "<105>":
            finalKey = phoneLayout ? "3" : "9"
            break;

        case "4":
        case "<100>":
            finalKey = 4;
            break;

        case "5":
        case "<65437>":
        case "<101>":
            finalKey = 5;
            break;

        case "6":
        case "<102>":
            finalKey = 6;
            break;

        case "1":
        case "<97>":
            if(!phoneLayout) return sendPasteSignal(one)
            finalKey = phoneLayout ? "7" : "1"
            break;

        case "2":
        case "<98>":
            finalKey = phoneLayout ? "8" : "2"
            break;

        case "3":
        case "<99>":
            finalKey = phoneLayout ? "9" : "3"
            break;
    }

    if("123456789".includes(finalKey)){
        history.push(finalKey)
    }

    checkWords(history.join(""))


}

function sendPasteSignal(word){

    const pasteProcess = spawn("python3", ["paste.py", word, pasteInsteadOfTyping]);

    handleErrorAndClose(pasteProcess)

    pasteProcess.stdout.on("data", (data) => {
        data = data.toString();
        if(data.includes("pasted")) pasteInProgress = false;
    });

    restart();
}

function handleKeyRelease(key){
    if(key.includes("ctrl")) ctrlPressed = false
}


async function waitForJSONFile(wordsJsonProcess){
    await new Promise((res,rej) => {
        wordsJsonProcess.stdout.on("data", (data) => {
            data = data.toString();
            if(data.includes("ready")) res();
            else rej()
        })
    })
}

function sortWordsByFrequency(words) {
    const wordCounts = {};
    for (const word of words) {
        wordCounts[word] = (wordCounts[word] || 0) + 1;
    }

    const sortedWords = Object.keys(wordCounts).sort((a, b) => wordCounts[b] - wordCounts[a]);

    return sortedWords;
}

function restart(){
    one = "";
    two = "";
    three = "";
    four = "";
    five = "";
    history = [];
    filteredWords = [...allWords];
    pasteInProgress = false;
}


function checkWords(text){

    filteredWords = allWords.filter(word => word.length >= text.length);

    for (let index = 0; index < text.length; index++) {
        const number = text[index];
        const allowedLetters = letters[number];

        filteredWords = filteredWords.filter(word => {
            return allowedLetters.some(letter => word[index] === letter);
        });
    }


    filteredWords = [...filteredWords.filter(word => word.length === text.length), ...filteredWords.filter(word => word.length !== text.length)]
    //console.clear();

    one = filteredWords.length ? filteredWords[0] + " " : " "
    two = filteredWords.length > 1 ? filteredWords[1] + " " : " "
    three = filteredWords.length > 2 ? filteredWords[2] + " " : " "
    four = filteredWords.length > 3 ? filteredWords[3] + " " : " "
    five = filteredWords.length > 4 ? filteredWords[4] + " " : " "

    console.log(one, two, three, four, five)
}


async function main(){

    console.log("HELLO! PREPARING RESOURCES");

    if(!useExistingJSON){
        const wordsJsonProcess = spawn("python3", ["createDict.py"]);
        handleErrorAndClose(wordsJsonProcess)

        try{
            await waitForJSONFile(wordsJsonProcess);
            console.log("DICT IS CREATED");
        }
        catch(error){
            console.log("error loading JSON file", error);
            throw error;
        }
    }

    console.log("READING DICT");

    const jsonWords = fs.readFileSync("dict.json");
    allWords = JSON.parse(jsonWords).words;

    console.log("DICT LOADED, NUMBER OF WORDS: ", allWords.length);

    allWords = sortWordsByFrequency(allWords);
    allWords = [...new Set(allWords)];

    console.log("UNIQUE WORDS:", allWords.length);

    console.log("STARTING KEYLOGGER");

    const keyloggerProcess = spawn('python3', ['keylogger.py']);
    handleErrorAndClose(keyloggerProcess);
    keyloggerProcess.stdout.on('data', handleKeylogger);

    console.log("KEYLOGGER STARTED");
}





main()
