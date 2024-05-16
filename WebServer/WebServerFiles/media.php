<!DOCTYPE html>
<html>
<head>
    <title>Media Page</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div class="nav">
        <a href="index.php">Controls</a>
        <a href="media.php" class="active">Media</a>
        <a href="statistics.php">Statistics</a>
    </div>

    <div class = "container">
        <!--<div class="image-gallery">-->
            <?php
            $folder = 'videofiles/'; // Specify the folder path relative to the current file
            $files = array_diff(scandir($folder), array('.', '..'));

            foreach ($files as $file) {
                $filePath = $folder . $file;
                echo "<div class='image-container'>";
                echo "<img src='$filePath' alt='$file'>";
                echo "<br>";
                echo "<a href='$filePath' download='$file' class='download-btn'>Download</a> <br> <br>";
                echo "</div>";
            }
            ?>
        <!--</div>-->
    </div>
    
</body>
</html>