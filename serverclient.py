import sys
import time
import requests
import hashlib
import hmac

cmd, why, val = "open", "turnassist", "0"

up_key = 'UPKEY'

payload = f'cmd={cmd}&why={why}&val={val}'

hash = hmac.new(up_key.encode('utf8'), payload.encode('utf8'), hashlib.sha256).hexdigest().upper()
print(hash)
r = requests.post('http://mattermore.zeus.gent/doorkeeper', payload, headers={'HMAC': hash})

print(r.text)
