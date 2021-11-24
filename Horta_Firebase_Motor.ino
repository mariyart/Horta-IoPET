/************************* Inclusão das Bibliotecas *********************************/
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <Ticker.h>
#include <Arduino_JSON.h>

/************************* Conexão WiFi*********************************/
 
#define WIFI_SSID       "Artuso Terreo" // nome de sua rede wifi
#define WIFI_PASS       "toby0511"     // senha de sua rede wifi

/********************* Credenciais Firebase *************************/
 
#define FIREBASE_HOST "dashboard-9d315-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "U6XT78qpHKDaImtAS7CDzld7j1lsez7g4to8uhaH"


/********************** Variaveis globais *******************************/
 
WiFiClient client;

float FazLeituraUmidade(void);

#define LIMITE_UMIDADE_PARA_REGAR   30     //limite percentual inferior para a planta se auto regar
#define TEMPO_PARA_REGAR            2000   //tempo (em ms) ao qual a válvula de vazão solenóide de água será acionada quando for necessário regar a planta 
#define SAIDA_COMANDO_VALVULA       D0     //saída do NodeMCU que acionará a válvula de vazão solenóide de água

#define DHTPIN D4                // Pinos do DHT11
#define DHTTYPE DHT11              // Tipo de DHT
DHT dht(DHTPIN, DHTTYPE); 
 
FirebaseData firebaseData;
Ticker ticker;

// Publique a cada 5 min
#define PUBLISH_INTERVAL 1000*60
bool publishNewState = true;



void publish(){
  publishNewState = true;
}



void setupPins(){

  pinMode(SAIDA_COMANDO_VALVULA, OUTPUT);   // Definindo pino como saída
  digitalWrite(SAIDA_COMANDO_VALVULA, LOW); // Inicia com válvula fechada
  dht.begin();                              // Inicia leitura
}

void setupWifi(){
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
}

void setupFirebase(){
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  if (Firebase.setBool(firebaseData, "DHT11/Motor", false))
  {
    Serial.print("Motor desligado");
  }
}

void setup() {
  Serial.begin(9600);

  setupPins();
  setupWifi();    

  setupFirebase();

  // Registra o ticker para publicar de tempos em tempos
  ticker.attach_ms(PUBLISH_INTERVAL, publish);
}

void loop() {

  // Apenas publique quando passar o tempo determinado
  if(publishNewState){
    Serial.println("Publica novo Estado");
    // Obtem os dados do sensor DHT 
    float umidade = dht.readHumidity();            // Lê umidade
    Serial.println(umidade);
    float temperatura = dht.readTemperature();     // Lê Temperatura
    Serial.println(temperatura);
    if(!isnan(umidade) && !isnan(temperatura)){    // Se leitura foi feita
      // Manda para o firebase
      if (Firebase.pushFloat(firebaseData, "DHT11/Temperature", temperatura));
      if (Firebase.pushFloat(firebaseData, "DHT11/Humidity", umidade));    
 

      // Faz leitura da umidade do solo

    float UmidadePercentualLida; 
    int UmidadePercentualTruncada;

    UmidadePercentualLida = FazLeituraUmidade();
    UmidadePercentualTruncada = (int)UmidadePercentualLida; //trunca umidade como número inteiro
 
    delay(200);
    
    Serial.print(UmidadePercentualTruncada); Serial.println("%");

    if (Firebase.pushInt(firebaseData, "DHT11/UmidadeSolo", UmidadePercentualTruncada));  

    if (UmidadePercentualTruncada <= LIMITE_UMIDADE_PARA_REGAR)
    {
        //em caso positivo, a planta é regada no tempo contido no define TEMPO_PARA_REGAR
        digitalWrite(SAIDA_COMANDO_VALVULA,HIGH);  //abre a válvula de vazão solenóide de água
        if (Firebase.setBool(firebaseData, "DHT11/Motor", true));
        delay(TEMPO_PARA_REGAR);
        digitalWrite(SAIDA_COMANDO_VALVULA,LOW);  //fecha a válvula de vazão solenóide de água
        if (Firebase.setBool(firebaseData, "DHT11/Motor", false));
    }

    publishNewState = false;
      
    }else{
      Serial.println("Error Publishing");
    }
  }
  
  // Verifica o valor do motor no firebase 
  if(Firebase.getBool(firebaseData, "DHT11/Motor")) {
    if (firebaseData.dataType() = "bool") {
      Serial.print(firebaseData.intData());
      //em caso positivo, a planta é regada no tempo contido no define TEMPO_PARA_REGAR
      digitalWrite(SAIDA_COMANDO_VALVULA,HIGH);  //abre a válvula de vazão solenóide de água
      delay(TEMPO_PARA_REGAR);
      digitalWrite(SAIDA_COMANDO_VALVULA,LOW);  //fecha a válvula de vazão solenóide de água
    }
  } else {
    Serial.println(firebaseData.errorReason());
  }
  
  
  delay(200);
}


// Leitura da umidade
float FazLeituraUmidade(void)
{
    int ValorADC;
    float UmidadePercentual;
   
     ValorADC = analogRead(0);   //418 -> 1.0V
     Serial.print("[Leitura ADC] ");
     Serial.println(ValorADC);
        
     UmidadePercentual = map(ValorADC, 1024, 300, 0, 100);  // Função map calcula porcentagem obtida em um certo intervalo 
   
     return UmidadePercentual;
}
