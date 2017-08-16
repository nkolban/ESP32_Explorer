## Purpose
The core notion is that the library will expose a Web Server that will serve up information about the operation
of the ESP32.

The ESP32 can be thought of as having multiple separate internal functional areas.  These include:

* WiFi
* GPIO
* File systems
* I2S

In addition there is the ESP32 "System" itself which includes memory management, flash partitions and more.

As an application that you may have written runs on the ESP32, we don't necessarily have good visibility into the state.
We can, of course, attach debuggers and other useful tools but they pretty much give us insight into the operation 
and logic of your own application, and not the environment in which your application runs.  This library can be linked
with your own application to present the information you might otherwise not see.