# ESP32_ulp_blink

Blink an LED blink using the ESP32's ULP, after putting the main core in deep sleep mode
I couldn't find a simple example, so I cobbled this together.
 
Bonus: on some ESP32 boards, the on-board (blue) LED is on GPIO16. If GPIO2 is tied to GPIO16, 
it reduces the component count and an external LED isn't required. As GPIO16 is not in 
the RTC register, the on-board LED cannot otherwise be used by the ULP.
