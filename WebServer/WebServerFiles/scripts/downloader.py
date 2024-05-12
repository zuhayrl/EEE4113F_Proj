import requests
import os

esp32_ip = "192.168.4.1"

# Get file list
response = requests.get("http://" + esp32_ip + "/files")
file_list = response.text.splitlines()

# Define folder names for different file types
text_folder = 'textfiles'
video_folder = 'videofiles'
default_folder = 'bin'

# Download files based on type
for file_name in file_list:
    response = requests.get("http://" + esp32_ip + "//" + file_name)
    
    # Determine file type based on extension
    file_extension = file_name.split('.')[-1].lower()
    
    if file_extension == 'txt':
        folder_name = text_folder
    elif file_extension == 'mp4':
        folder_name = video_folder
    else:
        folder_name = default_folder
    
    file_path = os.path.join(folder_name, file_name)
    
    # Create folder if it doesn't exist
    #if not os.path.exists(folder_name):
    #    os.makedirs(folder_name)
    
    with open(file_path, "wb") as f:
        f.write(response.content)
