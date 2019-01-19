# Aquarium
Automised aquarium with STM32

## Description of the project:
Automated aquarium with the following features:
- temperature measurement
- maintaining given temperature
- feeding
- managing by wi-fi

## Required elements
1) STM32F4
2) [Display Nokia 5110](https://www.sparkfun.com/datasheets/LCD/Monochrome/Nokia5110.pdf)
3) [Programmable Resolution 1-Wire Digital Thermometer DS18B20](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf)
4) []
5)
6) [4x4 Matrix Membrane Keypad](https://www.parallax.com/sites/default/files/downloads/27899-4x4-Matrix-Membrane-Keypad-v1.2.pdf)
7) [SERVO MOTOR SG90](http://www.ee.ic.ac.uk/pcheung/teaching/DE1_EE/stores/sg90_datasheet.pdf)
8) [ESP-12F WiFi Module](https://www.elecrow.com/download/ESP-12F.pdf)

## Microcontroller pin configuration

### Display:
For dislpay we need 8 pins, we connect it as follows: RST - PB10, CE - PB14, DC - PB12, DIN - PB15, CLK -PB13, VCC - 5V, BL - PB7, GND - Ground.
The functions for display is in [aq_interface.c](Src/aq_interface.c)

### Thermometer:
For thermometer we need 3 pins

### Keyboard:
From left to right first four for output - PD8-PD11, next four for input - PD0-PD3 (configured for interrupt).
The code for keyboard is in [keyboard.c](Src/keyboard.c).

### 
