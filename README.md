# ESP32Subcounter
required libraries:
already available when using ESP32:

>Wifi.h
>HTTPClient.h

have to be installed extra:

>SevSeg.h
>ArduinoJson.h

connections:
> 7-seg display

<pre>
  14  2  5  27 26 0 
   ┇  |  |  ┇  ┇  |
|====================|
|     8. 8. 8. 8.    |
|====================|
   |  |  |  |  |  ┇  
  17 16  19 4 18  25
  
┇ = resistor ~220 Ω
</pre>
> RGB LED:
<pre>
  R   G   B
  |   |   |
  21  22  23
</pre>
> Touch: 
<pre>
[metal strip] - 32
</pre>

required settings:
<pre>
const char* ssid = "your WIFI name";
const char* password = "your WIFI pwd";
const String UID = "creator ID";
const String APIkey = "your YT API key";
</pre>
