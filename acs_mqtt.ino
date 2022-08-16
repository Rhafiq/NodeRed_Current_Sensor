/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "YOUR_MQTT_BROKER_IP_ADDRESS";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//uncomment the following lines if you're using SPI
/*#include <SPI.h>
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5*/

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
float temperature = 0;
float humidity = 0;

// LED Pin
const int ledPin = 4;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/////////////*/
//ACS712
        /* 0- General */

        int decimalPrecision = 2;                   // decimal places for all values shown in LED Display & Serial Monitor


        /* 1- AC Current Measurement */

        int CurrentAnalogInputPin = A1;             // Which pin to measure Current Value (A0 is reserved for LCD Display Shield Button function)
        float mVperAmpValue = 185;                  // If using ACS712 current module : for 5A module key in 185, for 20A module key in 100, for 30A module key in 66
                                                    // If using "Hall-Effect" Current Transformer, key in value using this formula: mVperAmp = maximum voltage range (in milli volt) / current rating of CT
                                                    // For example, a 20A Hall-Effect Current Transformer rated at 20A, 2.5V +/- 0.625V, mVperAmp will be 625 mV / 20A = 31.25mV/A 
        float offsetSampleRead = 0;                 /* to read the value of a sample for offset purpose later */
        float currentSampleRead  = 0;               /* to read the value of a sample including currentOffset1 value*/
        float currentLastSample  = 0;               /* to count time for each sample. Technically 1 milli second 1 sample is taken */
        float currentSampleSum   = 0;               /* accumulation of sample readings */
        float currentSampleCount = 0;               /* to count number of sample. */
        float currentMean ;                         /* to calculate the average value from all samples, in analog values*/ 
        float RMSCurrentMean ;                      /* square roof of currentMean, in analog values */
        float adjustRMSCurrentMean ;                /* RMScurrentMean including currenOffset2, in analog values */    
        float FinalRMSCurrent ;                     /* the final RMS current reading*/


            /*1.1 Offset AC Current */
    
            float currentOffset1 = 0;                   // to Offset deviation and accuracy. Offset any fake current when no current operates. 
                                                        // Offset will automatically callibrate when SELECT Button on the LCD Display Shield is pressed.
                                                        // If you do not have LCD Display Shield, look into serial monitor to add or minus the value manually and key in here.
                                                        // 26 means add 26 to all analog value measured
            float currentOffset2 = 0;                   // to offset value due to calculation error from squared and square root.
      

void setup() {
  Serial.begin(9600);
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  //status = bme.begin();  
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);

}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    // Temperature in Celsius
    temperature = bme.readTemperature();   
    // Uncomment the next line to set temperature in Fahrenheit 
    // (and comment the previous temperature line)
    //temperature = 1.8 * bme.readTemperature() + 32; // Temperature in Fahrenheit
    
    // Convert the value to a char array
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("esp32/temperature", tempString);

    humidity = bme.readHumidity();
    
    // Convert the value to a char array
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("esp32/humidity", humString);

    //ACS712
    /* 1- AC Current Measurement */
        
        if(millis() >= currentLastSample + 1)                                                                 /* every 1 milli second taking 1 reading */
          {
           offsetSampleRead = analogRead(CurrentAnalogInputPin)-512;                                          /* Read analog value. This is for offset purpose */ 
           currentSampleRead = analogRead(CurrentAnalogInputPin)-512 + currentOffset1;                        /* read the sample value including offset value*/
           currentSampleSum = currentSampleSum + sq(currentSampleRead) ;                                      /* accumulate total analog values for each sample readings*/
           
           currentSampleCount = currentSampleCount + 1;                                                       /* to count and move on to the next following count */  
           currentLastSample = millis();                                                                      /* to reset the time again so that next cycle can start again*/ 
          }
        
        if(currentSampleCount == 1000)                                                                        /* after 1000 count or 1000 milli seconds (1 second), do this following codes*/
          {
            currentMean = currentSampleSum/currentSampleCount;                                                /* average accumulated analog values*/
            RMSCurrentMean = sqrt(currentMean);                                                               /* square root of the average value*/
            adjustRMSCurrentMean = RMSCurrentMean + currentOffset2;                                           /* square root of the average value including offset value */
            FinalRMSCurrent = (((adjustRMSCurrentMean /1024) *5000) /mVperAmpValue);                          /* calculate the final RMS current*/
            Serial.print(" The Current RMS value is: ");
            Serial.print(FinalRMSCurrent,decimalPrecision);
            Serial.println(" A ");
            currentSampleSum =0;                                                                              /* to reset accumulate sample values for the next cycle */
            currentSampleCount=0;                                                                             /* to reset number of sample for the next cycle */
          }

  }
}
