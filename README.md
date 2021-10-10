# WakePi
How to remotely turn on a Raspberry Pi?
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
This sketch works only with magic packages which are sent to UDP port 12287 - this seems to be some sort of "standard" in the windows world.
But this sketch will e.g. not work with "etherwake"!

# WakePi - in a "standard" way

As mentioned above, the sketch is only suitible to wake a device with WOL sent via broadcast, based on TCP/IP. 
If you e.g. want to use a FRITZ!Box to start your Pi, this will not work, as the FRITZ!Box actually sends a WOL package with a different ethernet protocol. 
To deal with this situation, you can use a small python script:

```python
import socket, sys
import requests
from struct import *

#Convert a string of 6 characters of ethernet address into a dash separated hex string
def eth_addr (a) :
  b = "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x" % (a[0] , a[1] , a[2], a[3], a[4], a[5])
  return b

#create a AF_PACKET type raw socket (thats basically packet level)
#define ETH_P_ALL    0x0003          /* Every packet (be careful!!!) */
try:
        s = socket.socket( socket.AF_PACKET , socket.SOCK_RAW , socket.ntohs(0x0003))
except socket.error as msg:
        print ('Socket could not be created. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])
        sys.exit()

# receive a packet
while True:
        packet = s.recvfrom(65565)

        #packet string from tuple
        packet = packet[0]

        #parse ethernet header
        eth_length = 14

        eth_header = packet[:eth_length]
        eth = unpack('!6s6sH' , eth_header)
        eth_protocol = socket.ntohs(eth[2])
        #if ( eth_protocol == 16904):
        #  print ('Destination MAC : ' + eth_addr(packet[0:6]) + ' Source MAC : ' + eth_addr(packet[6:12]) + ' Protocol : ' + str(eth_protocol))
        #  for i in range(40):
        #         v = "%.2x " % (packet[i])
        #         print ( v, end='' )
        #  print ()

        #Parse IP packets, IP Protocol number = 8
        if eth_protocol == 16904:
                #Parse IP header
                #take first 20 characters for the ip header
                mac_destination = packet[0:6]
                mac_source      = packet[6:12]
                protocol = packet[12:14]
                sync_stream = packet[14:20]
                mac_wol = packet[20:26]
                print ('Destination MAC : ' + eth_addr(mac_destination) + ' Source MAC : ' + eth_addr(mac_source) + ' Protocol : ' + str(protocol) + ' Target MAC : ' + eth_addr(mac_wol))

                if (mac_wol[0] == 0xE4) and (mac_wol[1] == 0x5F) and (mac_wol[2] == 0x01) and (mac_wol[3] == 0x54) and (mac_wol[4] == 0x1D) and (mac_wol[5] == 0xE1):
                        print ( "WOL Signal detected ...")
                        response = requests.get("http://192.168.20.100/cm?cmnd=Power%20on")```

```
This script is basically sniffing for all packets - regardless if TCP/IP, UDP, ... but it is waiting for packages with protocol "16904" (WOL). If such a package comes in, it compares the destination MAC. If the MAC is as expected, a web request to power the smart plug is issued ...
This approach also would work with eatherwake!
So far however I did not find a reasonable way to implement this on a ESP8266.

