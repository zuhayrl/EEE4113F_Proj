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
        <a href="about.php">About</a>
        <a href="contact.php">Contact</a>
        <a href="testing.php" class="active">Testing</a>
    </div>
    <div class="container">
        <h1>Temperature Data</h1>
        <canvas id="temperatureChart"></canvas>
        <script>
            // Read temperature data from the file
            fetch('stats/temperature.txt')
                .then(response => response.text()) 
                .then(data => {
                    // Split the data into an array of lines
                    const lines = data.trim().split('\n');
                    // Extract the last 30 lines or all lines if less than 30
                    const lastLines = lines.slice(-30);

                    // Extract temperatures from the second column of the lines
                    const temperatures = lastLines.map(line => {
                        const columns = line.split(', ');
                        return parseFloat(columns[1]); // Assuming temperature is in the second column
                    });

                    // Create labels for the chart
                    const labels = Array.from({length: Math.min(30, lines.length)}, (_, i) => 'Day ' + (i + 1));

                    // Create the bar graph
                    const ctx = document.getElementById('temperatureChart').getContext('2d');
                    new Chart(ctx, {
                        type: 'bar',
                        data: {
                            labels: labels,
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
