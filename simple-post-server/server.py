from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import os
import paho.mqtt.client as mqtt
import struct
import influxdb_client, os, time
import requests
from datetime import date, datetime
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    

    def do_POST(self):
    
        token = ""
        org = ""
        url = ""
        bucket = ""

        write_client = influxdb_client.InfluxDBClient(url=url, token=token, org=org)
        write_api = write_client.write_api(write_options=SYNCHRONOUS)
        
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        print("Received POST request with payload:", post_data.decode('utf-8'))
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        response = {'status': 'success'}
        self.wfile.write(json.dumps(response).encode('utf-8'))
        
        # Decoce the data
        data = json.loads(post_data)
        float_list = data["object"]["tension"]["1"]

        # Step 1: Convert the list of floats to integers
        int_list = [int(x) for x in float_list]
        print("List of integers:", int_list)

        # Step 2: Convert the list of integers to a byte array
        byte_array = bytearray(int_list)
        print("Byte array:", byte_array)

        signed_int = struct.unpack('<i', byte_array)[0]

        #print(signed_int)

        point = Point("smartlash").field("value", signed_int)

        write_api.write(bucket=bucket, org="iotlnu", record=point)
        

def run(server_class=HTTPServer, handler_class=SimpleHTTPRequestHandler, port=8080):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print(f'Starting httpd server on port {port}')
    httpd.serve_forever()

if __name__ == "__main__":
    run()
