#ifndef DEBUG_H
#define DEBUG_H
#include "Arduino.h"
#include "CameraOV7670.h"

typedef void (*ProcessFrameData)(CameraOV7670 &camera);
const uint16_t UART_PIXEL_FORMAT_RGB565 = 0x01;
const uint16_t UART_PIXEL_FORMAT_GRAYSCALE = 0x02;
const uint8_t VERSION = 0x10;
const uint8_t COMMAND_NEW_FRAME = 0x01 | VERSION;
const uint8_t COMMAND_DEBUG_DATA = 0x03 | VERSION;
const uint8_t COMMAND_END = 0x04 | VERSION;

// Pixel byte parity check:
// Pixel Byte H: odd number of bits under H_BYTE_PARITY_CHECK and H_BYTE_PARITY_INVERT
// Pixel Byte L: even number of bits under L_BYTE_PARITY_CHECK and L_BYTE_PARITY_INVERT
//                                          H:RRRRRGGG
const uint8_t H_BYTE_PARITY_CHECK =  0b00100000;
const uint8_t H_BYTE_PARITY_INVERT = 0b00001000;
//                                          L:GGGBBBBB
const uint8_t L_BYTE_PARITY_CHECK =  0b00001000;
const uint8_t L_BYTE_PARITY_INVERT = 0b00100000;
// Since the parity for L byte can be zero we must ensure that the total byet value is above zero.
// Increasing the lowest bit of blue color is OK for that.
const uint8_t L_BYTE_PREVENT_ZERO  = 0b00000001;

const uint16_t COLOR_GREEN = 0x07E0;
const uint16_t COLOR_RED = 0xF800;

const uint16_t lineLength = 160;
const uint16_t lineCount = 120;
const uint32_t baud  = 500000;
void processGrayscaleFrameBuffered(CameraOV7670 &camera);
void processFilter(CameraOV7670 &camera, uint8_t (*filter)(uint8_t));
const ProcessFrameData processFrameData = processGrayscaleFrameBuffered;
const bool isSendWhileBuffering = true;
const uint8_t uartPixelFormat = UART_PIXEL_FORMAT_GRAYSCALE;

uint8_t lineBuffer [lineLength];
uint8_t * lineBufferSendByte;
bool isLineBufferSendHighByte;
bool isLineBufferByteFormatted;

uint16_t frameCounter = 0;
uint16_t processedByteCountDuringCameraRead = 0;

void commandStartNewFrame(uint8_t pixelFormat);
void commandDebugPrint(const String debugText);
uint8_t sendNextCommandByte(uint8_t checksum, uint8_t commandByte);

void sendBlankFrame(uint16_t color);
inline void sendPixelFromBuffer() __attribute__((always_inline));
inline void tryToSendNextRgbPixelByteInBuffer() __attribute__((always_inline));
inline void formatNextRgbPixelByteInBuffer() __attribute__((always_inline));
inline uint8_t formatRgbPixelByteH(uint8_t byte) __attribute__((always_inline));
inline uint8_t formatRgbPixelByteL(uint8_t byte) __attribute__((always_inline));
inline uint8_t formatPixelByteGrayscaleFirst(uint8_t byte) __attribute__((always_inline));
inline uint8_t formatPixelByteGrayscaleSecond(uint8_t byte) __attribute__((always_inline));
inline void waitForPreviousUartByteToBeSent() __attribute__((always_inline));
inline bool isUartReady() __attribute__((always_inline));
void debugPrint(String message);
void sendFrameOverSerial();


uint8_t formatPixelByteGrayscaleFirst(uint8_t pixelByte) {
  // For the First byte in the parity chek byte pair the last bit is always 0.
  pixelByte &= 0b11111110;
  if (pixelByte == 0) {
    // Make pixel color always slightly above 0 since zero is a command marker.
    pixelByte |= 0b00000010;
  }
  return pixelByte;
}

uint8_t formatPixelByteGrayscaleSecond(uint8_t pixelByte) {
  // For the second byte in the parity chek byte pair the last bit is always 1.
  return pixelByte | 0b00000001;
}

