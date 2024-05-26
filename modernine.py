import asyncio
import websockets
import threading
from pynput.keyboard import Key, Listener, Controller
import json
import pyperclip
import regex as re

keyboard = Controller()
connected_clients = set()

paste_in_progress = False

async def handle_client(websocket, path):
    connected_clients.add(websocket)
    try:
        async for message in websocket:
            print("Received message:", message)
            data = json.loads(message)

            if(data[0] == "paste"):
                paste_word(data[1], data[2], data[3])

            if(data[0] == "get_words_from_file"):

                words = get_words_from_file(data[1])

                message = ["got_words", words, data[1]]

                await send_message_to_clients(json.dumps(message))

    finally:
        connected_clients.remove(websocket)

async def send_message_to_clients(message):
    if connected_clients:  # Avoid errors if no clients are connected
        await asyncio.gather(*[client.send(message) for client in connected_clients])


def on_press(key):
    global paste_in_progress

    if not paste_in_progress:

        message = json.dumps(["press", str(key)])
        asyncio.run(send_message_to_clients(message))

def on_release(key):
    global paste_in_progress

    if not paste_in_progress:

        message = json.dumps(["release", str(key)])
        asyncio.run(send_message_to_clients(message))

def keyboard_listener_thread():
    with Listener(on_press=on_press, on_release=on_release) as listener:
        listener.join()

def paste():
    keyboard.press(Key.ctrl)
    keyboard.press('v')
    keyboard.release('v')
    keyboard.release(Key.ctrl)

def pressButton(button, times):
    for i in range(times):
        keyboard.press(button)
        keyboard.release(button)

def paste_word(word, pasteInstadOfTyping, size):

    global paste_in_progress

    paste_in_progress = True

    pressButton(Key.backspace, size)

    if pasteInstadOfTyping:
        pyperclip.copy(word)
        paste()
    else:
        for letter in word:
            pressButton(letter, 1)

    paste_in_progress = False


def get_words_from_file(file_path):
    with open(file_path, "r", encoding="utf-8") as file:
        text = file.read()

    pattern = r'\b\p{L}+\b'
    matches = re.findall(pattern, text)

    return matches

if __name__ == "__main__":
    server = websockets.serve(handle_client, 'localhost', 8765)
    asyncio.get_event_loop().run_until_complete(server)

    listener_thread = threading.Thread(target=keyboard_listener_thread)
    listener_thread.start()

    asyncio.get_event_loop().run_forever()
