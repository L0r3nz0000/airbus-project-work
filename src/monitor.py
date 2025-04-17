import serial
import numpy as np
import cv2
import time

# Configura la porta seriale
port = '/dev/ttyACM0'
baudrate = 500000

COMMAND = 0x0
COMMAND_NEW_FRAME = 0x01 | 0x10
COMMAND_DEBUG_DATA = 0x03 | 0x10
COMMAND_END = 0x04 | 0x10
PIXEL_FORMAT_GRAYSCALE = 0x02

cv2.namedWindow("Image", cv2.WINDOW_NORMAL)
frame_counter = 0

starting_point = (-1, -1)
segments = [
  [70, 35],   [130, 48],
  [136, 55],  [146, 98],
  [120, 136], [128, 180],
  [46, 195],  [101, 207],
  [38, 136],  [48, 180],
  [49, 58],   [59, 96],
  [64, 115],  [113, 125]
]

medie = [-1 for _ in range(7)]

def mouse_callback(event, x, y, flags, param):
  global starting_point
  if event == cv2.EVENT_LBUTTONDOWN:
    if -1 in starting_point:
      starting_point = (x, y)
      
      print(f"Starting point set to: {starting_point}")
      
cv2.setMouseCallback("Image", mouse_callback)

def media_segmento(frame, x1, y1, x2, y2):
  media = np.sum(frame[y1:y2+1, x1:x2+1])
  punti = (y2 - y1 + 1) * (x2 - x1 + 1)
  return (media / punti) / 255 if punti > 0 else 0

start_time = time.time()

try:
  ser = serial.Serial(port, baudrate, timeout=0)
  print(f"Connesso a {port} a {baudrate} baud")

  while True:
    if ser.in_waiting > 1:
      command_byte = ser.read(1)
      if command_byte[0] == COMMAND:
        command_byte = ser.read(1)

        if command_byte[0] == COMMAND_NEW_FRAME:
          data = ser.read(5)
          if len(data) < 5:
            print("Errore: Payload insufficiente per NEW_FRAME")
            continue

          # Combina i due byte per ottenere la variabile a 16 bit
          width = (data[1] << 8) | data[0]
          height = (data[3] << 8) | data[2]
          pixel_format = data[4]

          print(f"fps: {1 / (time.time() - start_time)}")
          start_time = time.time()
          print(f"* Nuovo frame - Width: {width}, Height: {height}, Pixel format: {pixel_format}")
          if pixel_format == PIXEL_FORMAT_GRAYSCALE:
            frame_counter += 1
            print("reading image...")
            image = []
            
            for y in range(height):
              image.append([])
              for x in range(width):
                while ser.in_waiting == 0: pass
                
                byte = ser.read(1)
                if byte:
                  image[y].append(byte[0])
            image_array = np.array(image, dtype=np.uint8)
            resized_image = cv2.resize(image_array, None, fx=3, fy=3, interpolation=cv2.INTER_LINEAR)

            if frame_counter == 3:
              while -1 in starting_point:
                cv2.waitKey(1)
              
              for i in range(len(segments)):
                segments[i][0] += starting_point[0]
                segments[i][1] += starting_point[1]
            elif frame_counter > 3:
              print(np.max(resized_image))
              for i in range(0, len(segments), 2):
                medie[i//2] = media_segmento(resized_image, segments[i][0], segments[i][1], segments[i+1][0], segments[i+1][1])

                cv2.rectangle(resized_image, (segments[i][0], segments[i][1]), (segments[i+1][0], segments[i+1][1]), (0, 255, 0), -1)
              print("medie: ", medie)
              
            cv2.imshow("Image", resized_image)
            cv2.waitKey(1)
            
            print("done ({} bytes)".format(len(image)*len(image[0])))
          else:
            print("Errore: Pixel format non supportato")
            exit(1)

        elif command_byte[0] == COMMAND_DEBUG_DATA:
          debug_message = bytearray()
          while True:
            byte = ser.read(1)
            if byte:
              if byte[0] == COMMAND_END:
                break
              else:
                debug_message.append(byte[0])
            
          print(f"Debug: {debug_message.decode(errors='ignore')}")

        else:
          print(f"Comando sconosciuto: {command_byte.hex()}")
      else:
        print(f"Byte sconosciuto: {command_byte.hex()}")

except serial.SerialException as e:
    print(f"Errore: {e}")
except KeyboardInterrupt:
    print("\nChiusura della connessione seriale...")
    ser.close()
