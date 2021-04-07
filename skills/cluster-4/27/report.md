#  ESP32 FSM

Author: Samuel Sze

Date: 2021-04-05

-----

## Summary
1. Read up on Finite State machines. 

2. Create sketch on FSM for whack-a-mole

3. Create Pseudo C-code for whack-a-mole FSM.

    a. Psuedo code attached under code directory. 

## State table
| Event\State  | No mole  | One mole  |
|---|---|---|
|  Whack Right |  No mole |  No mole + points |
|  Whack Wrong |  No mole | No mole  |
|  Idle timout |  One mole | No mole |

## Sketches and Photos
<img src="images/download.png" width="" height="" />


## Modules, Tools, Source Used Including Attribution
Sources:

    1. http://whizzer.bu.edu/skills/state-models

    2. http://whizzer.bu.edu/briefs/design-patterns/dp-state-machine

## Supporting Artifacts
