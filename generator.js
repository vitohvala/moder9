import fs from "fs";

let dict = "latin.dic";
let aff = "latin.aff";
let out = "out.txt";

let allWords = [];
let wordLines = [];
let sufixLines = []

const props = {
	encoding: "utf8",
}

generateWords(dict, aff, out);

function generateWords(dict, aff, out) {

	let i = 0;

	let words = fs.readFileSync(dict, props)
	wordLines = words.split("\n")

	let sufixes = fs.readFileSync(aff, props);
	sufixLines = sufixes.split("\n");

	for(let line of wordLines){

		console.log( ++i, " / ", wordLines.length)

		let [word, sufix_id] = line.split("/");

		if(sufix_id){

			sufix_id = sufix_id.split(",")[0]

			allWords.push(word);

			if(sufix_id) checkForWords(word, sufix_id, 0)
		}
	}
	
	let allWordsString = allWords.join("\n");

	fs.writeFileSync(out, allWordsString)

}

function checkForWords(word, sufix_id, index) {
	const sufixLine = sufixLines[index];

	if(sufixLine === undefined) return;

	let [, sufixId, toDelete, sufix] = sufixLine.split(" ")


	sufix_id = sufix_id.trim();


	sufixId = sufixId.trim()
	toDelete = toDelete.trim()
	sufix = sufix.trim()

	// First line of right sufixes
	if(toDelete === "Y" && sufixId === sufix_id){
		applaySuffixes(word, index, sufix)
		return
	}

	// empty line, most likely wont heppen but just in case:
	if(sufixId === undefined){
		console.log("Empty line")
		checkForWords(word, sufix_id, index + 1);
		return;	
	}


	// first line of wrong sufixes
	if(toDelete === "Y" && sufixId !== sufix_id){
		checkForWords(word, sufix_id, index + parseInt(sufix) + 2)
		return
	}


}


function applaySuffixes(word, index, numberOfLines) {

	const lines = [];

	for(let i = 1; i <= numberOfLines; i++){
		lines.push(sufixLines[index + i])
	}

	lines.forEach(line => {
		let [, , toDelete, toAdd] = line.split(" ")
		
		toDelete = toDelete.trim();
		toAdd = toAdd.trim();

		let newWord = replaceWithSufix(word, toDelete, toAdd)

		allWords.push(newWord)
	})

}	

function replaceWithSufix(word, substring, sufix){
	word = word.slice(0, -1 * substring.length);
	word += sufix;
	return word;
}
