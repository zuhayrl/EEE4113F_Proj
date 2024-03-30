/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-web-server-multiple-pages
 */

const char *HTML_CONTENT_HOME = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>Home Page</title>
</head>
<body>
    <h1>Welcome to the Home Page</h1>
    <ul>
        <li><a href="/led.html">LED Page</a></li>
        <li><a href="/temperature.html">Temperature Page</a></li>
    </ul>
</body>
</html>
)=====";
