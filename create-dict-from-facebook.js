import fs, { readFileSync } from "fs"
import path from "path";

const mainFolder = "facebook-messages"
let allJsonFiles = []
const allMessages = []

function fixEncoding(input) {
    try {
        return decodeURIComponent(escape(input));
    } catch (e) {
        console.error("Error in decoding:", e);
        return input; // Vrati originalni ulaz ako dođe do greške
    }
}

const dir = fs.readdirSync(mainFolder);

dir.forEach(personDir => {
    let files = fs.readdirSync(path.join(mainFolder, personDir));

    files = files.map(f => path.join(mainFolder, personDir, f))
    allJsonFiles = [...allJsonFiles, ...files];
})

allJsonFiles.forEach(file => {
    const fileContent = readFileSync(file);
    const jsMessagesObject = JSON.parse(fileContent.toString('utf8'));

    let messagesArray = jsMessagesObject.messages;

    messagesArray.forEach(message => {
        if (message.content) {
            const fixedContent = fixEncoding(message.content);
            allMessages.push(fixedContent);
        }
    })
})

fs.writeFileSync("facebook-messages.txt", allMessages.join(" "))
