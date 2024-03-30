/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-web-server-multiple-pages
 */

const char *HTML_CONTENT_LED = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>LED Page</title>
</head>
<body>
    <h1>LED Page</h1>
    <p>LED State: <span style="color: red;">%STATE%</span></p>
    <a href='/led.html?state=on'>Turn ON</a>
    <br><br>
    <a href='/led.html?state=off'>Turn OFF</a>
    <p><a href="/index.html">Home Page</a></p>
</body>
</html>
)=====";
