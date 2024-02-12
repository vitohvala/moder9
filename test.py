import os
from datetime import datetime
import pyxhook
import pyperclip
from pynput.keyboard import Key, Controller
import re
from collections import Counter
from itertools import islice

keyboard = Controller()

phoneLayout = True
pasteInstadOfTyping = False

history = [""] * 100
ctrlPressed = False

one = ""
two = ""
three = ""
four = ""
five = ""

#možda da na 1 budu naša slova, verovatno će biti preciznije predviđanje
letters = {
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

with open("pasted-words.txt", 'r') as file:
    text = file.read()

with open("entered-words.txt", 'r') as file:
    text += file.read()

with open("dict.txt", 'r') as file:
    text += file.read()

words = re.findall(r'\b[^\d\W]+\b', text)
words = [word.lower() for word in words]

print("Modernine pokrenut:")
print("Obrnut redosled tastature: ", "Isključen" if phoneLayout else "Uključen")
print("Pronađeno:", len(words), " reči!")

word_counts = Counter(words)
words = sorted(word_counts, key=word_counts.get, reverse=True)
local_words = [:]

print("Od toga unikatnih:", len(words))
print("Srećno!")

def onKeyUp(event):
    if(event.Key == "Control_L" or event.Key == "Control_R"):
        ctrlPressed = False

def OnKeyPress(event):
    global one, two, three, four, five, history, ctrlPressed
    key = event.Key

    # Check for ctrl + backspace shortcut:
    if(key == "Control_L" or key == "Control_R"):
        ctrlPressed = True

    if key == "BackSpace":
        if(ctrlPressed):
            return restart()

        else:
            history.pop()
            checkLetter("".join(history))
            return


    if(key == "space"):
        return restart()

    if key == "P_Home" and phoneLayout or key == "P_End" and not phoneLayout:
        return paste_word(one)

    if key == "P_Divide":
        return paste_word(two)

    if key == "P_Multiply":
        return paste_word(three)

    if key == "P_Subtract":
        return paste_word(four)

    if key == "P_Add":
        return paste_word(five)

    if  key == "P_End":
        key = 1 if not phoneLayout else 7

    if key == "P_Down":
        key = 2 if not phoneLayout else 8

    if key == "P_Next":
        key = 3 if not phoneLayout else 9

    if key == "P_Left":
        key = 4

    if key == "P_Begin":
        key = 5

    if key == "P_Right":
        key = 6

    if key == "P_Home":
        key = 7 if not phoneLayout else 1

    if key == "P_Up":
        key = 8 if not phoneLayout else 2

    if key == "P_Page_Up":
        key = 9 if not phoneLayout else 3

    key = str(key);

    if key in "123456789":

        for i in range(0, len(history) - 1):
            history[i] = history[i+1]

        history[-1] = key

        checkLetter("".join(history))

new_hook = pyxhook.HookManager()
new_hook.KeyDown = OnKeyPress
new_hook.keyUp = onKeyUp
new_hook.HookKeyboard()
new_hook.start()

def paste():
    keyboard.press(Key.ctrl)
    keyboard.press('v')
    keyboard.release('v')
    keyboard.release(Key.ctrl)

def checkLetter(text):

    global one, two, three, four, five, local_words;

    local_words = [word for word in local_words if len(word) >= len(text)]

    for index, number in enumerate(text):

        allowed_letters = letters[number]
        local_words = [word for word in local_words if any(word[index] == letter for letter in allowed_letters)]

    local_words = [x for x in local_words if len(x) == len(text)] +  [x for x in local_words if len(x) != len(text)]

    clear_console()

    one = local_words[0] + " " if len(local_words) else " "
    two = local_words[1] + " " if len(local_words) > 1 else " "
    three = local_words[2] + " " if len(local_words) > 2 else " "
    three = local_words[3] + " " if len(local_words) > 3 else " "
    three = local_words[4] + " " if len(local_words) > 4 else " "

    print(one, two, three, four, five)

def pressButton(button, times):
    for i in range(times):
        keyboard.press(button)
        keyboard.release(button)

def restart():

    global one, two, three, four, five, history

    one = ""
    two = ""
    three = ""
    four = ""
    five = ""
    history = [""] * 100
    local_words = words[:]

def paste_word(word):
    numbers = "".join(history);

    pressButton(Key.backspace, len(numbers) + 1)

    if pasteInstadOfTyping:
        pyperclip.copy(word)
        paste()
    else:
        for letter in word:
            pressButton(letter, 1)

    restart()


def clear_console():
    os.system('cls' if os.name == 'nt' else 'clear')
