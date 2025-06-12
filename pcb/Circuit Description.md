# PROJECT-TITLE Circuit Description

## ATmega32U4
- Used this processor because it's compatible with QMK. Has a built in USB controller.
- 5V supply via USB.
- Minimum 500R current limiting resistors on GPIO pins as seen on Table 29-1 of the datasheet. 
- USB Implementation based on Section 21.3 Typical Application Implementation of datasheet.