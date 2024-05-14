import os

def split_data(input_file, output_dir):
    data_types = ["trigger", "temperature", "humidity", "light_level"]
    data_files = {data_type: open(os.path.join(output_dir, f"{data_type}.txt"), "a") for data_type in data_types}
    
    with open(input_file, "r") as file:
        for line in file:
            parts = line.strip().split(", ")
            date_time = parts[0]
            values = map(int, parts[1:])
            for data_type, value in zip(data_types, values):
                data_files[data_type].write(f"{date_time}, {value}\n")
    
    for file in data_files.values():
        file.close()

input_file = "textfiles/esp_data.txt"
output_directory = "stats"
split_data(input_file, output_directory)
