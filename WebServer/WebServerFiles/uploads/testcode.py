import requests
import os

esp32_ip = "192.168.1.213"

# Get the list of files from the ESP32
print("Fetching file list from ESP32")
response = requests.get("http://"+esp32_ip+"/files")
file_list = response.text.splitlines()

# Download all files from the ESP32
for file_name in file_list:
    print("Downloading "+file_name)
    response = requests.get("http://"+esp32_ip+"//"+file_name)
    with open(file_name, "wb") as f:
        f.write(response.content)
    print(file_name+" downloaded")
