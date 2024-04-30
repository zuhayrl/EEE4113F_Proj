<!DOCTYPE html>
<html>
    
<head>
    <title>Sensor Readings and Video/File Downloads</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="style.css">

    <script>
        function updateSensorReadings() {
            fetch('sensor_data.php')
                .then(response => response.text())
                .then(data => {
                    document.getElementById('sensor-readings').innerHTML = data;
                });
        }

        function updateFileList() {
            fetch('file_list.php')
                .then(response => response.text())
                .then(data => {
                    document.getElementById('file-list').innerHTML = data;
                });
        }

        // Update sensor readings every 5 seconds
        setInterval(updateSensorReadings, 5000);

        // Update file list every 10 seconds
        setInterval(updateFileList, 10000);

    </script>

</head>

<body>

    <div class="nav">
        <a href="index.php" class="active">Home</a>
        <a href="about.php">About</a>
        <a href="contact.php">Contact</a>
        <a href="testing.php">Testing</a>
    </div>

    <div class="container">
        
        <h2>Sensor Readings</h2>
        <div id="sensor-readings">
            <?php
                // Read sensor data from a file or GPIO pins
                $temperature = 25.3;
                $humidity = 60.2;

                // Display the sensor readings
                //echo "Temperature: " . $temperature . " °C<br>";
                //echo "Humidity: " . $humidity . " %";
		        echo "<b>Please wait for values to load</b>";
                
            ?>
        </div>

        <h2>Video File</h2>
        <div>
            <?php
                // Set the directory path where the files are stored
                $files_dir = "videofiles/";

                // Check if the "testvid.mp4" file exists
                $video_file = $files_dir . "testvid.mp4";
                if (file_exists($video_file)) {
                    // Display the video player
                    //echo "<video controls><source src='" . $video_file . "' type='video/quicktime'>Your browser does not support the video element.</video><br>";
                    echo "<a href='" .$video_file . "' download> Download Video </a><br>";
                } else {
                    echo "The 'testvid.mp4' file was not found in the 'videofiles/' directory.";
                }
            ?>
        </div>

        <h2>Download Files</h2>
        <div id="file-list">
            <?php
                // Set the directory path where the files are stored
                $files_dir = "files/";

                // Get a list of all files in the directory"
                $files = scandir($files_dir);
                foreach ($files as $file) {
                    if ($file === "." || $file === "..") {
                        continue;
                    }

                    // Get the creation date of the file
                    $creation_date = date("Y-m-d", filectime($files_dir . $file));

                    // Create the download link
                    echo "<a href='" . $files_dir . $file . "' download>" . $file . "</a>:  " . $creation_date . "<br>";
                }
            ?>
        </div>

        <p><button id="clearTempFiles">Clear Temp Files</button></p>

        <div id="confirmModal" class="modal">
            <div class="modal-content">
                <h2>Confirm File Deletion</h2>
                <p>Are you sure you want to delete the files in the tempfiles folder?</p>
                <div style="text-align: right;">
                    <button id="confirmDelete">Delete</button>
                    <button id="cancelDelete">Cancel</button>
                </div>
            </div>
        </div>

        <script>
            document.getElementById('clearTempFiles').addEventListener('click', function() {
                document.getElementById('confirmModal').style.display = 'block';
            });

            document.getElementById('confirmDelete').addEventListener('click', function() {
                document.getElementById('confirmModal').style.display = 'none';
                fetch('delete_temp_files.php', {
                    method: 'POST'
                })
                .then(response => response.text())
                .then(data => {
                    console.log(data);
                    // You can add additional code here to update the UI or display a success message
                })
                .catch(error => {
                    console.error('Error:', error);
                    // You can add additional code here to handle errors
                });
            });

            document.getElementById('cancelDelete').addEventListener('click', function() {
                document.getElementById('confirmModal').style.display = 'none';
            });
        </script>

    </div>

    

</body>
</html>
