#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>


#define Unknown_Mode 1
#define Send_Mode 2
#define Receive_Mode 3


/************** WiFi settings **************/
const char* ssid;
const char* password;

/************** MQTT Broker settings **************/
const char* mqtt_server;
const char* mqtt_username = "car1";        //"GP_Car_Network";
const char* mqtt_password = "car1";        //"G@2426579722#p";

const int mqtt_port = 8883;

/************** User Info **************/
const char* user_ID = "car1";  //"ESP8266Client_1";
const char* Subscribed_Topic = "test";


// WiFiClientSecure espClient;
WiFiClient espClient;
PubSubClient client(espClient);

char DataByte_Received_UART = 0;

char published_message_buffer[512] = { 0 };
int message_index = 0;
char publish_flag = 0;

int Flag_Mode = Unknown_Mode;

/************** Connect ESP8266 To Wifi Network **************/
void setup_wifi(void) {
  /* Delay 10 Milli Seconds */
  delay(10);

  /* Set Wifi In Station Mode */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   /* Tring To reconnect To Wifi Network In 500 Milli Seconds */
  //   delay(500);
  // }
  delay(2000);

  if (WiFi.status() != WL_CONNECTED) {
    delay(3000);
    // WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  }
}


/************** Blocking Reconnect To MQTT Broker in case it can't connect **************/
void Client_Reconnect_To_MQTT_Broker(void) {
  /* Loop until we're reconnected */
  while (!client.connected()) {
    /* Attempting to connect */
    if (client.connect(user_ID, mqtt_username, mqtt_password)) {
      /* Subscribe the topics here */
      client.subscribe(Subscribed_Topic);
    } else {
      /* Trying Again In 5 Seconds */
      delay(5000);
    }
  }
}


/************** CallBack When receiveng New Messages To Subscribed Topics **************/
void callback(char* topic, byte* payload, unsigned int length) {
  String Incomming_Message = "";

  if ((Flag_Mode == Unknown_Mode) || (Flag_Mode == Send_Mode)) {
    Flag_Mode = Receive_Mode;
  } else {
    /* Send The Incomming Message To UART In case Of Receive Mode */
    Incomming_Message.clear();

    Flag_Mode = Receive_Mode;

    for (int i = 0; i < length; i++) Incomming_Message += (char)payload[i];

    Serial.println(Incomming_Message);
  }
}


/************** void setup **************/
void setup() {
  

  while(Serial.available() > 0) Serial.read();

  Serial.flush();
  
  /* Set Baud Rate */
  Serial.begin(115200);
  
  while(Serial.available() == 0);

  setup_wifi();


  //espClient.setInsecure();
  /*if (espClient.setFingerprint(FingerPrint))
  {
    pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  }*/

  /*setClock();
  X509List cert(IRG_Root_X1);
  espClient.setTrustAnchors(&cert);
  */

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Client_Reconnect_To_MQTT_Broker();

  Serial.print('R');

  delay(300);

  Serial.flush();

  while(Serial.available() > 0) Serial.read();
}


/************** void loop **************/
void loop() {

  if (!client.connected()) {
    /* Set Flag Mode */
    Flag_Mode = Receive_Mode;
    
    Client_Reconnect_To_MQTT_Broker();
  }


  Flag_Mode = Receive_Mode;


  client.loop();

  /* Initalize String Buffer To Hold The Published Message once */
  // String Published_Message_Buffer = "";



  // if (Serial.available() > 0) {
  //   delay(1);

  //   /* Buffer The Inputs While Still Receving New Char */
  //   do {
  //     DataByte_Received_UART = Serial.read();

  //     Published_Message_Buffer += DataByte_Received_UART;

  //     delayMicroseconds(50);

  //   } while (Serial.available() > 0);
  // }
  while (Serial.available() > 0 && message_index < 512) {

    // DataByte_Received_UART = Serial.read();

    published_message_buffer[message_index] = Serial.read();

    message_index++;

    if (published_message_buffer[message_index - 2] == '\r' && published_message_buffer[message_index - 1] == '\n' && published_message_buffer[message_index] == '\0') {
      publish_flag = 1;
      break;
    }
  }


  // /* Send The Message While The Buffer Isn't Empty */
  // if (!Published_Message_Buffer.isEmpty()) {
  //   /* Set Flag Mode */
  //   Flag_Mode = Send_Mode;

  //   client.publish(Subscribed_Topic, String(Published_Message_Buffer).c_str(), true);

  //   /* Clear The Buffer To Hold A New Message */
  //   Published_Message_Buffer.clear();
  // }
  if (publish_flag == 1) {
    Flag_Mode = Send_Mode;
    client.publish(Subscribed_Topic, (const unsigned char*)published_message_buffer, message_index, true);

    for (int i = 0; i <= message_index; i++) {
      published_message_buffer[i] = 0;
    }

    message_index = 0;
    publish_flag = 0;
  }
}
