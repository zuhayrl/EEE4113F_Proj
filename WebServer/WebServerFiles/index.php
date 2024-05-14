<?php
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Execute the Python script
    //exec('python3 \scripts\text.py');
    shell_exec('python3 scripts/splitdata.py');

}
?>

<!DOCTYPE html>
<html>

    <head>
        <title>About</title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <link rel="stylesheet" href="style.css">
    </head>

    <body>
        <div class="nav">
            <a href="index.php" class="active">Controls</a>
            <a href="media.php">Media</a>
            <a href="statistics.php">Statistics</a>
        </div>
        <div class="container">
            <h1>System Controls</h1>
            <form method="POST">
                <input type="submit" value="Run Data Processing">
            </form>
        </div>
    </body>

    <div class="container">
        <h1>SD Card Stroage</h1> 
        <?php
            // Execute shell command to get disk usage information
            $disk_usage = shell_exec("df -h /");
            
            // Split the output into lines and get the second line (which contains the usage info)
            $lines = explode("\n", $disk_usage);
            $usage_info = explode(" ", preg_replace('/\s+/', ' ', $lines[1]));
            
            // Extract relevant information (total, used, and available space)
            $total_space = $usage_info[1];
            $used_space = $usage_info[2];
            $available_space = $usage_info[3];
            
            // Output the information on the webpage
            echo "Total Space: $total_space <br>";
            echo "Used Space: $used_space <br>";
            echo "Available Space: $available_space <br>";
        ?>
    </div>


</html>