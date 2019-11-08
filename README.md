# μledd

[![LGPL-2.1-only license][license-badge]][license-ref]
[![CI][ci-badge]][ci-ref]

## About

μledd is OpenWrt's micro daemon which allows users to control PWM driven RGB LEDs using ubus.  Currently it is possible
to make LEDs blink at different brightnesses and/or make them fade in/out between colours.


## Install

μledd is [available](https://github.com/openwrt/packages/tree/master/utils/uledd) via [OpenWrt package feeds](https://github.com/openwrt/packages/), so the installation should be straight forward.

```shell
opkg install uledd
```

## Usage examples

### Turn green LED on and red LED off

```json
ubus call led set '{ "leds": { "ap:green:status": 255, "ap:red:status": 0 } }'
```

### Make green LED and red LED fade on/off over 2 seconds:

```json
ubus call led set '{ "leds": { "ap:green:status": [0, 255], "ap:red:status": [255, 0] }, "on": 2000, "off": 2000, "fade": 1 }'
```

[ci-badge]: https://gitlab.com/ynezz/openwrt-uledd/badges/master/pipeline.svg
[ci-ref]: https://gitlab.com/ynezz/openwrt-uledd/commits/master
[license-badge]: https://img.shields.io/github/license/blogic/uledd.svg?style=flat-square
[license-ref]: LICENSE
