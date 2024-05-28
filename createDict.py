import regex as re
import json

def get_words_from_file(file_path):
    with open(file_path, "r", encoding="utf-8") as file:
        text = file.read()

    pattern = r'\b\p{L}+\b'
    matches = re.findall(pattern, text)

    return matches


includeFullDictionary = False
includeEnteredWords = True
includePastedWords = True

def process_files():
    files = []
    words = []

    if includeFullDictionary:
        files.append("dict.txt")
    if includeEnteredWords:
        files.append("pasted-words.txt")
    if includeEnteredWords:
        files.append("entered-words.txt")

    for file_path in files:
        file_words = get_words_from_file(file_path)
        words.extend(file_words)

    return words

words_collected = process_files()

words_dict = {"words": words_collected}

# Define the filename
file_name = 'dict.json'


with open(file_name, 'w', encoding='utf-8') as file:
    json.dump(words_dict, file, ensure_ascii=False, indent=4)

print("ready")
