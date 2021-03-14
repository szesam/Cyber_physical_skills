#  ESP32 Accelerometer

Author: Samuel Sze

Date: 2021-03-13

-----

## Summary
Setup Accelerometer on ESP32 and configure necessary read/write function and roll/pitch conversion
1. Follow example on Adafruit to wire up the ADXL343 accelerometer on breadboard with ESP32
2. Create module that reads in acceleration in the x,y,z axes. Referencing i2c material on course website.

    a). Single-byte read. 

    b). Single-byte write.

    c). Multi-byte read - combine two 8 bit acceleration data into one signed 16 bit integer using bitwise operator OR and left shift.

3. Write function to convert static acceleration into tilt data. Referencing dfrobot documentation. 
3. Build, flash, take video


## Sketches and Photos

## Modules, Tools, Source Used Including Attribution
Sources:

    1. http://whizzer.bu.edu/briefs/design-patterns/dp-i2c
    
    2. https://cdn-learn.adafruit.com/assets/assets/000/070/556/original/adxl343.pdf?1549287964

    3. https://wiki.dfrobot.com/How_to_Use_a_Three-Axis_Accelerometer_for_Tilt_Sensing

    4. https://www.geeksforgeeks.org/bitwise-operators-in-c-cpp/#:~:text=The%20%7C%20(bitwise%20OR)%20in,the%20two%20bits%20are%20different.

    5. https://learn.adafruit.com/adxl343-breakout-learning-guide/overview

## Supporting Artifacts
Link to video:

        https://drive.google.com/file/d/1KbA4bBWBcSOFwAxE1fqDdToCJdZ_DJPD/view?usp=sharing