#include <Arduino.h>
#include <math.h>
#include "camera.h"

CameraOV7670 camera(CameraOV7670::RESOLUTION_QQVGA_160x120, CameraOV7670::PIXEL_YUV422, 4);

// x: 152 y: 88; x: 222 y: 106
typedef struct {
  int x1, x2, x3, x4;
} Rettangolo;

int posizione_display_x = 85, posizione_display_y = 48;

Rettangolo segmenti[] = {
  { 152, 222, 88, 106 },  // A
  { 224, 102, 234, 152 }, // B
  { 209, 187, 215, 249 }, // C
  { 128, 251, 194, 267 }, // D
  { 118, 188, 126, 246 }, // E
  { 133, 99, 140, 159 },  // F
  { 142, 165, 202, 185 }, // G
};

/*
la funzione filter viene chiamata per ogni pixel e deve capire in quale segmento sta per calcolarne la relativa media
*/

uint8_t filter(uint8_t pixel, int x, int y) {
  for (int i = 0; i < sizeof(segmenti) / sizeof(Rettangolo); i++) {
    if (x >= segmenti[i].x1 && x <= segmenti[i].x2 && y >= segmenti[i].x3 && y <= segmenti[i].x4) {
      return 0;
    }
  }
  return pixel;
}

void setup() {
  initializeCamera(camera);
}

void loop() {
  processFrame(camera, filter);
}