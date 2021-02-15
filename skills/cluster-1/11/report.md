#  Skill Name

Author: Samuel Sze  

Date: 2021-02-15
-----

## Summary
1. Initialize LED display, push button, and timer in ESP32.
2. Declare interrupt function using global variable 'flag' on push button. 
3. Declare global variable 'time_counter' corresponding to timer in ESP32. 

    i. 'time_counter' counts up everytime an interrupt occurs in the timer (set to 1second intervals).
4. Display 'time_counter' variable on LED display. 
6. On push button press, reset 'time_counter' variable and start counting from 0 again. 
## Sketches and Photos


## Modules, Tools, Source Used Including Attribution
Sources:

    1. http://whizzer.bu.edu/skills/timer
    2. http://whizzer.bu.edu/briefs/design-patterns/dp-timer
    3. https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/timer.html
    4. https://github.com/espressif/esp-idf/blob/17ac4bad7381e579e5a7775755cc25480da47d97/examples/peripherals/timer_group/main/timer_group_example_main.c

## Supporting Artifacts
Link to video artifact (shows LED timer counting up, push button testing to reset timer)
https://drive.google.com/file/d/1DMdxPMbmY0MGq_p7Ys5biuklzi6jkY1d/view?usp=sharing

-----
