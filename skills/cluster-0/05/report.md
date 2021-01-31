#  Setup Espressif Toolchain

Author: Samuel Sze

Date: 2021-01-30
-----

## Summary
1. Install USD driver CP2104 - directory in EC444 local folder.
2. Install Espressif Installer for Windows - directory under C:\Users\Samuel\esp
3. Created command prompt for ESP-IDF
4. Started project blink under examples/get-started/blink:

        cd %userprofile%\esp
        xcopy /e /i %IDF_PATH%\examples\get-started\blink blink

5. Connect ESP32 to computer, note down serial port under device manager. 
6. Configure and build project, for blinking onboard LED, the GPIO pin is 13. This is changed in menuconfig. 

        cd %userprofile%\esp\blink
        idf.py set-target esp32 #set build target
        idf.py menuconfig  #configure various parameters
        idf.py build    #build project
        idf.py -p PORT flash    #flash onto device
        idf.py -p PORT monitor  #monitor project. 

## Sketches and Photos
**Click on photo for youtube link**

[![](http://img.youtube.com/vi/gCvh9C8rbUM/0.jpg)](http://www.youtube.com/watch?v=gCvh9C8rbUM "ESP-IDF blink project")
* ESP32 board at the bottom right of the video. 
* ESP-CMD on the left side, monitor project is active.

## Modules, Tools, Source Used Including Attribution
Sources:

    1. https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#step-3-set-up-the-tools
    2. https://github.com/espressif/esp-idf/tree/v4.2/examples
    3. https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/pinouts

## Supporting Artifacts


-----
