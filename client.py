import sys
import time
import requests
import hashlib
import hmac
uplink = 'http://10.0.1.5/lockbot'
#uplink = 'https://kelder.zeus.ugent.be/lockbot'

command = sys.argv[1]
if command == 'close':
    command = 'lock'

with open('tokens.h') as credfile:
    down_key = [line.split('"')[1] for line in credfile if 'DOWN_COMMAND_KEY' in line][0]

t = int(time.time()) * 1000
payload = f'{t};{command}'

hash = hmac.new(down_key.encode('utf8'), payload.encode('utf8'), hashlib.sha256).hexdigest().upper()
print(hash)
r = requests.post(uplink, payload, headers={'HMAC': hash})
print(r.text)
