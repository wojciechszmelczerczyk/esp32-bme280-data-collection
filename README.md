# ESP-32 and BME280

## Description

Project describe set up of `ESP-32` and `BME280` sensor. `BME280` sensor collect data about air metrics. It measures variety of parameters such as `Humidity`, `Temperature` and `Pressure` in the air and return those values. `ESP-32` intercept those air metrics and additionaly receive `Air Quality Index` value from [ESP-32 + MQ-135](https://github.com/wojciechszmelczerczyk/esp32-mq-135-data-collection). After sensors data complete, it is send to `Influxdb` time series database every 15 minutes.

## Techstack

- `Arduino`
