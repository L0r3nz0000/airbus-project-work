#include <Arduino.h>
#include <math.h>
#include "camera.h"

CameraOV7670 camera(CameraOV7670::RESOLUTION_QQVGA_160x120, CameraOV7670::PIXEL_YUV422, 4);

// Parametri per il filtro
uint8_t mean = 127;
float factor = 1.5;

uint8_t contrastFilter(uint8_t value) {
  // return value;
  value = 1.5 * (value - 45);
  return value < 0 ? 0 : value > 255 ? 255 : value;
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