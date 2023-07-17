#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// Influxdb cloud config data
#define INFLUXDB_URL ""
#define INFLUXDB_TOKEN ""
#define INFLUXDB_ORG ""
#define INFLUXDB_BUCKET ""

// Time zone info
#define TZ_INFO "UTC-2"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare Data point
Point sensor("environment");

// Wi-Fi credentials
const char* ssid = "";
const char* password = "";

// MQ135 data variables
int mqData = 0;

WiFiServer server(80);


// initialize the BME280 sensor object
Adafruit_BME280 bme;

void setup() {
  Serial.begin(115200);

  // establish wifi connection
  WiFi.begin(ssid, password);

  // check status, repeat if status is not connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  delay(2000);

  // print successful connection message
  Serial.println("Connected with WiFi!");
  Serial.print("IP adress: ");
  Serial.println(WiFi.localIP());

  // initialize the BME280 sensor
  if(!bme.begin(0x76)){
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while(1);
  }

  delay(2000);

  // print successful connection message
  Serial.println("BME280 setup correctly!");

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
    if (client.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }

  // Start the server
  server.begin();
}

void loop() {
  // Check for incoming client connections
  WiFiClient wifiClient = server.available();
  Serial.println(wifiClient);
  if (wifiClient) {
    // Wait for data from the client
    while (wifiClient.connected() && !wifiClient.available()) {
      delay(1);
    }

    // Read the HTTP request
    String request = wifiClient.readString();
    Serial.println(request);

    // Extract MQ135 data from the request
    int index = request.indexOf("mq_data=");
    if (index != -1) {
      String mqDataString = request.substring(index + 8);
      mqData = mqDataString.toInt();
    }

    // Close the connection
    wifiClient.stop();
  }

  float temperature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;
  float humidity = bme.readHumidity();

  // Create data point
  sensor.clearFields();
  sensor.addField("temperature", temperature);
  sensor.addField("pressure", pressure);
  sensor.addField("humidity", humidity);
  sensor.addField("air_index", mqData);
  
  Serial.println(sensor.toLineProtocol());


  // Write data point to InfluxDB
  if (client.writePoint(sensor)) {
    Serial.println("Data point sent to InfluxDB successfully");
  } else {
    Serial.print("Error sending data point to InfluxDB: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Print the sensor readings
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  delay(60000); // Wait for 1 minute before the next reading

}
