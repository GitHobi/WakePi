# WakePi
As this topic comes up over and over again, find here a prototype of a solution.

## What is it doing?
Assume you have a device which should not be powered up all the time - like Rasperry Pi :)
You want to be able to power up your device via standard wake on lan (WOL).
This sketch can help with this.

## Preparation
To enable "wake on lan" for your device, you need to power your device with a "smart" power outlet. 
Preferably you use some Tasmota enabled device, like a reflashed Sonoff Pow, or e.g. a Shelly-1.
By using this smart power outlet you can enable / disable the power for your device.

## How is it working
Function of this sketch is simple:
It is waiting for a WOL package for your device. YOu just need to grab a "virtual" Mac-Address, 
where you will send your WOL requests to. The sketch will capture them and once retrieved, connect 
to the smart power outlet and turn the power on.

## Used code
This sketch is heavilly based on this this post:
https://www.hackster.io/erkr/wake-on-lan-wol-gateway-a6a639

## Attention!
It seems that Wake on Lan is not always Wake on Lan!
This sketch works only with magic packages which are sent to UDP port 12287 - this seems to be somrt sort of "standard" in the windows world.
But this sketch will e.g. not work with "etherwake"!
