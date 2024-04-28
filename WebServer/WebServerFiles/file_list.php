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
