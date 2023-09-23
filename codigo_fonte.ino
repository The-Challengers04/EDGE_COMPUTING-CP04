#include "Arduino.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "DHT.h"

#define ldr A0
#define dhtPin 13

int umidade = 0;  // variavel para armazenar o valor do sensor de umidade
int temp = 0;     // variavel para armazenar o valor do sensor de temperatura
int valueLDR = 0; // variavel para armazenar o valor do sensor de luz

DHT dht(dhtPin, DHT11);

HTTPClient client;

char ssid[] = "<Nome da Rede>";
char pass[] = "<Senha da Rede>";
char serverAddress[] = "https://api.tago.io/data"; // TagoIO address
char contentHeader[] = "application/json";
char tokenHeader[] = "<Chave do servidor>"; // TagoIO Token

void setup()
{
  Serial.begin(9600);
  dht.begin();
  pinMode(ldr, INPUT); // LDR
  init_wifi();
}

void init_wifi()
{
  Serial.println("Conectando WiFi");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Conectado");
  Serial.print("Meu IP eh: ");
  Serial.println(WiFi.localIP());
}

void clearVariables()
{
  umidade = 0;
  temp = 0;
  valueLDR = 0;
}

void loop()
{
  clearVariables();

  for (int i = 0; i < 5; i++)
  {
    valueLDR += analogRead(ldr); // Armazenando o valor do LDR
    temp += dht.readTemperature();
    umidade += dht.readHumidity();
  }
  valueLDR = valueLDR / 5; // Pegando Media de Luz
  temp = temp / 5;
  umidade = umidade / 5; // Pegando a media dos valores obtidos do sensor de umidade

  postSensorInfoToServer(temp, umidade, valueLDR);
  delay(5000);
}

void postSensorInfoToServer(int temperature, int moisture, int luminosity)
{
  char postData[500];
  char tempStructureStart[50];
  char formattedTemp[30];
  char tempStructureEnd[30];
  char moistureStructureStart[50];
  char formattedMoisture[30];
  char moistureStructureEnd[28];
  char luminosityStructureStart[50];
  char formattedLuminosity[30];
  char luminosityStructureEnd[47];
  char endJSON[30];
  char bAny[30];

  int statusCode = 0;

  strcpy(tempStructureStart, "{\"variable\": \"temperature\",\"value\": "); // Copia o texto para postData
  dtostrf(temperature, 6, 2, formattedTemp);                                // Arredonda o valor da temperatura
  strcpy(tempStructureEnd, ",\"unit\": \"C\"}");

  // Adicionando umidade ao JSON
  strcpy(moistureStructureStart, ",{\"variable\": \"moisture\",\"value\": ");
  dtostrf(moisture, 6, 2, formattedMoisture);
  strcpy(moistureStructureEnd, ",\"unit\": \"%\"}");

  // Adicionando luminosidade ao JSON
  strcpy(luminosityStructureStart, ",{\"variable\": \"luminosity\",\"value\": ");
  itoa(luminosity, formattedLuminosity, 10); // Converte um inteiro para string
  strcpy(luminosityStructureEnd, ",\"unit\": \"lux\"}");

  strcpy(postData, "[");

  strncat(postData, tempStructureStart, 100);
  strncat(postData, formattedTemp, 100);
  strncat(postData, tempStructureEnd, 100);

  strncat(postData, moistureStructureStart, 100);
  strncat(postData, formattedMoisture, 100);
  strncat(postData, moistureStructureEnd, 100);

  strncat(postData, luminosityStructureStart, 100);
  strncat(postData, formattedLuminosity, 100);
  strncat(postData, luminosityStructureEnd, 100);

  strncat(postData, endJSON, 100);

  Serial.println(postData);

  client.begin(serverAddress);
  client.addHeader("Content-Type", contentHeader);
  client.addHeader("Device-Token", tokenHeader);
  statusCode = client.POST(postData);

  delay(2000);
  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.println("End of POST to TagoIO");
  Serial.println();
}