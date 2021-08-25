#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include "AsyncTelegram.h"

#define pinRele D8
#define pinSensor D7

AsyncTelegram myBot;
WiFiClientSecure client;

const char* WIFI_SSID = "seuSSID";     		// nome de sua rede WiFi
const char* WIFI_SENHA = "suaSenha";  // senha de sua rede WiFi
const char* BOT_TOKEN = "1234567890:AAFGVBIztlsdj2SjM7OyWYjlXyo89JoWgzA" ; // Token de seu bot.
const uint CHAT_ID = 1723188539; // Troque por seu ID de chat do telegram.
const String URL_FORM = "GQeSLQIFA1I_bVp1Ge5MHbJI_loUtyX-9CYz8PnrSxeMHA3lhCgK1yWQ";
const String FORM_INPUT = "1234567890"; 
uint32_t tempoSensor;
bool statusSensor = true;

void setup() {
  pinMode(pinRele, OUTPUT);
  pinMode(pinSensor, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println("Iniciando Bot...");

  //Conexão com Wi-Fi...
  WiFi.setAutoConnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  delay(500);

  Serial.print("Procurando Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }


  // Para garantir a validação do certificado, WiFiClientSecure necessita do horário atualizado.
  myBot.setClock("CET-1CEST,M3.5.0,M10.5.0/3");

  myBot.setUpdateTime(2000);
  myBot.setTelegramToken(BOT_TOKEN);

  // Inicia o bot
  Serial.print("\nIniciando conexão com Telegram... ");
  myBot.begin() ? Serial.println("Conexão sucedida.") : Serial.println("Falha na conexão.");

  Serial.print("Nome do Bot: @");
  Serial.println(myBot.userName);
  String mensagem = "Olá! Bem-vindo ao bot Telegram-Esp.\n";
  mensagem += "Este é um exemplo de bot com entrada/saída de comando e registro em nuvem.\n\n";
  mensagem += "Segue abaixo a lista de comandos:\n";
  mensagem += "/acenderLuz;\n";
  mensagem += "/apagarLuz;\n";
  mensagem += "/desligarSensor;\n";
  mensagem += "/ligarSensor.\n";
  mensagem += "Em caso de acionamento do sensor será emitido um alerta ao contato com a ID salva.\n";
  myBot.sendToUser(CHAT_ID, mensagem);
}

void loop() {
  delay(500);
  TBMessage msg; // Variável que armazena os dados da mensagem.
  if (myBot.getNewMessage(msg)) { // Caso receba uma nova mensagem...
    if (msg.text == "/acenderLuz") {
      digitalWrite(pinRele, 1);
      myBot.sendMessage(msg, "Luzes acesas.");
    }
    if (msg.text == "/apagarLuz") {
      digitalWrite(pinRele, 0);
      myBot.sendMessage(msg, "Luzes apagadas.");
    }
    if (msg.text == "/desligarSensor") {
      statusSensor = false;
      myBot.sendMessage(msg, "Sensor Desligado.");
    }
    if (msg.text == "/ligarSensor") {
      statusSensor = true;
      myBot.sendMessage(msg, "Sensor Ligado.");
    }
  }
  if (statusSensor) { // Caso o sensor esteja ativado
    if (digitalRead(pinSensor) == 0 && (millis() - tempoSensor > 6000)) {
      Serial.println("Sensor do quarto acionado!");
      alertaTelegram("Sensor do quarto acionado!");
      alertaGoogle("Sensor%20IR"); // Espaços são representados por %20
    }
  }
}

void alertaTelegram(String mensagem) {
  TBMessage alerta;
  alerta.text = mensagem;
  myBot.sendToUser(CHAT_ID, alerta.text);
  tempoSensor = millis();
}

void alertaGoogle(String nomeSensor) {

  client.setInsecure();
  if (client.connect("docs.google.com", 443) == 1)//Tenta se conectar ao servidor do Google docs na porta 443 (HTTPS)
  {
    String toSend = "GET /forms/d/e/"; 
    toSend += URL_FORM + "/formResponse?ifq&entry." + FORM_INPUT + "=" + nomeSensor; 
    toSend += "&submit=Submit HTTP/1.1";

    client.println(toSend); // Envio do GET ao Servidor
    client.println("Host: docs.google.com"); 
    client.println();
    client.stop();//Encerra a conexao com o servidor
    Serial.println("Dados enviados.");
  }
  else
  {
    Serial.println("Erro ao se conectar");
  }
}