void processGrayscaleFrameBuffered(CameraOV7670 &camera) {
  camera.waitForVsync();
  camera.ignoreVerticalPadding();

  for (uint16_t y = 0; y < lineCount; y++) {
    lineBufferSendByte = lineBuffer;
    camera.ignoreHorizontalPaddingLeft();

    uint16_t x = 0;
    while (x < lineLength) {
      camera.waitForPixelClockRisingEdge(); // YUV422 grayscale byte
      camera.readPixelByte(lineBuffer[x]);
      lineBuffer[x] = formatPixelByteGrayscaleFirst(lineBuffer[x]);

      camera.waitForPixelClockRisingEdge(); // YUV422 color byte.
      if (isSendWhileBuffering) {
        sendPixelFromBuffer();
      }
      x++;

      camera.waitForPixelClockRisingEdge(); // YUV422 grayscale byte
      camera.readPixelByte(lineBuffer[x]);
      lineBuffer[x] = formatPixelByteGrayscaleSecond(lineBuffer[x]);

      camera.waitForPixelClockRisingEdge(); // YUV422 color byte.
      if (isSendWhileBuffering) {
        sendPixelFromBuffer();
      }
      x++;
    }
    camera.ignoreHorizontalPaddingRight();

    // Debug info to get some feedback how mutch data was processed during line read.
    processedByteCountDuringCameraRead = lineBufferSendByte - (&lineBuffer[0]);

    // Send rest of the line
    while (lineBufferSendByte < &lineBuffer[lineLength]) {
      sendPixelFromBuffer();
    }
  };
}

void processFilter(CameraOV7670 &camera, uint8_t (*filter)(uint8_t, int, int)) {
  camera.waitForVsync();
  camera.ignoreVerticalPadding();

  for (uint16_t y = 0; y < lineCount; y++) {
    lineBufferSendByte = lineBuffer;
    camera.ignoreHorizontalPaddingLeft();

    for (uint16_t x = 0; x < lineLength; x += 2) {
      // Leggi il primo pixel (Y0)
      camera.waitForPixelClockRisingEdge();
      camera.readPixelByte(lineBuffer[x]);
      lineBuffer[x] = formatPixelByteGrayscaleFirst(filter(lineBuffer[x], x, y));

      // Salta il byte di crominanza blu (U)
      camera.waitForPixelClockRisingEdge();
      sendPixelFromBuffer();

      // Leggi il secondo pixel (Y1)
      camera.waitForPixelClockRisingEdge();
      camera.readPixelByte(lineBuffer[x+1]);
      lineBuffer[x+1] = formatPixelByteGrayscaleSecond(filter(lineBuffer[x+1], x+1, y));
      
      sendPixelFromBuffer();

      // Salta il byte di crominanza rossa (V)
      camera.waitForPixelClockRisingEdge();
    }
    
    camera.ignoreHorizontalPaddingRight();

    while (lineBufferSendByte < &lineBuffer[lineLength]) {
      sendPixelFromBuffer();
    }
  }
}

// this is called in Arduino loop() function
void processFrame(CameraOV7670 &camera, uint8_t (*filter)(uint8_t, int, int)) {
  processedByteCountDuringCameraRead = 0;
  commandStartNewFrame(uartPixelFormat);
  noInterrupts();
  processFilter(camera, filter);
  interrupts();
  
  frameCounter++;
  debugPrint("Frame " + String(frameCounter));
}

void initializeCamera(CameraOV7670 &camera) {

  // Enable this for WAVGAT CPUs
  // For UART communiation we want to set WAVGAT Nano to 16Mhz to match Atmel based Arduino
  //CLKPR = 0x80; // enter clock rate change mode
  //CLKPR = 1; // set prescaler to 1. WAVGAT MCU has it 3 by default.

  Serial.begin(baud);
  if (camera.init()) {
    sendBlankFrame(COLOR_GREEN);
    delay(1000);
  } else {
    sendBlankFrame(COLOR_RED);
    delay(3000);
  }
}

