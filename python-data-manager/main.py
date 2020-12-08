import asyncio
from datetime import datetime

from nats.aio.client import Client as NATS
from nats.aio.errors import ErrConnectionClosed, ErrTimeout, ErrNoServers

from datamanager.datamanager import *

async def run(loop):

    dm = DataManager(['localhost'])

    nc = NATS()

    async def disconnected_cb():
        print("Got disconnected...")

    async def reconnected_cb():
        print("Got reconnected...")

    await nc.connect("nats://ruser:T0pS3cr3t@127.0.0.1:4222",
                     reconnected_cb=reconnected_cb,
                     disconnected_cb=disconnected_cb,
                     max_reconnect_attempts=-1,
                     loop=loop)

    async def message_handler(msg):
        subject = msg.subject
        reply = msg.reply
        models = dm.deserialize(msg.data.decode())['models']
        
        for m in models:
            # ['timestamp', 'link', 'text', 'filename', 'ip', '
            print('{} - {} links'.format(m['link'], len(m['links'])))
            dm.insert(m)

            d = dm.get_models_to_download(m)
            print('sending', d)
            await nc.publish('downloader', d)

    await nc.subscribe("data-manager", "qdata-manager", cb=message_handler)

if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(run(loop))
    loop.run_forever()
    loop.close()