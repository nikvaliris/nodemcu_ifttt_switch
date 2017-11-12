# nodemcu_ifttt_switch
IFTTT integrated switch using the NodeMCU (ESP8266)

Been busy building some WiFi smart switches using the small cheap Nodemcu boards.  I use it to trigger themes and was < £10 in parts.  It uses IFTTT which works with Samsung Smartthings and many other other things to trigger events.  It's a small compact unit and can support up to 6 switches and just needs a micro USB to power it.

My test device has been working flawlessly for a month now and handle Wi-Fi outages nicely by reconnecting.

Instructions:

1. You need to setup IFTTT Webhook first and connect to Smartthings, add some theme/event names for actions to be trigged and test
2. Once the simple switch is built (wiring diagram I'll busy doing now)
3. NodeMCU Needs initially flashed with my code.  Grab the bin file and Arduino flashing software and flash the nodeMCU (ESP8266).  Alternatively download the Arduino software and paste the code and plug the switch into the PC and flash it that way.  Details on using that software with the NodeMCU boards is available on other sites.
4. When powered on after flashing and build the board should power and present a WiFi SSID hotspot which you can connect to via a browser or phone to set up the switch. The default SSID will be ESPxxxx and password 12345678. The URL for the browser page is 192.168.4.1.
5.  Once the pages is up you can set:
   - IFTTT key taken from the Webhook
   - Set theme/event names and name of swtich to display on the OLED display
   - Set home Wi-Fi credentials / password you wish the switch to use
   - You can even reset the default switch SSID name and password for multiple switches in the house.  
   - Each switch can be upgraded later over WiFi but clicking firmware upgrade button with no need to connect to a computer.  It picks up the lastest firmware from my server.  So really simple for upgrades no PC or flashing required.
   - Restart switch to apply changes,  OLED screen will display if its been successful in connecting and Wifi on initial screen.
   
[note I'll do proper build instructions in a bit]
