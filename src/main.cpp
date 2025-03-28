#include <Arduino.h>
#include <math.h>
#include "camera.h"

CameraOV7670 camera(CameraOV7670::RESOLUTION_QQVGA_160x120, CameraOV7670::PIXEL_YUV422, 4);

// Parametri per il filtro
uint8_t mean = 127;
float factor = 1.5;

uint8_t contrastFilter(uint8_t x) {
  // uint8_t t = x - 127;
  // uint8_t y = 2*x - 127 + 20;
  // y = y < 0 ? 0 : y;
  // y = y > 255 ? 255 : y;
  return x;
}

void setup() {
  initializeCamera(camera);
}

void loop() {
  debugPrint("inizio");
  processFrame(camera, contrastFilter);
  debugPrint("fine");

  // processFilter(camera);
  // sendFrameOverSerial();
  //blackRectangle(20, 20, 100, 100);
}