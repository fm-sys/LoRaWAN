import requests
import serial
import json
import time
from datetime import date
import serial.tools.list_ports as list_ports

# to keep the API key secure, store it in a seperated file which is not contained in the git repository
import apikeys



def addData(value):
    # make sure, value is a float (e.g. 123.45)
    value = float(value)


    # create json payload
    
    ####################################
    # payload will look like this:
    ####################################
    #
    # {
    #   "data": {
    #     "GRAPH_NAME": [
    #       {
    #         "value": 25.05
    #       },
    #       {
    #         "value": 25.99,
    #         "epoch": 1516195980
    #       }
    #     ]
    #   }
    # }
    #
    ####################################
    
    payload = {}
    payload["data"] = {}
    payload["data"]["Temperatur"] = []
    temp_data = payload["data"]["Temperatur"]

    # set current value
    temp_data.append({"value":value})

    # add data to the graph
    temp_data.append({"value":value, "epoch":int(time.time())})

    # post the request
    requests.post("http://iotplotter.com/api/v2/feed/829369784150976580", headers=apikeys.headers, data=json.dumps(payload))


import sys, signal
def signal_handler(signal, frame):
    print('closing...')
    ser.close()
    time.sleep(1)
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)


print("search for Arduino...")
device = list_ports.grep("Arduino").send(None)
print("found: " + device.description)

ser = serial.Serial(device.device, 9600)

print("Serial communication is ready!")
try:
    while True:
        try:
            line_value = ser.readline()
            print (line_value)
            addData(line_value)
                
    
        except:
            time.sleep(5)
            
except KeyboardInterrupt:
    print('interrupted! Closing...')
    ser.close()
    time.sleep(1)



