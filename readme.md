# Welcome to the Autonomous Greenhouse Project with Arduino

## Presentation

The goal of this project is to build a self-sustaining home greenhouse using an Arduino module. This greenhouse permanently probes the temperature, the ambient humidity, the humidity of the earth and the water level of the tank, and triggers the watering or the ventilation if necessary according to pre-established rules.
This greenhouse is connected to Cayenne, an application available on iOS and Android that allows to quickly and easily view the data sent by the Arduino module, and also allows to send commands.

## Equipment used

- 1 Wemos D1 mini ESP8266
- 1 Humidity and temperature sensor (DHT11)
- 1 Moisture sensor in the soil
- 1 water level sensor (float)
- 2 fixed computer fans
- 1 laptop power supply
- 2 LED strips
- 1 relay module (4 channel)
- 1 test plate
- 1 water pump (type Château de Versailles)
- 1 plastic hose for watering (2 meters)
- 1 Water Tank
- 1 transparent plastic box large enough

__Total: 200 € __

## Electrical Installation Diagram

! [Electrical Mounting] (media / montage.png)

## Rules of operation

The autonomous greenhouse is regulated according to certain criteria. For light, it is advisable to illuminate the greenhouse 12 hours a day. The watering is done according to the humidity in the ground and according to the ambient humidity. Finally the ventilation is done if the humidity or the temperature is too high in the greenhouse.

The thresholds used in our case study (Basilisk):

- Maximum humidity in summer: 60%
- Maximum humidity in winter: 80%
- Minimum all-season humidity: 20%
- Maximum temperature of the greenhouse: 25 ° C
- Minimum humidity of the earth: 20%
- Maximum humidity of the earth: 80%
- Start of the day: 8H
- End of the day: 20h


## Tutorial

Cut out the plastic crate on the widths, circle shape and body size of the fans. Cut out a small access hatch at the base of one of the lengths to insert the sensor wires and lights, as well as the garden hoses.

Attach the fans so that one bladder and the other blows air into the greenhouse to circulate it.

Fix the LED rails connected in series on the top of the plastic box, inside.

Attach the humidity and temperature sensor to a wall of the crate inside.

Prepare the moisture sensor in the earth so.

Fix the different cables cleanly inside the greenhouse and bring them to the hatch at the base of the box.

Outside the box, attach the relay card and the Wemos. Prepare a container in which will be fixed a float, at the bottom. This container will also receive the pump.

Follow the wiring diagram precisely to make the connections. Using the Arduino software, upload the following ino script, adding in advance your Cayenne credentials and your internet access credentials.
To make the script work, in Library Manager it is necessary to add:
- DHT Sensor Library
- Adafruit Unified Sensor
- NTPClient
- Cayenne MQTT for ESP

The last is at this address [GitHub](https://github.com/myDevicesIoT/Cayenne-MQTT-ESP.git)

## Feedback from Experience

With so many components, a calculation of power requirements is very important. We thought we had enough energy to power the sensors and light for example with the module Wemos but no. Then we put the light on a relay card but this time there was too much power, we burned the lamps ...
Finally, if there is use of a relay card like us, you have to be very careful about the direction of connection of the electric current.

## Some pictures

![Electrical Mounting](media/1.jpg)

![Electrical Mounting](media/2.jpg)

![Electrical Mounting](media/3.jpg)

![Electrical Mounting](media/4.jpg)

![Electrical Mounting](media/5.jpg)