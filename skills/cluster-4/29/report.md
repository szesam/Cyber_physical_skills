#  Security in Connected Systems

Author: Samuel Sze

Date: 2021-04-07

-----

## Summary
Analyze security issues.

### Overall Flow of information to drive a car with remote control over the internet
1. Remote control(client) sends UDP packet over internet to car's Raspberry Pi(server) located in a local area network through DDNS port forwarding.

    a. When certain event is triggered on the webpage (html), UDP packet is sent back to server using socketio. 

    b. Car's Pi server runs on nodejs UDP server with interface to webpage through socketio.

2. Car's Raspberry Pi sends the packet to ESP32 through UDP in a local area network where the microcontroller interpolates the UDP packet to retrieve meaningful data (Pose, Acceleration, Speed).

3. At the same time, Car's PI sends acknowledge UDP packet back to internet. 

4. ESP32 performs the task. Sends updated UDP information back to Raspberry PI. 

4. Car's PI sends back updated information packet back to internet.  

5. Internet receives packet, replies with UDP acknowledgment. 

6. Repeat from step 1.

### Weaknesses of overall system

## Sketches and Photos


## Modules, Tools, Source Used Including Attribution


## Supporting Artifacts


-----
