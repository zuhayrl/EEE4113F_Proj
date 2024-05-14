<!DOCTYPE html>
<html>
<head>
    <title>Testing Page</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="style.css">
    <!--<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>-->
    <script src="javascript/chart.js"></script>


</head>
<body>
    <div class="nav">
        <a href="index.php">Home</a>
        <a href="picontrols.php">Pi Controls</a>
        <a href="contact.php">Contact</a>
        <a href="statistics.php" class="active">Statistics</a>
    </div>
    <div class="container">
        <h1>Temperature Data</h1>
        <canvas id="temperatureChart"></canvas>
        <script>
            // Read temperature data from the file
            fetch('stats/temperature.txt')
                .then(response => response.text()) 
                .then(data => {
                    // Split the data into an array of temperatures
                    const temperatures = data.trim().split('\n').map(Number);

                    // Create the bar graph
                    const ctx = document.getElementById('temperatureChart').getContext('2d');
                    new Chart(ctx, {
                        type: 'bar',
                        data: {
                            labels: ['Day 1', 'Day 2', 'Day 3', 'Day 4', 'Day 5', 'Day 6', 'Day 7'],
                            datasets: [{
                                label: 'Average Temperature',
                                data: temperatures,
                                backgroundColor: 'rgba(75, 192, 192, 0.2)',
                                borderColor: 'rgba(75, 192, 192, 1)',
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
                })
                .catch(error => {
                    console.error('Error reading temperature data:', error);
                    document.getElementById('temperatureChart').innerHTML = 'Error: Unable to load temperature data.';
                });
        </script>
    </div>
</body>
</html>