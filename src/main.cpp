#include <Arduino.h>
#include <math.h>
#include "camera.h"

CameraOV7670 camera(CameraOV7670::RESOLUTION_QQVGA_160x120, CameraOV7670::PIXEL_YUV422, 4);

// x: 152 y: 88; x: 222 y: 106
typedef struct {
  int x1, y1, x2, y2;
} Rettangolo;

int posizione_display_x = 326, posizione_display_y = 105;

Rettangolo segmenti[] = {
  {  67, 174,  3,  58 },  // A
  { 139,  54, 149, 104 }, // B
  { 124, 139, 130, 201 }, // C
  {  43, 203, 109, 219 }, // D
  {  33, 140,  41, 198 }, // E
  {  48,  51,  55, 111 }, // F
  {  57, 117, 117, 137 }, // G
};

/*
la funzione filter viene chiamata per ogni pixel e deve capire in quale segmento sta per calcolarne la relativa media
*/

int media_segmenti[sizeof(segmenti) / sizeof(Rettangolo)] = {0};
int media_segmenti_count[sizeof(segmenti) / sizeof(Rettangolo)] = {0};

uint8_t filter(uint8_t pixel, int x, int y) {
  // for (uint8_t i = 0; i < sizeof(segmenti) / sizeof(Rettangolo); i++) {
  //   if (x >= segmenti[i].x1 && x <= segmenti[i].y1 && y >= segmenti[i].x2 && y <= segmenti[i].y2) {
  //     media_segmenti[i] += pixel;
  //     media_segmenti_count[i]++;
  //     if (media_segmenti_count[i] > 0) {
  //       media_segmenti[i] /= media_segmenti_count[i];
  //     }
  //     if (media_segmenti_count[i] > 100) {
  //       media_segmenti_count[i] = 0;
  //       media_segmenti[i] = 0;
  //     }
  //   }
  // }
  return pixel;
}

void setup() {
  for (uint8_t i = 0; i < sizeof(segmenti) / sizeof(Rettangolo); i++) {
    segmenti[i].x1 += posizione_display_x;
    segmenti[i].y1 += posizione_display_y;
    segmenti[i].x2 += posizione_display_x;
    segmenti[i].y2 += posizione_display_y;
  }
  initializeCamera(camera);
}

void loop() {
  processFrame(camera, filter);
}