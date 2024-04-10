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

bool showCircleAnimation = true; // 控制是否显示大小圆切换动画的标志
bool showBigCircle = true; // 控制显示大圆还是小圆

bool showCross = false;           // 控制是否显示叉号的标志
bool showCircleProgress = false;  // 控制是否显示大圆顺时针点亮动画的标志
int currentPixel = 0;             // 当前正在处理的像素点在大圆中的位置

// 大圆形的图案
uint8_t bigCircle[8] = { 0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C };
// 小圆形的图案
uint8_t smallCircle[8] = {0x00, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00};

// 顺时针点亮大圆的LED顺序
int circleProgress[20] = { 5, 4, 3, 2, 9, 16, 24, 32, 40, 49, 58, 59, 60, 61, 54, 47, 39, 31, 23, 14 };

// 叉形的图案
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
    mx.setPoint(row, col, true);  // 点亮当前像素点
    mx.update();
    currentPixel = (currentPixel + 1) % 20;  // 移动到下一个像素点
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    String text = String((char*)payload).substring(0, length);
    Serial.println(text);

    if (text == "预览开始") {
      showCircleAnimation = true;  // 开启大小圆切换动画
      showCircleProgress = false;  // 停止大圆顺时针点亮动画
      previousMillis = millis();   // 重置计时器
    } else if (text == "开始分析") {
      showCircleAnimation = false;  // 停止大小圆切换动画
      showCircleProgress = true;    // 开启大圆顺时针点亮动画
      previousMillis = millis();    // 重置计时器用于控制像素点的点亮
      currentPixel = 0;             // 从大圆的第一个像素点开始
    } else if (text == "未检测到人体") {
      showCross = true;
      displayCross();
      showCircleProgress = false;
    } else if (text == "好的坐姿") {
      showCircleProgress = false; 
      showCross = false;
      drawHappyFace();  
    } else if (text == "坏的坐姿") {
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
    drawCircle(showBigCircle);       // 切换大小圆
    showBigCircle = !showBigCircle;  // 切换显示的圆形大小
  }

  if (showCircleProgress && currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    updateCircleProgress();  // 更新大圆的顺时针点亮动画
  }
}
