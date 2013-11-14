This project is meant to create an Arduino weather station using the DHT21 Temperature & Humidity Sensor, a 0.2mm per switch closure Rainguage.
Ultimately additional Temperature sensors along with wind speed and direction and perhaps Barometric pressure will be added.
Water tank level may also be added to look at Rainfall vs tank level.
All of the values are uploaded using an HTTP GET command to a cloud based data platform called nimbits.

I am using a Freetronics Etherten that has an ATMega328 and Wiznet2100 Ethernet interface.
The 2k of RAM seems to be an issues as well as making reliable connections to Nimbits.