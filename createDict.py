import regex as re
import json
import os

def str_to_bool(s):
    if isinstance(s, bool):
        return s
    return s.lower() in ["true", "1", "t", "y", "yes"]


def get_words_from_file(file_path):
    with open(file_path, "r", encoding="utf-8") as file:
        text = file.read()

    pattern = r'\b\p{L}+\b'
    matches = re.findall(pattern, text)

    return matches


INCLUDE_FULL_DICTIONARY = str_to_bool(os.getenv('INCLUDE_FULL_DICTIONARY', True))
INCLUDE_ENTERED_WORDS = str_to_bool(os.getenv('INCLUDE_ENTERED_WORDS', True))
INCLUDE_PASTED_WORDS = str_to_bool(os.getenv('INCLUDE_PASTED_WORDS', True))
INCLUDE_FACEBOOK_DICT = str_to_bool(os.getenv('INCLUDE_FACEBOOK_DIST ', True))


ENTERED_WORDS_FILE_NAME = os.getenv("ENTERED_WORDS_FILE_NAME", "entered-words.txt")
PASTED_WORDS_FILE_NAME = os.getenv("PASTED_WORDS_FILE_NAME", "pasted-words.txt")
DICT_FILE_NAME = os.getenv("DICT_FILE_NAME", "dict.txt")
FACEBOOK_DICT_FILE_NAME = os.getenv("FACEBOOK_DICT_NAME", "facebook-messages.txt")

def process_files():
    files = []
    words = []

    if INCLUDE_FULL_DICTIONARY:
        files.append(DICT_FILE_NAME)
    if INCLUDE_ENTERED_WORDS:
        files.append(ENTERED_WORDS_FILE_NAME)
    if INCLUDE_PASTED_WORDS:
        files.append(PASTED_WORDS_FILE_NAME)

    if INCLUDE_FACEBOOK_DICT:
        files.append(FACEBOOK_DICT_FILE_NAME)

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
