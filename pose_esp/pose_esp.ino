#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include "secret.h"
#include <MD_MAX72xx.h>
#include <SPI.h>

#define MAX7219_CLK 14
#define MAX7219_CS 12
#define MAX7219_DIN 13

MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::GENERIC_HW, MAX7219_CS, 1);
WebSocketsServer webSocket = WebSocketsServer(81);

unsigned long previousMillis = 0;
const long interval = 500;

bool showCircleAnimation = true;
bool showBigCircle = true;

bool showCross = false;
bool showCircleProgress = false;
int currentPixel = 0;

uint8_t bigCircle[8] = { 0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C };

uint8_t smallCircle[8] = {0x00, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00};

int circleProgress[20] = { 5, 4, 3, 2, 9, 16, 24, 32, 40, 49, 58, 59, 60, 61, 54, 47, 39, 31, 23, 14 };

uint8_t cross[8] = { 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81 };

void drawCircle(bool big) {
  mx.clear();
  uint8_t* circleData = big ? bigCircle : smallCircle;
  for (uint8_t i = 0; i < 8; i++) {
    mx.setRow(0, i, circleData[i]);
  }
  mx.update();
}

void displayCross() {
  mx.clear();
  for (uint8_t i = 0; i < 8; i++) {
    mx.setRow(0, i, cross[i]);
  }
  mx.update();
}

void drawWarningTriangle() {
  mx.clear();
  
  // 绘制三角形
  mx.setRow(0, 1, 0b00011000);
  mx.setRow(0, 2, 0b00011000);
  mx.setRow(0, 3, 0b00011000);
  mx.setRow(0, 4, 0b00011000);
  mx.setRow(0, 6, 0b00011000);
  mx.setRow(0, 7, 0b00011000);
  
  mx.update();
}

void drawHappyFace() {
  mx.clear();

  mx.setRow(0, 1, 0b01100110);
  mx.setRow(0, 2, 0b01100110);
  mx.setRow(0, 5, 0b01000010);
  mx.setRow(0, 6, 0b00111100);
  
  mx.update();
}

void updateCircleProgress() {
  if (!showCross) {
    mx.clear();
    int pixel = circleProgress[currentPixel];
    int row = pixel / 8;
    int col = pixel % 8;
    mx.setPoint(row, col, true);
    mx.update();
    currentPixel = (currentPixel + 1) % 20;
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    String text = String((char*)payload).substring(0, length);
    Serial.println(text);

    if (text == "start preview") {
      showCircleAnimation = true;
      showCircleProgress = false;
      previousMillis = millis();
    } else if (text == "start analysis") {
      showCircleAnimation = false;
      showCircleProgress = true;
      previousMillis = millis();
      currentPixel = 0;
    } else if (text == "no human detected") {
      showCross = true;
      displayCross();
      showCircleProgress = false;
    } else if (text == "good posture") {
      showCircleProgress = false; 
      showCross = false;
      drawHappyFace();  
    } else if (text == "bad posture") {
      showCross = false;
      showCircleProgress = false;
      drawWarningTriangle();
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  mx.begin();
  mx.clear();
}

void loop() {
  webSocket.loop();

  unsigned long currentMillis = millis();
  if (showCircleAnimation && currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    drawCircle(showBigCircle);
    showBigCircle = !showBigCircle;
  }

  if (showCircleProgress && currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    updateCircleProgress();
  }
}
