# ssd1306_spi.py
# author: Venkata Naga Ravikiran Bulusu

# OLED GND → Pin 6 (GND) on Raspberry Pi
# OLED VCC → Pin 1 (3.3V) on Raspberry Pi
# OLED D0 (SCK/CLK) → Pin 23 (SCLK) on Raspberry Pi
# OLED D1 (MOSI) → Pin 19 (MOSI) on Raspberry Pi
# OLED RES → Pin 18 (GPIO 24) on Raspberry Pi
# OLED DC → Pin 22 (GPIO 25) on Raspberry Pi
# OLED CS → Pin 24 (CE0) on Raspberry Pi

import board
import digitalio
import adafruit_ssd1306
from PIL import Image, ImageDraw, ImageFont
import time

# Define the Reset Pin
oled_reset = digitalio.DigitalInOut(board.D24)

# Define the SPI connection
spi = board.SPI()
oled_cs = digitalio.DigitalInOut(board.CE0)
oled_dc = digitalio.DigitalInOut(board.D25)

# Create the SSD1306 OLED class
oled = adafruit_ssd1306.SSD1306_SPI(128, 64, spi, oled_dc, oled_reset, oled_cs)

# Clear display
oled.fill(0)
oled.show()

# Create blank image for drawing.
image = Image.new("1", (oled.width, oled.height))

# Get drawing object to draw on image.
draw = ImageDraw.Draw(image)

# Load default font.
font = ImageFont.load_default()

# Draw text
draw.text((0, 0), "Welcome chuchulu!", font=font, fill=255)

# Display image
oled.image(image)
oled.show()

# Pause for 5 seconds to show the text
time.sleep(5)

# Clear the display buffer before exiting
oled.fill(0)
oled.show()

print("Display cleared and program exiting.")