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

### Weaknesses in the overall system

1. UDP communication from Raspberry Pi to the internet has no security mechanisms in place to check if the message is legitimate or sent properly. 

    a. Man in the middle attack could happen where a malicious user pretends to be the internet or the Pi and send out information. 

    b. Denial of service attack to overload the Pi, causing malfunctions.

    c. Retriving sensitive data from the PI by intercepting the UDP packet. 

2. The Local Area Network (LAN) setup is not a secure wifi network. Therefore, it can be breached and other malicious users will have access to the ESP32 on the car, the Raspberry Pi, and relevant connected devices. 

3. Information on relevant IP addresses, ports and information can be viewed in the html sourcecode, leading to potential for malicious use. 

### "Bad Actor' attack on system
1. Physically attack the sensors and circuitry on the car.

2. SSH tunnel into the PI and steal data stored on the server. 

3. Impersonating as a PI/ESP32 on local area network and transmit malicious UDP packets to the internet.

4. Impersonate as the html webpage and send malicious UDP packets to the PI/ESP32.

5. Overload the html webpage by sending errornous commands (Pose, acceleration, velocity). 

### Mitigation
1. To prevent DDOS attacks by overloading the ESP32 with commands coming from external sources, I can set up a queue system to hold maximum number of requests before dropping oldest packets. 

2. Secure the UDP connection either by switching to TCP connection where encrpytion and secure transmission are garanteed, or setting up some certificate or public key agreement protocol. 

3. IoT provisioning each device onto an IT infrastructure. This requires each new device or sensor to only be enrolled within the system after authentication and verification. Also, this allows for a monitoring platform to be setup for the entire connected system. 


## Sketches and Photos


## Modules, Tools, Source Used Including Attribution
Sources:

    1. https://davra.com/how-iot-device-provisioning-works/

    2. http://whizzer.bu.edu/skills/security

## Supporting Artifacts


-----
