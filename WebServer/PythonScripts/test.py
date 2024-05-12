import os

def split_data(input_file, output_dir):
    data_types = ["number", "temperature", "humidity", "light_level"]
    data_files = {data_type: open(os.path.join(output_dir, f"{data_type}_data.txt"), "w") for data_type in data_types}
    
    with open(input_file, "r") as file:
        for line in file:
            parts = line.strip().split(", ")
            date_time = parts[0]
            values = map(int, parts[1:])
            for data_type, value in zip(data_types, values):
                data_files[data_type].write(f"{date_time}, {value}\n")
    
    for file in data_files.values():
        file.close()

input_file = "C:\\Users\\zuhay\\OneDrive\\University\\ForthYear\\Sem1\\EEE4113F\\Github\\EEE4113F_Proj\\WebServer\\PythonScripts\\TestData\\output_file.txt"  # Replace with your input file name
output_directory = "C:\\Users\\zuhay\\OneDrive\\University\\ForthYear\\Sem1\\EEE4113F\\Github\\EEE4113F_Proj\\WebServer\\PythonScripts\\TestData"  # Replace with your desired output directory
split_data(input_file, output_directory)
