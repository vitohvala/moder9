import sys
import pyperclip
from pynput.keyboard import Key, Controller

keyboard = Controller()

def pressButton(button, times):
    for i in range(times):
        keyboard.press(button)
        keyboard.release(button)

def paste():
    keyboard.press(Key.ctrl)
    keyboard.press('v')
    keyboard.release('v')
    keyboard.release(Key.ctrl)

def paste_word(word, pasteInsteadOfTyping):

    size = len(word)

    pressButton(Key.backspace, size)

    if pasteInsteadOfTyping:
        pyperclip.copy(word)
        paste()
    else:
        for letter in word:
            pressButton(letter, 1)

    print("pasted")

if __name__ == "__main__":
    word = sys.argv[1]
    pasteInsteadOfTyping = sys.argv[2]
    paste_word(word, pasteInsteadOfTyping)
