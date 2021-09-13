import sys
import time
import requests
import hashlib
import hmac


command = sys.argv[1]
if command == 'close':
    command = 'lock'

t = int(time.time())
down_key = 'DOWNKEY'

payload = f'{t};{command}'

hash = hmac.new(down_key.encode('utf8'), payload.encode('utf8'), hashlib.sha256).hexdigest().upper()
print(hash)
r = requests.post('https://kelder.zeus.ugent.be/lockbot', payload, headers={'HMAC': hash})

print(r.text)
