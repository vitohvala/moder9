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

def paste_word(word, pasteInsteadOfTyping, size):

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
    paste_instead_of_typing = sys.argv[2]
    number_of_letters_to_delete = int(sys.argv[3]) if len(sys.argv) > 3 else len(word)
    paste_word(word, paste_instead_of_typing, number_of_letters_to_delete)
