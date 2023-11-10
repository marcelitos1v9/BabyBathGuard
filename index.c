#include "DHTesp.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
///########### configuração do wifi
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
// Defining the WiFi channel speeds up the connection:
#define WIFI_CHANNEL 6
int status = WL_IDLE_STATUS; // the Wifi radio's status
///########## configuração do MQTT
const char* mqtt_server = "test.mosquitto.org";// MQTT broker
char* my_topic_SUB = "FIT/SUB";// the chosen topic
char* my_topic_PUB1 = "FIT/PUB_Temperatura";// the chosen topic
char* my_topic_PUB2 = "FIT/PUB_Umidade";// the chosen topic
/////##### configuração do wifi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);
/////###configuração do tempo de leitura
int contador = 1;
char mensagem[30];
unsigned long lastMillis = 0;
////#############Configuração do DHT22
const int DHT_PIN = 15;
DHTesp dhtSensor;
float ambiente[2];
float lastTemp = -999.0;
float lastHum = -999.0;
void setup() {
// put your setup code here, to run once:
Serial.begin(115200);
Serial.println("Olá, ESP32!");
dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
conectawifi();
client.setServer(mqtt_server, 1883); // Set the MQTT broker server and port
client.setCallback(callback); // Set the callback function for incoming MQTT messages
}
void loop() {
// put your main code here, to run repeatedly:
sensor_ambiente();
// Verifica se houve uma alteração nos valores de temperatura ou umidade
if ( ambiente[0] != lastTemp || ambiente[1] != lastHum) {
Serial.print("Temperatura: ");
Serial.print( ambiente[0]);
Serial.print("°C\tUmidade: ");
Serial.print( ambiente[1]);
Serial.println("%");
if (ambiente[0] >= 35 && ambiente[0] <= 38) {
Serial.println("A temperatura está perfeita para o banho do bebê.");
} else if (ambiente[0] < 35) {
Serial.println("Está frio para o banho do bebê!");
} else {
Serial.println("Está quente para o banho do bebê!");
}
// Atualiza os valores anteriores
lastTemp = ambiente[0];
lastHum = ambiente[1];
}
// Check MQTT connection and process incoming messages
if (!client.connected()) {
reconnect();
}
client.loop();
delay(100); // Add a delay to avoid flooding the MQTT broker with messages
//Aguarda 15 segundos para enviar uma nova mensagem
if (millis() - lastMillis > 15000) {
lastMillis = millis();
//sprintf(mensagem, "MQTT ESP32 - Mensagem no. %d", contador);
///envio da temperatura
int val_int = (int) ambiente[0];
float val_float = (abs(ambiente[0]) - abs(val_int)) * 100000;
int val_fra = (int)val_float;
sprintf(mensagem, "%d.%d", val_int, val_fra );
Serial.print("Mensagem enviada: ");
Serial.println(mensagem);
//Envia a mensagem ao broker
client.publish(my_topic_PUB1, mensagem);
Serial.print(contador);
Serial.println("-Temperatura enviada com sucesso...");
///envio da umidade
val_int = (int) ambiente[1];
val_float = (abs(ambiente[1]) - abs(val_int)) * 100000;
val_fra = (int)val_float;
sprintf(mensagem, "%d.%d", val_int, val_fra );
//sprintf(mensagem,"%d", ambiente[1] );
Serial.print("Mensagem enviada: ");
Serial.println(mensagem);
//Envia a mensagem ao broker
client.publish(my_topic_PUB2, mensagem);
Serial.print(contador);
Serial.println("-Umidade enviada com sucesso...");
//Incrementa o contador
contador++;
}
}
void conectawifi()
{
WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
Serial.print("Conectando ao WiFi ");
Serial.print(WIFI_SSID);
// Wait for connection
while (WiFi.status() != WL_CONNECTED) {
delay(100);
Serial.print(".");
}
Serial.println(" Conectado!");
Serial.print("IP address: ");
Serial.println(WiFi.localIP());
}
// Process incoming MQTT message and control the servo motor
void callback(char* topic, byte* payload, unsigned int length) {
String string;
Serial.print("Chegou a mensagem [");
Serial.print(topic);
Serial.print("] ");
for (int i = 0; i < length; i++) {
string+=((char)payload[i]);
}
Serial.print(string);
}
// Attempt to reconnect to the MQTT broker if the connection is lost
void reconnect() {
while (!client.connected()) {
Serial.print("Tentando conectar ao MQTT ...");
if (client.connect("ESPClient")) {
Serial.println("Conectado");
client.subscribe(my_topic_SUB);
} else {
Serial.print("falhou, rc=");
Serial.print(client.state());
Serial.println(" Tente novamente em 5 segundos");
}
}
}
void sensor_ambiente()
{
TempAndHumidity data = dhtSensor.getTempAndHumidity();
ambiente[0]= data.temperature;
ambiente[1] = data.humidity;
//Serial.println("Temperatura: " + String(ambiente[0], 2) + "°C");
//Serial.println("Umidade: " + String(ambiente[1], 1) + "%");
//.println("---");
// delay(200);
}
