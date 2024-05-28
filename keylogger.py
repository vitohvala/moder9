# keylogger.py
from pynput.keyboard import Key, Listener
import json
import os

def on_press(key):
    keyToSend = key.char if hasattr(key, 'char') else key
    print(f'["press", "{keyToSend}"]', flush=True)

def on_release(key):
    keyToSend = key.char if hasattr(key, 'char') else key
    print(f'["release", "{keyToSend}"]', flush=True)

with Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
