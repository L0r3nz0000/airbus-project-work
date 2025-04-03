import serial
import numpy as np
import cv2

# Configura la porta seriale
port = '/dev/ttyACM0'
baudrate = 500000

COMMAND = 0x0
COMMAND_NEW_FRAME = 0x01 | 0x10
COMMAND_DEBUG_DATA = 0x03 | 0x10
COMMAND_END = 0x04 | 0x10
PIXEL_FORMAT_GRAYSCALE = 0x02

cv2.namedWindow("Image", cv2.WINDOW_NORMAL)

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
            print("❌ Errore: Payload insufficiente per NEW_FRAME")
            continue

          # Combina i due byte per ottenere la variabile a 16 bit
          width = (data[1] << 8) | data[0]
          height = (data[3] << 8) | data[2]
          pixel_format = data[4]

          print(f"📷 Nuovo frame - Width: {width}, Height: {height}, Pixel format: {pixel_format}")
          if pixel_format == PIXEL_FORMAT_GRAYSCALE:
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

            cv2.imshow("Image", resized_image)
            cv2.waitKey(1)
            
            print("done ({} bytes)".format(len(image)*len(image[0])))
          else:
            print("❌ Errore: Pixel format non supportato")
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
            
          print(f"🐞 Debug: {debug_message.decode(errors='ignore')}")

        else:
          print(f"❓ Comando sconosciuto: {command_byte.hex()}")
      else:
        print(f"❓ Byte sconosciuto: {command_byte.hex()}")

except serial.SerialException as e:
    print(f"Errore: {e}")
except KeyboardInterrupt:
    print("\nChiusura della connessione seriale...")
    ser.close()
