# JAFaR (Just Another Fatshark Receiver Module)

DIY OpenSource 40ch receiver OSD integrated

I'm still writing this documentation, for constant updates please refer to http://www.rcgroups.com/forums/showthread.php?t=2619124 while I'm still writing the readme :)

The aim of the project is to build an opensource 40ch receiver for Fatshark goggles which has to be easy to develop, 
upgrade and maintain. And, of course, it must be cheap!
<p align="center">
<img src="/docs/diversity_goggles.jpg" width="50%" height="50%" />
</p>

technical details:
* 40ch (8 channels for 5 bands, including race band)
* no external switch, or hall effect sensor, or buzzer, it must use __only__ the goggles buttons (up-down)
* auto search feature
* it should include the “scanner” feature (see my recent project here https://github.com/MikyM0use/OLED-scanner)
* frequency selection via OSD inside the goggles
* I tested it only with Fatshark HD but the module should work with every fatshark with the RX external module (like Dominator V1, V2, V3, HD, HD2...)

This project uses the “rx5808” receiver module, which can be hacked to be SPI-programmed by a microcontroller (more details later).
So what I needed is a micro controller with an easy and well-know software IDE… nothing better than Arduino. So I decided to use an Atmega 328p as micro controller.

To select the frequency there's a series of menus on the screen inside the goggles, the useful TVOUT Arduino library is perfect and requires just a couple of resistors ([url]http://playground.arduino.cc/Main/TVout[/url]). A digital switch (TS5A3359) allow to switch from OSD to the normal receiving video from the quadcopter.

The project is mainly divided into two PCBs: 
* one main base module 
* one "diversity" daughterboard

##Main module

the main module is usable alone as a 40ch receiver module, but if you connect the second external module, 
the microcontroller onboard can decide which one to use (so-called "diversity") based on the received signal strength. Indeed, this RX5808 modules has an analog output called RSSI proportional on the "quality" of the signal.
on the base module the frequency is set by the user using an OSD menu.

<p align="center">
<img src="/docs/base_front.jpg" width="50%" height="50%" />
<img src="/docs/base_back.jpg" width="50%" height="50%" />
</p>

__MAIN MODULE (V4.0) BOM__

https://docs.google.com/spreadsheets/d/1CrZzOQHyK_d1VfHCQKwdNuoyMji0IYryI1VuffojdGA/edit?usp=sharing

__DIY Instructions__

stages to success:
*1) follow the instructions on the page https://www.arduino.cc/en/Tutorial/ArduinoISP to program the module (select "arduino micro" as target board)
*2) after the burn of the bootloader, use the FTDI connections in the upper right corner of the board to flash the code present in the Github page of the project
*3) the very first time you power on the module, it need to calibrate itself. So please turn on the module with a Vtx a couple of meter away (the video frequency doen't matter). This process can take a while (about 30-50 seconds).

*4) when you power on the module, BOTH the leds (upper left corner of the pcb) must be on, and you must see a relatively stable splash screen for about 5 seconds
*5) you can now use the "selection" buttons on the goggles to scroll up/down the menu to select the band
*6) after the timout of the countdown (upper left corner of the screen) you entered the frequency selection (of the selected band)
*7) scroll up/down to select the frequency and wait the timeout

*8) at this moment only one led on the module has to be ON, it means that the RX video output is routed to the video_input of the goggles. If you have a Vtx near, you should see the video coming from it. 

*9) if you press the "selection" buttons of the goggles now, you change the frequency by the 8 frequencies of the previously selected band.

note ONLY FOR 3.2 version of the PCB: the PCB allow the use of a capacitor OR a resistor on the video out. DON'T use both!

__Troubleshooting__

after the initial calibration, the module must turn on in couple of seconds and you must see the splash screen for some seconds before the "band selection" screen.
In the "band selection" screen you can see the 5 bands with the percentage of the maximum RSSI detected on that band. At least one of the band must be at 98-100%. If you "enter" one band, every channel must shows a different percentage of RSSI, otherwise the SPI-mod of the module couls be failed.


##Diversity module

The diversity module is an optional second receiver, connected to the "main" base module, it has:
*integrated linear 5v regulator with ENABLE feature connected to the VCC of the main module. In fact power a second module from the internal goggles voltage regulator IS VERY DANGEROUS!
*an optinal operational buffer for RSSI signal (I noticed that lately modules seems to be more delicate)

<p align="center">
<img src="/docs/diversity_front.jpg" width="50%" height="50%" />
<img src="/docs/diversity_back.jpg" width="50%" height="50%" />
<p>

please notice that because of the linear regulator it's not possibile to use a 3s battery to power the diversity module at the moment.

__DIVERSITY MODULE (V2.1) BOM__

https://docs.google.com/spreadsheets/d/1OpagubuxicmGTKqOHWwWdxZxY6wCs_DZqTYRxVoD4Tc/edit?usp=sharing

__DIY Instructions__

TODO

##Reference thread 

http://www.rcgroups.com/forums/showthread.php?t=2619124 while I'm still writing the readme :)

##FAQ:

why the OSD is on the blank screen?
it’s simpler because one cannot have to recognize the vsync and the hsync pulses.

why the switch is so small and hard to solder?!?
I need a small 3-way switch, if you have a "simpler" one, just tell me :)

please note that you try and use this project AT YOUR OWN RISK!

##Donations:

Hardware or paypal donations to carry on the project are always accepted.

[![paypal](https://www.paypalobjects.com/it_IT/IT/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=YKP2PH72RKPRY)
