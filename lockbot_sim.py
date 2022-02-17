import sys
import time
import requests
import hashlib
import hmac
uplink = 'http://localhost:3000'

command = "change"
reason = "state"
value = 0

#  closed    = 0
#  open      = 1

with open('tokens.h') as credfile:
    up_key = [line.split('"')[1] for line in credfile if 'UP_COMMAND_KEY' in line][0]


message = 'cmd={command}&reason={reason}&value={value}'
hash = hmac.new(up_key.encode('utf8'), message.encode('utf8'), hashlib.sha256).hexdigest().upper()
print(hash)
r = requests.post(uplink, message , headers={'HMAC': hash})
print(r.text)
