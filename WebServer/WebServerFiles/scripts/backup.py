import requests
import os

esp32_ip = "192.168.1.211"

#get file list
response = requests.get("http://"+esp32_ip+"/files")
file_list = response.text.splitlines()

folder_name = 'scripts'
#download files
for file_name in file_list:
    response = requests.get("http://"+esp32_ip+"//"+file_name)
    file_path = os.path.join(folder_name, file_name)
    with open(file_path, "wb") as f:
        f.write(response.content)
