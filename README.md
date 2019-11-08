This daemon allows users to control PWM driven RGB LEDs using ubus.

Currently it is possible to make leds blink at different brightnesses and or make them fade in between colours.

the following call will turn green on and red off:
ubus call led set '{ "leds": { "ap:green:status": 255, "ap:red:status": 0 } }'

the following call will make green and red fade on/off over 2 seconds:
ubus call led set '{ "leds": { "ap:green:status": [0, 255], "ap:red:status": [255, 0] }, "on": 2000, "off": 2000, "fade": 1 }'

