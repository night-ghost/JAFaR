# JAFaR (Just Another Fatshark Receiver Module)

DIY OpenSource 40ch receiver OSD integrated

## This is a major refactor of MikyM0use's original code

**Changes**
* Has a favorites section which remembers the last 8 channels used ordered with the last used at the top of the list. It re-orders the list when you select a new channel.
* The scanner is continuously running in the main menu or scanner menu. This means that the RSSI values are pretty much always up to date.
* There's a timer progress bar on the right rather than a counter.
* Double-click (i.e. another press within 1 second) takes you back to the main menu.
* When in a band selection menu, favorites or autoscan a single click of the buttons selects the next or previous channel respectively.

## MikyM0use's original README follows

The aim of the project is to build an opensource 40ch receiver for Fatshark goggles which has to be easy to develop,
upgrade and maintain. And, of course, it must be cheap!
<p align="center">
<img src="/docs/diversity_goggles.jpg" width="50%" height="50%" />
</p>


* 40ch (8 channels for 5 bands, including race band)
* No external switch, or hall effect sensor, or buzzer, it must use __only__ the goggles buttons (up-down)
* Auto search feature
* It should include the “scanner” feature (see my recent project here https://github.com/MikyM0use/OLED-scanner)
* Frequency selection via OSD inside the goggles
* I tested it only with Fatshark HD but the module should work with every Fatshark with the RX external module (like Dominator V1, V2, V3, HD, HD2...)

This project uses the “RX5808” receiver module, which can be hacked to be SPI-programmed by a microcontroller (more details later).
So what I needed is a micro controller with an easy and well-know software IDE… nothing better than Arduino. So I decided to use an Atmega 328p as micro controller.

To select the frequency there's a series of menus on the screen inside the goggles, the 
useful TVOUT Arduino library is perfect and requires just a couple of resistors 
(http://playground.arduino.cc/Main/TVout). 
A digital switch (TS5A3359) allow to switch from OSD to the normal receiving video from the quadcopter.

The project is mainly divided into two PCBs: 
* one main base module 
* one "diversity" daughterboard (optional)

it's also possibile to use an SPI-OLED display and use the module as standalone RX.

##Table of contents
* [Main module](#main-module)
	* [DIY Instructions](#diy-instructions)
	* [MAIN MODULE Troubleshooting](#main-module-troubleshooting)
* [Diversity module](#diversity-module)
* [OLED module](#oled-module)
* [STANDALONE mode](#standalone-mode)
* [Reference thread](#reference-thread)
* [FAQ:](#faq)
* [Donations:](#donations)


##Main module

This is the main RX module, usable alone as a 40ch receiver module, but if you connect an 
"external" module, the onboard microcontroller can decide which RX module has better 
reception (so-called "diversity") based on the received signal strength. Indeed, this 
RX5808 modules has an analog output called RSSI proportional on the "quality" of the 
received video signal. The receiving video channel (frequency) is set by the user using an OSD menu.

<p align="center">
<img src="/docs/base_front.jpg" width="50%" height="50%" />
<img src="/docs/base_back.jpg" width="50%" height="50%" />
</p>

__MAIN MODULE (V4.0) BOM__

https://docs.google.com/spreadsheets/d/1CrZzOQHyK_d1VfHCQKwdNuoyMji0IYryI1VuffojdGA/edit?usp=sharing

__LEGACY MAIN MODULE (V3.2)__

This module is actually the same as the v4.0, but it doesn't have the buffer on the RSSI output, 
and the PCB allows the use of a capacitor OR a resistor on the video out. DON'T use both! after some
tests I realized that a 10ohm resistor (and no cap) is enough.

<p align="center">
<img src="/docs/base_front_32.jpg" width="50%" height="50%" />
</p>

(the back side is the same of the v4.0).

__LEGACY MAIN MODULE (V3.2) BOM__

https://docs.google.com/spreadsheets/d/1-763imBV3QsQ71GKDZH-BirTz1k32szNuJZUKQrE00g/edit?usp=sharing

###DIY Instructions

1. Remove the shield of the RX5808 module and desolder the resistor marked by the arrow. This 
is the SPI-mod required to control the module with an external microcontroller.

<p align="center">
<img src="/docs/rx5808mod.jpg" width="35%" height="35%" />
</p>

2. Follow the instructions on the page https://www.arduino.cc/en/Tutorial/ArduinoISP to burn the bootloader (select __"Arduino Pro Mini 5v 16MHz"__ as target board)
using the pins in the upper right corner of the board

3. Use the FTDI connections in the left side of the board to upload the code present in the 
Github page of the project. Please remember to copy all the subfolders of "libs" dir in the 
libraries folder of Arduino (e.g. /Arduino/libraries).
Here you can find more infos and alternative methods to import the libraries: 
https://www.arduino.cc/en/Guide/Libraries

4. When you power on the module, BOTH the leds (upper left corner of the pcb) must be on, and you must see a relatively stable splash screen for about 5 seconds

5. The very first time you power on the module, it need to calibrate itself. So please turn on the module with a working Vtx a couple of meter away (the video frequency doesn't matter). This process can take a while (about 30-50 seconds).

6. You can now use the "selection" buttons on the goggles to scroll up/down the menu to select the band

7. After the timout of the countdown (upper left corner of the screen) you entered the frequency selection (of the selected band)

8. Scroll up/down to select the frequency and wait the timeout

9. At this moment only one led on the module has to be ON, it means that the RX video output is routed to the video_input of the goggles. If you have a Vtx near, you should see the video coming from it. 

10. If you press the "selection" buttons of the goggles now, you change the frequency by the 8 frequencies of the previously selected band.

###MAIN MODULE Troubleshooting

After the initial calibration, the module must turn on in couple of seconds and you must see the splash screen for some seconds before the "band selection" screen.
In the "band selection" screen you can see the 5 bands with the percentage of the maximum RSSI detected on that band. At least one of the band must be at 98-100%. 
If you "enter" one band, every channel must shows a different percentage of RSSI, otherwise the SPI-mod of the module could be failed.

__Unable to flash/Aduino issues:__

(thanks to pHysiX for the help in this section!)
Got the error "no reply" when trying to load the firmware:
the ATmega microcontroller is not soldered properly / there is a short on some pins.
try a quick solder, clean and reflow.

Got the error "incorrect signature" when trying to load the firmware:
This means Arduino is able to pick up the ATMega, but it was not communicating as expected. 
This is related to poor soldering of the crystal. It's possible to do the crystal using 
an iron, but I strongly recommend using flow method/hot air to do this, to make sure you 
don't short the three legs accidentally onto the metal case.

__Display issues:__

They are mostly related to poor soldering for the OSD components on the board. Go back and 
reflow especially the Switch, and then just make sure to clean up and touch up all the caps 
and resistors near and around the switch.

If you have an external monitor, you can bypass the digital switch by power on the module with external 5v and connecting the "video in" of the monitor on the test point marked in yellow the picture:

<img src="/docs/video_test_points.jpg" width="50%" height="50%" />

and you should see the splash screen and the menu.

If you connect the "video in" of the monitor to the test point marked in green, you should see the initial scanning (used to determine the % of the RSSI on each channel), this means a sequence of different "fogs" and (at least) one "good" image (for a fraction of second), if you have a Vtx + cam transmitting. 

__Frequency issues:__

If the module is having trouble Setting Frequency, or you are not getting video when you are 
on the right channel and frequency, make sure the SPI mod has been done, and that the RX5808 
is soldered on well. Pretty hard to mess this part up.

##Diversity module

The diversity module is an optional second receiver, connected to the "main" base module, it has:
* integrated linear 5v regulator with ENABLE feature connected to the VCC of the main module. 
In fact power a second module from the internal goggles voltage regulator IS VERY DANGEROUS!
* an optional operational buffer for RSSI signal (I noticed that lately modules seems to be more delicate)

<p align="center">
<img src="/docs/diversity_front.jpg" width="50%" height="50%" />
<img src="/docs/diversity_back.jpg" width="50%" height="50%" />
</p>

please notice that because of the linear regulator it's not possibile to use a 3s battery to power the diversity module.

__DIVERSITY MODULE (V2.1) BOM__

https://docs.google.com/spreadsheets/d/1OpagubuxicmGTKqOHWwWdxZxY6wCs_DZqTYRxVoD4Tc/edit?usp=sharing


##OLED module
It's possible to use an external SPI OLED module in case of any problems with the internal OSD
or to use the module with an external monitor / different goggles.

<img src="/docs/oled_pic.jpg" width="50%" height="50%" />

It's important that the module has only SDA/SCL/RES/DC pins.

To use the OLED module instead of the internal OSD, uncomment  the line:

```
#define USE_OLED
```

in the file const.h

please remember to comment #define USE_DIVERSITY!

__Connections__

| ATMEGA328p pin        |  PCB silkscreen name           | OLED silkscreen name  |
| :-------------------:	|:------------------------------:| :--------------------:|
| VCC      				| 		EN 						| 		VCC 			|
| GND      				| GND 							| 					GND |
| 8      				| SSEL 							| 				SCL 	|
| A1      				| RSSI      					|   		SDA 		|
| A4 					|  NOT CONNECTED      			|    	NOT CONNECTED 	|
| 11 					| SDA      						|    DC 				|
| 13				 	| CLK      						|    RESET 				|

...or graphically:

<p align="center">
<img src="/docs/oled_connections.jpg" width="35%" height="35%" />
</p>

there's also a "PCB" inside the PCBs folder, but it's just a simple adaptor.

##Standalone mode

It's possible to use the project also as a standalone receiver to be used in combination
with any external monitor or other kind of goggles.

In this case you must uncomment the line

```
#define STANDALONE
```

in the file const.h. You can use the standalone mode either with OSD or with the external SPI OLED module.

to change band/channel, the connections are the following (every normally-open momentary switch will do the job):

<p align="center">
<img src="/docs/standalone_connections.jpg" width="60%" height="60%" />
</p>



##Reference thread 

http://www.rcgroups.com/forums/showthread.php?t=2619124 while I'm still writing the readme :)

##FAQ

why the OSD is on the blank screen?
it’s simpler because one cannot have to recognize the vsync and the hsync pulses.

why the switch is so small and hard to solder?!?
I need a small 3-way switch, if you have a "simpler" one, just tell me :)

please note that you try and use this project AT YOUR OWN RISK!

##Donations

Hardware or paypal donations to carry on the project are always accepted.

[![paypal](https://www.paypalobjects.com/it_IT/IT/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=YKP2PH72RKPRY)
