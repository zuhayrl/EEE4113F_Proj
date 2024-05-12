#include "esp_camera.h"
#include "Arduino.h"
#include "esp_sleep.h"
#include "FS.h"      // SD Card ESP32
#include "SD_MMC.h"  // SD Card ESP32
// #include "soc/soc.h"           // Disable brownour problems
// #include "soc/rtc_cntl_reg.h"  // Disable brownour problems
// #include "driver/rtc_io.h"
#include <EEPROM.h>  // read and write from flash memory
#include "SoftwareSerial.h"
#include "HardwareSerial.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// Other pins
#define LED_PIN 33
#define IR_LAMP_PIN (gpio_num_t)16
#define RF_LINK_PIN (gpio_num_t)1

//Coms
const char *ssid = "TNCAPC197C1";
const char *password = "953E33B8A6";
AsyncWebServer server(80);

unsigned long lastWakeUpTime = 0;

// File handlers
uint32_t picture_number = 0;
String txt_path = "/data.txt";
String txt_new_line = "";
File txt_file;

//Parallel task handlers
TaskHandle_t Camera_capture;
TaskHandle_t SD_card_write;

camera_fb_t *fb_A;
camera_fb_t *fb_B;

bool fb_A_data_ready = false;
bool fb_B_data_ready = false;
bool capture = true;

void setup() {
  Serial.begin(115200);

  // create txt file
  fs::FS &fs = SD_MMC;
  txt_file = fs.open(txt_path.c_str(), FILE_WRITE);
  txt_file.close();

  //wakes
  pinMode(RF_LINK_PIN, INPUT);
  gpio_wakeup_enable(RF_LINK_PIN, GPIO_INTR_HIGH_LEVEL);
  esp_sleep_enable_gpio_wakeup();  //wake from user
  // uart_set_wakeup_threshold(0, 1);
  esp_sleep_enable_uart_wakeup(0);  //wake from sensor

  //IR lamp
  pinMode(IR_LAMP_PIN, OUTPUT);
  digitalWrite(IR_LAMP_PIN, LOW);  //turn off
}

void init_cam() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA;  // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 63;
    config.fb_count = 2;
  } else {
    Serial.println("PSRAM was not Found");
    while (true) {}  //error handler
    // config.frame_size = FRAMESIZE_SVGA;
    // config.jpeg_quality = 12;
    // config.fb_count = 1;
  }

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    while (true) {}
  }
  Serial.println("Camera Started");
}

void init_SD_card() {
  // Serial.println("Starting SD Card");
  if (!SD_MMC.begin()) {
    Serial.println("SD Card Mount Failed");
    while (true) {}
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached");
    while (true) {}
  }
  Serial.println("SD Card Started");
}

void capture_image(void *pvParameters) {
  init_cam();
  while (capture) {
    if (!fb_A_data_ready) {
      fb_A = esp_camera_fb_get();
      fb_A_data_ready = true;
    } else if (!fb_B_data_ready) {
      fb_B = esp_camera_fb_get();
      fb_B_data_ready = true;
    }
  }
  esp_camera_deinit();
  vTaskDelete(NULL);
}

void store_image(void *pvParameters) {
  init_SD_card();
  while (capture) {
    if (fb_A_data_ready) {
      String path = "/" + String(picture_number) + ".jpg";

      fs::FS &fs = SD_MMC;
      // Serial.printf("Picture file name: %s\n", path.c_str());

      File file = fs.open(path.c_str(), FILE_WRITE);
      // Serial.println("Opened");
      file.write(fb_A->buf, fb_A->len);
      // Serial.println("Written");
      file.close();
      // Serial.println("Closed");

      esp_camera_fb_return(fb_A);
      // Serial.println("Ret.");

      fb_A_data_ready = false;
      picture_number++;
    } else if (fb_B_data_ready) {
      String path = "/" + String(picture_number) + ".jpg";

      fs::FS &fs = SD_MMC;
      // Serial.printf("Picture file name: %s\n", path.c_str());

      File file = fs.open(path.c_str(), FILE_WRITE);
      file.write(fb_B->buf, fb_B->len);
      file.close();

      esp_camera_fb_return(fb_B);
      fb_B_data_ready = false;
      picture_number++;
    }
  }
  SD_MMC.end();
  vTaskDelete(NULL);
}

