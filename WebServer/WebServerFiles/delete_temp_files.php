<?php
$folderPath = 'stats/';

// Delete all files in the folder
$files = glob($folderPath . '*');
foreach ($files as $file) {
    if (is_file($file)) {
        unlink($file);
    }
}

echo "Temp files have been deleted.";