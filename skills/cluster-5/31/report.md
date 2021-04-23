#  LIDAR V3

Author: Samuel Sze

Date: 2021/04/23

-----

## Summary

1. Wire the LIDAR v3 to the ESP32 SDA, SLA, USB and GRD.

2. Use I2C template, modify the read and write register procedure based on the Lidar v3 datasheet.

3. Modify the task while loop to accomodate for different register read. 

    a. For v3, constantly read register 0x01 until first bit goes to 0, then read distance register at 0x8f. 

4. Double check to see if results makes sense.

## Sketches and Photos


## Modules, Tools, Source Used Including Attribution
Sources:

    1. http://whizzer.bu.edu/skills/lidar-lite

    2. http://static.garmin.com/pumac/LIDAR_Lite_v3_Operation_Manual_and_Technical_Specifications.pdf

    3. https://www.robotshop.com/community/blog/show/lidar-lite-laser-rangefinder-simple-arduino-sketch-of-a-180-degree-radar


## Supporting Artifacts


-----
