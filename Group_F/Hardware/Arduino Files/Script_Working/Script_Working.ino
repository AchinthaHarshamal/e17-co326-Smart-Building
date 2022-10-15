#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define arraySize 10
#define periodTime 100

const char* ssid = "Eng-Student";
const char* password = "3nG5tuDt";

const char* mqtt_server = "10.40.18.10";
const char* topic_voltageSensor = "326project/smartbuilding/pv/pvVoltage";
const char* topic_kwhmeterSensor = "326project/smartbuilding/pv/kWhmeter";
const char* topic_sw1 = "326project/smartbuilding/pv/controls/sw1";
const char* topic_sw2 = "326project/smartbuilding/pv/controls/sw2";

const int currentSensor = A0; // Defining LDR PIN 
const int relay = D7;
int input_val = 0;  // Varible to store Input values

float meterReading = 0;
float adjustedValue = 0;
float gradient = 5.0;
float offset = 2690;


int lastvalues[arraySize]= {}; //initialize all the values to zero
int tempnew = 0;
int templast = 0;

int maxVal = -32768;
int minVal = 32768;

//INT_MIN
//INT_MAX
int difference = 0;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(topic_voltageSensor);
      client.subscribe(topic_sw1);
      client.subscribe(topic_sw2);
      client.subscribe(topic_kwhmeterSensor);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(String topic, byte* message, unsigned int length) {
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

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic==topic_sw2){
      Serial.print("Changing Room lamp to ");
      if(messageTemp == "ON"){
        digitalWrite(relay, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "OFF"){
        digitalWrite(relay, LOW);
        Serial.print("Off");
      }
  }
  Serial.println();
}

void setup() {
 pinMode(relay, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);  // public
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
//Start of kWpMeter Sensor reading Function
  
  unsigned long now = millis();
  if (now - lastMsg > periodTime) { // publish every two seconds
      lastMsg = now;
       
      meterReading = analogRead(currentSensor);
      Serial.print("Sensor Vlaue = ");
      adjustedValue = meterReading*gradient - offset;
  
      for (byte i = 1; i <= arraySize; i = i + 1) {
        //a function to move values one down the array; 
        tempnew = lastvalues[arraySize-i];
        lastvalues[arraySize-i] = templast;
        templast = tempnew;
    }
    lastvalues[arraySize-1] = adjustedValue;

    Serial.println(adjustedValue);

    for (byte i = 0; i < 10; i = i + 1) {
      //takes the max and min values in the array
      maxVal = max(lastvalues[i],maxVal);
      minVal = min(lastvalues[i],minVal);

      //a function to print the array values;
      Serial.print(i);
      Serial.print(" = ");
      Serial.print(lastvalues[i]);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("Max Value = ");
    Serial.print(maxVal);
    Serial.print("  Min Value = ");
    Serial.print(minVal);
    difference = maxVal - minVal;
    Serial.print("  Difference = ");
    Serial.print(difference);

  
    //making maxVal and MinVal zero for the next loop
    maxVal = 0;
    minVal = 0;

    Serial.println();
//End of kWpMeter Sensor reading Function
  
//    input_val = analogRead(LDR);      // Reading Input
//    snprintf (msg, MSG_BUFFER_SIZE, "%d", input_val);
//    Serial.print("Publish message: ");
//    Serial.println(input_val);
//    client.publish(topic_voltageSensor, msg);
  }
}
