#include <WiFi.h>
#include <esp_camera.h>
#include "Thing.CoAP.h"

// CoAP client and packet provider
Thing::CoAP::Client coapClient;
Thing::CoAP::ESP::UDPPacketProvider udpProvider;

// Camera configuration
#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#endif

// Wi-Fi credentials
const char* ssid = "Huawei nova 7";
const char* password = "ftogames97000111";

// CoAP server configuration
IPAddress serverIP(192, 168, 43, 42);  // CoAP server's IP address
const uint16_t serverPort = 5683;       // CoAP server port (usually 5683)

// Camera initialization
void initCamera() {
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.println("Camera initialization failed");
    while (1);
  }
}

// Function to send image as PUT request
void sendImage() {
  // Capture a frame from the camera
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Print debug information
  Serial.print("Sending to IP: ");
  Serial.println(serverIP);
  Serial.print("Payload size: ");
  Serial.println(fb->len);

  // Prepare image data for CoAP PUT request
  std::vector<uint8_t> payload(fb->buf, fb->buf + fb->len);

  // Send the image as a PUT request
  Serial.println("coapClient.put is executed:");
  coapClient.Put("/video", payload, [](Thing::CoAP::Response response) {
    // Debugging: Print server response
    Serial.println("CoAP response received:");
    std::vector<uint8_t> responsePayload = response.GetPayload();
    std::string received(responsePayload.begin(), responsePayload.end());
    Serial.println("Server response:");
    Serial.println(received.c_str());
  });

  

  // Return the frame buffer to the driver
  Serial.println("return fb");
  esp_camera_fb_return(fb);

  // Schedule the next image capture
  delay(5000);
  Serial.println("sendImage() executed");

  sendImage();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize the camera
  initCamera();

  // Configure CoAP client
  Serial.println("Configuring CoAP client...");
  coapClient.SetPacketProvider(udpProvider); // Attach UDP provider
  coapClient.Start(serverIP, serverPort);    // Start CoAP with server IP and port
  Serial.println("CoAP client configured.");

  // Send the first image
  sendImage();
}

void loop() {
  // Process CoAP client
  coapClient.Process();
}
