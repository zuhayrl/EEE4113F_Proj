# Path to the text file containing the data formatted with commas
file_path = 'C:\\Users\\zuhay\\OneDrive\\University\\ForthYear\\Sem1\\EEE4113F\\Github\\EEE4113F_Proj\\WebServer\\PythonScripts\\TestData\\output_file.txt'

#output path
folder_path = 'C:\\Users\\zuhay\\OneDrive\\University\\ForthYear\\Sem1\\EEE4113F\\Github\\EEE4113F_Proj\\WebServer\\PythonScripts\\TestData\\'

# File paths for each data type
file_paths = ['time.txt', 'trigger.txt', 'temp.txt', 'humidity.txt', 'light.txt', 'other.txt']

# Read the content of the text file
with open(file_path, 'r') as file:
    # Read each line of the file
    for line in file:
        # Remove leading/trailing whitespaces and split the line by commas
        data_items = [item.strip() for item in line.split(',')]

        # Write data items to corresponding files
        for index, data_item in enumerate(data_items):
            # Get the corresponding file path
            output_file_path = folder_path+file_paths[index]

            # Write data item to the corresponding file
            with open(output_file_path, 'a') as output_file:
                output_file.write(data_item + '\n')

#print("Data has been written to individual text files.")
