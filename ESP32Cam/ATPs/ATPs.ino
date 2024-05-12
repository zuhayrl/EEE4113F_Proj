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
#include "ESP32_OV5640_AF.h"

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
  init_cam();
  init_SD_card();

  OV5640 ov5640 = OV5640();
  sensor_t *sensor = esp_camera_sensor_get();
  ov5640.start(sensor);

  if (ov5640.focusInit() == 0) {
    Serial.println("OV5640_Focus_Init Successful!");
  }

  // Set Continuous AF MODE
  if (ov5640.autoFocusMode() == 0) {
    Serial.println("OV5640_Auto_Focus Successful!");
  }

  camera_fb_t *fb;
  for (int i = 0; i < 10; i++) {
    fb = esp_camera_fb_get();
    String path = "/high" + String(i) + ".jpg";
    fs::FS &fs = SD_MMC;
    File file = fs.open(path.c_str(), FILE_WRITE);
    file.write(fb->buf, fb->len);
    file.close();
    esp_camera_fb_return(fb);
    Serial.println("Picture " + String(i) + " written");
  }
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
    config.frame_size = FRAMESIZE_HD;  // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 1;
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

  return;
  delay(time_s * 1000);
  capture = false;
  while ((eTaskGetState(SD_card_write) == eRunning) || (eTaskGetState(Camera_capture) == eRunning)) {}  //wait for tasks to finish.
  Serial.println("Writen from " + String(picture_number_start) + " to " + String(picture_number));
}

void loop() {
  // put your main code here, to run repeatedly:
}
