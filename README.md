# ToiletStatus
Advanced toilet sensor with battery powered node(s) and a gateway. 

## Nodes
As nodes I reuse my pcbs used in this project: https://github.com/stremlau/uniflaechentaster-pcb

Software is in folder *simple-toilet-node*.

## Gateway
A ESP8266 connected with a NRFL24 is used. See https://www.mysensors.org/build/connect_radio#nrf24l01+-&-esp8266 for wiring.

Use the Arduino IDE to upload the software in *esp-gateway-webinterface* using http://esp8266.github.io/Arduino/versions/2.0.0/doc/installing.html.

In addition you need to upload the content of the /data folder as described in:
http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html#uploading-files-to-file-system

Start your ESP, you should see a network called "configMe". Connect to it and open http://192.168.4.1/settings.html in your browser. Insert your SSID and password and click "Save". You can see your connection status in the "Status" section.