void start_video(uint32_t time_s) {

  uint32_t picture_number_start = picture_number;
  capture = true;

  xTaskCreatePinnedToCore(
    capture_image,   /* Task function. */
    "Capure Image",  /* name of task. */
    4096,            /* Stack size of task */
    NULL,            /* parameter of the task */
    1,               /* priority of the task */
    &Camera_capture, /* Task handle to keep track of created task */
    1);              /* pin task to core 0 */

  xTaskCreatePinnedToCore(
    store_image,    /* Task function. */
    "Store Image",  /* name of task. */
    4096,           /* Stack size of task */
    NULL,           /* parameter of the task */
    1,              /* priority of the task */
    &SD_card_write, /* Task handle to keep track of created task */
    0);             /* pin task to core 0 */

  delay(time_s * 1000);
  capture = false;
  while ((eTaskGetState(SD_card_write) == eRunning) || (eTaskGetState(Camera_capture) == eRunning)) {}  //wait for tasks to finish.
  Serial.println("Writen from " + String(picture_number_start) + " to " + String(picture_number));
}

uint8_t fetch_data(void) {
  uint8_t buf[15] = { 0 };
  Serial.readBytes(buf, 15);
  if (buf[10] == 0) return 2;

  uint8_t date, hours, minutes, seconds;
  float temperature, humidity;
  uint16_t lux;
  uint8_t triggers;

  date = buf[14];
  hours = buf[13];
  minutes = buf[12];
  seconds = buf[11];

  triggers = buf[10];

  u_int8_t new_temp_buf[4];
  new_temp_buf[0] = buf[9];
  new_temp_buf[1] = buf[8];
  new_temp_buf[2] = buf[7];
  new_temp_buf[3] = buf[6];
  temperature = (float)*((float *)new_temp_buf);

  u_int8_t new_hum_buf[4];
  new_hum_buf[0] = buf[5];
  new_hum_buf[1] = buf[4];
  new_hum_buf[2] = buf[3];
  new_hum_buf[3] = buf[2];
  humidity = (float)*((float *)new_hum_buf);

  lux = (buf[1] << 8) | buf[0];

  //Days, Hours:Minutes:Seconds, Trigger, Temp, Hum, Lux, PicStart, PicEnd
  txt_new_line = String(date) + "," + String(hours) + ":" + String(minutes) + ":" + String(seconds) + "," + String(triggers) + "," + String(temperature) + "," + String(humidity) + "," + String(lux);
  return (lux > 100);
}

void update_txt(uint32_t picture_number_start) {
  String new_line = txt_new_line + "," + String(picture_number_start) + "," + String(picture_number);
  fs::FS &fs = SD_MMC;
  txt_file = fs.open(txt_path.c_str(), FILE_APPEND);
  txt_file.println(new_line);
  txt_file.close();
}

void host_server(void) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/files", HTTP_GET, handleFileList);
  server.begin();
  Serial.println("HTTP server started");
}

void handleFileList(AsyncWebServerRequest *request) {
  if (!SD_MMC.begin()) {
    Serial.println("Error initializing SD card");
    request->send(500, "text/plain", "Error initializing SD card");
    return;
  }

  String fileList = "";
  File root = SD_MMC.open("/");
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      fileList += file.name();
      fileList += "\n";
    }
    file = root.openNextFile();
  }

  request->send(200, "text/plain", fileList);
}

void clean_files(void) {
  init_SD_card();
  SD_MMC.remove(txt_path);
  String path;
  for (uint32_t i = 0; i < picture_number; i++) {
    path = "/" + String(picture_number) + ".jpg";
    SD_MMC.remove(path);
  }
  picture_number = 0;
  SD_MMC.end();
}

void loop() {
  // sleep
  delay(1000);
  esp_light_sleep_start();
  esp_sleep_wakeup_cause_t wake_cause = esp_sleep_get_wakeup_cause();
  Serial.println("Waking with cause " + String(wake_cause));

  // wake_cause = (esp_sleep_wakeup_cause_t) 0;
  if (wake_cause == ESP_SLEEP_WAKEUP_UART) {
    //uart wake
    uint8_t ret = fetch_data();
    if (ret != 2) {
      if (ret = 1) {
        //turn on IR lamp
        digitalWrite(IR_LAMP_PIN, HIGH);
      }
      uint32_t picture_number_start = picture_number;
      start_video(15);
      digitalWrite(IR_LAMP_PIN, LOW);
      update_txt(picture_number_start);
    }
  } else if (wake_cause == ESP_SLEEP_WAKEUP_GPIO) {
    // gpio wake: start server.
    host_server();
    delay(15 * 60 * 1000);  //open server for 15 min
    WiFi.disconnect(true);
  }

  if (millis() - lastWakeUpTime >= 24L * 60L * 60L * 1000L) {
    //if 24 hours has elapsed, wake the pi and send data.
    lastWakeUpTime = millis();
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
    pinMode(RF_LINK_PIN, OUTPUT);
    digitalWrite(RF_LINK_PIN, HIGH);
    delay(1000);
    digitalWrite(RF_LINK_PIN, LOW);
    host_server();
    pinMode(RF_LINK_PIN, INPUT);
    esp_sleep_enable_gpio_wakeup();
    delay(5 * 60 * 1000);  //open server for 5 min
    WiFi.disconnect(true);
    clean_files();
  }
}