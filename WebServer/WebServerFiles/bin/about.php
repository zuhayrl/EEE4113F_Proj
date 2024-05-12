<?php
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Execute the Python script
    //exec('python3 \scripts\text.py');
    shell_exec('python3 scripts/downloader.py');

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
            <a href="index.php">Home</a>
            <a href="about.php" class="active">About</a>
            <a href="contact.php">Contact</a>
            <a href="testing.php">Testing</a>
        </div>
        <div class="container">
            <h1>Run Python Script</h1>
            <form method="POST">
                <input type="submit" value="Try to Run Python Script">
            </form>
        </div>
    </body>

</html>