void sendBlankFrame(uint16_t color) {
  uint8_t colorH = (color >> 8) & 0xFF;
  uint8_t colorL = color & 0xFF;

  // commandStartNewFrame(UART_PIXEL_FORMAT_RGB565);
  commandStartNewFrame(UART_PIXEL_FORMAT_GRAYSCALE);
  for (uint16_t j=0; j<lineCount; j++) {
    for (uint16_t i=0; i<lineLength; i++) {
      waitForPreviousUartByteToBeSent();
      // UDR0 = formatRgbPixelByteH(colorH);
      // waitForPreviousUartByteToBeSent();
      // UDR0 = formatRgbPixelByteL(colorL);
      UDR0 = 0xFF;  // bianco
    }
  }
}

// RRRRRGGG
uint8_t formatRgbPixelByteH(uint8_t pixelByteH) {
  // Make sure that
  // A: pixel color always slightly above 0 since zero is end of line marker
  // B: odd number of bits for H byte under H_BYTE_PARITY_CHECK and H_BYTE_PARITY_INVERT to enable error correction
  if (pixelByteH & H_BYTE_PARITY_CHECK) {
    return pixelByteH & (~H_BYTE_PARITY_INVERT);
  } else {
    return pixelByteH | H_BYTE_PARITY_INVERT;
  }
}


// GGGBBBBB
uint8_t formatRgbPixelByteL(uint8_t pixelByteL) {
  // Make sure that
  // A: pixel color always slightly above 0 since zero is end of line marker
  // B: even number of bits for L byte under L_BYTE_PARITY_CHECK and L_BYTE_PARITY_INVERT to enable error correction
  if (pixelByteL & L_BYTE_PARITY_CHECK) {
    return pixelByteL | L_BYTE_PARITY_INVERT | L_BYTE_PREVENT_ZERO;
  } else {
    return (pixelByteL & (~L_BYTE_PARITY_INVERT)) | L_BYTE_PREVENT_ZERO;
  }
}

void debugPrint(String message) {
  waitForPreviousUartByteToBeSent();
  UDR0 = 0x0;
  waitForPreviousUartByteToBeSent();
  UDR0 = COMMAND_DEBUG_DATA;

  for (uint16_t i = 0; i < message.length(); i++) {
    waitForPreviousUartByteToBeSent();
    UDR0 = message[i];
  }
  waitForPreviousUartByteToBeSent();
  UDR0 = COMMAND_END;
}

void commandStartNewFrame(uint8_t pixelFormat) {
  waitForPreviousUartByteToBeSent();
  UDR0 = 0x0;  // New command
  waitForPreviousUartByteToBeSent();
  UDR0 = COMMAND_NEW_FRAME;

  uint8_t low_byte = lineLength & 0xFF;
  uint8_t high_byte = (lineLength >> 8) & 0xFF;

  waitForPreviousUartByteToBeSent();
  UDR0 = low_byte; // image width (low)
  waitForPreviousUartByteToBeSent();
  UDR0 = high_byte; // image width (high)
  
  low_byte = lineCount & 0xFF;
  high_byte = (lineCount >> 8) & 0xFF;
  waitForPreviousUartByteToBeSent();
  UDR0 = low_byte; // image height (low)
  waitForPreviousUartByteToBeSent();
  UDR0 = high_byte; // image height (high)

  waitForPreviousUartByteToBeSent();
  UDR0 = pixelFormat;  // pixel format
}

void sendPixelFromBuffer() {
  if (isUartReady()) {
    UDR0 = *lineBufferSendByte;
    lineBufferSendByte++;
  }
}

uint8_t sendNextCommandByte(uint8_t checksum, uint8_t commandByte) {
  waitForPreviousUartByteToBeSent();
  UDR0 = commandByte;
  return checksum ^ commandByte;
}

void waitForPreviousUartByteToBeSent() {
  while(!isUartReady()); //wait for byte to transmit
}


bool isUartReady() {
  return UCSR0A & (1<<UDRE0);
}

#endif