# Merge Text Files
import os
from datetime import datetime

# Path to the folder containing your files
folder_path = 'C:\\Users\\zuhay\\OneDrive\\University\\ForthYear\\Sem1\\EEE4113F\\Github\\EEE4113F_Proj\\WebServer\\PythonScripts\\TestData'

# Output file where all text will be combined
output_file_path = 'C:\\Users\\zuhay\\OneDrive\\University\\ForthYear\\Sem1\\EEE4113F\\Github\\EEE4113F_Proj\\WebServer\\PythonScripts\\TestData\\output_file.txt'

# Get a list of files in the folder
files = os.listdir(folder_path)

# Create a list to store file paths and their corresponding modification dates
file_info = []

# Iterate through each file in the folder
for file_name in files:
    # Get the full path of the file
    file_path = os.path.join(folder_path, file_name)
    
    # Get the modification timestamp of the file
    modification_time = os.path.getmtime(file_path)
    
    # Convert the timestamp to a datetime object
    modification_date = datetime.fromtimestamp(modification_time)
    
    # Append the file path and modification date to the list
    file_info.append((file_path, modification_date))

# Sort the list of files based on modification date
file_info.sort(key=lambda x: x[1])

# Now you can iterate through the sorted list and process the files in order of date
#for file_path, modification_date in file_info:
#    # Process the file as needed
#    print(f"File: {file_path}, Modification Date: {modification_date}")

# Create a new file to store all text content
with open(output_file_path, 'w') as output_file:
    # Iterate through the sorted list of files
    for file_path, _ in file_info:
        # Check if the file is a text file
        if file_path.lower().endswith('.txt'):
            # Read the content of the text file and write it to the output file
            with open(file_path, 'r') as input_file:
                #output_file.write(f"Content from: {file_path}\n")
                output_file.write(input_file.read())
                output_file.write('\n')

print(f"All text files have been concatenated")