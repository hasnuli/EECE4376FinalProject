# Example using an RGB character LCD wired directly to Raspberry Pi or BeagleBone Black.
import time
import _retrieveNote
import Adafruit_CharLCD as LCD

def detPosition(pitch, lowBound, highBound, mid):
    pos = ((pitch-lowBound)/(highBound-lowBound))*7
    if (pitch > mid)
         pos = pos + 50
    return pos

# BeagleBone Black configuration:
lcd_rs = 'P8_8'
lcd_en = 'P8_10'
lcd_d4 = 'P8_18'
lcd_d5 = 'P8_16'
lcd_d6 = 'P8_14'
lcd_d7 = 'P8_12'
lcd_red   = 'P8_7'
lcd_green = 'P8_9'
lcd_blue  = 'P8_11'

# Define LCD column and row size for 16x2 LCD.
lcd_columns = 16
lcd_rows    = 2

# Initialize the LCD using the pins above.
lcd = LCD.Adafruit_RGBCharLCD(lcd_rs, lcd_en, lcd_d4, lcd_d5, lcd_d6, lcd_d7, lcd_columns, lcd_rows, lcd_red, lcd_green, lcd_blue)

while True:
    # Screen for Tuner State
    if (returnState_returnState() == 0):
        # Clear screen, setup default characters and place them in the home position
        lcd.clear()
        lcd.home()
        lcd.message('b')
        lcd.set_cursor(15, 1)
        lcd.message('#')
        while (returnState_returnState() == 0):
            # Pitch[0], highBound[1], lowBound[2], mid[3], note[4]
            lcd.set_cursor(7, 1)
            n = retrieveNote_retrieveNote()
            lcd.message(n[4])
            lcd.set_cursor(detPosition(n[0], n[2], n[1], n[3]), 2)
    # Screen for Playback State
    elif (returnState_returnState() == 2):
        lcd.clear()
        lcd.set_cursor(2, 1)
        lcd.message('Playing Track')
    # Screen for Recording State
    elif (returnState_returnState() == 2 and returnState_returnState() == 3):
        lcd.clear()
        lcd.set_cursor(1, 1)
        lcd.message('Recording Audio')
    elif (returnState_returnState() == 0)
        lcd.clear()
        lcd.set_backlight(0)

