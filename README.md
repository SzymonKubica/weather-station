# Weather Station

This repository contains the code and instructions required to build a simple
weather station powered by an esp32 microcontroller, a DHT22 temperature sensor
and an infrared receiver.

It uses esp-idf framework for managing the system tasks and open-meteo api for
sourcing the weather data.

## Build Instructions

### Required components

- LOLIN32 Lite microcontroller (like this one [here](https://www.amazon.co.uk/AZDelivery-Lolin-ESP32-Parent/dp/B086V1P4BL?th=1))
- DHT22 temperature and humidity sensor (see [here](https://www.switchelectronics.co.uk/products/dht22-digital-temperature-and-humidity-sensor-module?variant=45334947561781&currency=GBP&utm_medium=product_sync&utm_source=google&utm_content=sag_organic&utm_campaign=sag_organic&gclid=Cj0KCQjw9fqnBhDSARIsAHlcQYTC_RPZrCdsrJcMWFUb5oEbM2ZXBoalp3reoWo0Z95ebyMr20l00vMaAtPtEALw_wcB))
- infrared remote and receiver (example [here](https://www.ebay.co.uk/itm/154724697624?chn=ps&_trkparms=ispr%3D1&amdata=enc%3A1ZrJC8Xh7S1O-VXLV_YROyg39&_ul=GB&norover=1&mkevt=1&mkrid=710-134428-41853-0&mkcid=2&mkscid=101&itemid=154724697624&targetid=1647205088320&device=c&mktype=pla&googleloc=9045888&poi=&campaignid=17206177401&mkgroupid=136851690655&rlsatarget=pla-1647205088320&abcId=9300866&merchantid=113112059&gclid=Cj0KCQjw9fqnBhDSARIsAHlcQYSEzWP9kBIrCusp_EtkQ7L0MUGv69uRiiCi-RFfC8OKJTxTwdI5lzMaApxjEALw_wcB))
- small breadboard
- jumper wires
- some batter compatible with the microcontroller.

### Wiring diagram

TODO

### Building the code

First you need to get the esp-idf framework set up on your machine. You can find
instuctions on how to do it [here](). After that you need to set the wifi
password for the project using the command:

```
idf.py menuconfig
```

For convenience I recommend aliasing idf.py to idf in you shell config.
After you've done that, the project can be built, flashed onto the board and
started to be monitored using the following command:

```
./run
```

In case you want to modify the code and submit a pull request, make sure to
have clangd installed and run the formatting script before submitting the code
to avoid code style merge conflicts.

```
./reformat
```

