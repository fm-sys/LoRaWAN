import requests
import serial
import json
import time
from datetime import date
import serial.tools.list_ports as list_ports

# to keep the API key secure, store it in a seperated file which is not contained in the git repository
import apikeys


def uploadData(value):
    global last_value

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
    data = payload["data"]
    timestamp = int(time.time())


    ####################################
    # extract data out of input string
    ####################################
    #
    # Data format:
    # 'd1|24.06|433'
    #
    ####################################
    
    device_name = value.split("|")[0]
    value_temp = float(value.split("|")[1])
    value_co2 = float(value.split("|")[2])

    ####################################
    # add data to payload
    ####################################

    # add Temperatur
    addData(data, device_name + "_Temperatur", value_temp, timestamp)

    # add CO2 (if available)
    if value_co2 >= 0:
        addData(data, device_name + "_CO2", value_co2, timestamp)

    # post the request
    requests.post("http://iotplotter.com/api/v2/feed/829369784150976580", headers=apikeys.headers, data=json.dumps(payload))

def addData(payload_data, name, value, time):
    payload_data[name] = []
    data_object = payload_data[name]

    # set current value
    data_object.append({"value":value})

    # add data to the graph
    data_object.append({"value":value, "epoch":time})

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
            line_value = ser.readline().decode()
            print (line_value)
            if "|" in line_value: # seems to be data in our protocol format
                uploadData(line_value)
                
    
        except Exception as e:
            print("Error: ", end="")
            print(e)
            time.sleep(1)
            
except KeyboardInterrupt:
    print('interrupted! Closing...')
    ser.close()
    time.sleep(1)



