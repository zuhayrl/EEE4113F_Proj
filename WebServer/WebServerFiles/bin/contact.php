<?php
// Read the contents of the text file into an array
$lines = file('stats/time_trigger.txt', FILE_IGNORE_NEW_LINES);

// Initialize counters for each option
$option1Count = 0;
$option2Count = 0;
$bothOptionsCount = 0;

// Loop through each line
foreach ($lines as $line) {
    // Split the line into time and number
    list($time, $number) = explode(', ', $line);

    // Check if the time falls within the desired range
    if (strtotime($time) >= strtotime('18:00:00') && strtotime($time) <= strtotime('06:00:00') + 86400) {
        // Increment the appropriate counter
        if ($number === '0') {
            $option1Count++;
        } elseif ($number === '1') {
            $option2Count++;
        } elseif ($number === '2') {
            $bothOptionsCount++;
        }
    }
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>Night Options</title>
    <!--<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>-->
    <script src="javascript/chart.js"></script>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div class="nav">
        <a href="index.php">Home</a>
        <a href="picontrols.php">Controls</a>
        <a href="contact.php"  class="active">Contact</a>
        <a href="statistics.php">Statistics</a>
    </div>

    <div class="container">
        <h1> Trigger Frequency at Night</h1>
        <canvas id="myChart"></canvas>
        <script>
            var ctx = document.getElementById('myChart').getContext('2d');
            var myChart = new Chart(ctx, {
                type: 'bar',
                data: {
                    labels: ['PiR', 'Radar', 'Both'],
                    datasets: [{
                        label: 'Night Options',
                        data: [<?php echo $option1Count; ?>, <?php echo $option2Count; ?>, <?php echo $bothOptionsCount; ?>],
                        backgroundColor: [
                            'rgba(255, 99, 132, 0.2)',
                            'rgba(54, 162, 235, 0.2)',
                            'rgba(255, 206, 86, 0.2)'
                        ],
                        borderColor: [
                            'rgba(255, 99, 132, 1)',
                            'rgba(54, 162, 235, 1)',
                            'rgba(255, 206, 86, 1)'
                        ],
                        borderWidth: 1
                    }]
                },
                options: {
                    scales: {
                        y: {
                            beginAtZero: true
                        }
                    }
                }
            });
        </script>
    </div>


</body>
</html>