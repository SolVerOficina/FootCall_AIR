 #include <NewPing.h>
 #include <BLEDevice.h>
 #include <BLEServer.h>
 #include <BLEUtils.h>
 #include <BLE2902.h>
 #include <MFRC522.h>

//#define TRIGGER_PIN1  6     //Sensor del medio, para cuando es solo una direccion
//#define ECHO_PIN1     7   

#define SS_PIN 21
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

int contador_rfid = 0;
int reseteo_rfid = 10;

#define TRIGGER_PIN2  4 //4//Sensor de subida, cercano al arduino
#define ECHO_PIN2     21 //piso 1: 27 PISO 2: 21  //35 - 21
#define TRIGGER_PIN3  5 //5 //Sensor de bajada, alejado del arduino
#define ECHO_PIN3     18 //PISO 1:34 - PISO 2:18
#define RELAYS        32 // 32 (Cable blanco +) y (Cable amarillo -)
#define RELAYB        33 // 33 (Cable azul +) y (Cable verde -)  
#define LedR          26 // 26 Led SUBIDA
#define LedV          25 // 25 Led BAJADA

// ARDUINO------
//
//#define TRIGGER_PIN2  5  //Sensor de subida, cercano al arduino
//#define ECHO_PIN2     4  
//#define TRIGGER_PIN3  21   //Sensor de bajada, alejado del arduino
//#define ECHO_PIN3     6
//#define RELAYS        3 
//#define RELAYB        2

#define MAX_DISTANCE 200
#define LASER         13

#define SERVICE_UUID           "ffe0" // UART service UUID a5f81d42-f76e-11ea-adc1-0242ac120002
#define CHARACTERISTIC_UUID_RX "ffe1"
#define CHARACTERISTIC_UUID_TX "ffe2"

//#define BUTTON_PIN_BITMASK 0x8010  // 2^15+2^4 en hexa

//NewPing sonar1(TRIGGER_PIN2, ECHO_PIN2, MAX_DISTANCE);
NewPing sonar2(TRIGGER_PIN2, ECHO_PIN2, MAX_DISTANCE);
NewPing sonar3(TRIGGER_PIN3, ECHO_PIN3, MAX_DISTANCE);

int contador1 = 0;
int contador2 = 0;
int contador3 = 0;
int restraso = 500;
bool flag = true;
bool deviceConnected = false;
char credentials[32];
float dist_set_subida;
float dist_set_bajada;
float switch_sub;
float switch_baj;
float distancia2;
float distancia3;
float txValue = 0;
BLECharacteristic *pCharacteristic;
 
// REPLACE WITH YOUR RECEIVER MAC Address
//MAC ADRESS DE ANTENA ROJA: 30:C6:F7:0D:D2:28
//uint8_t broadcastAddress[] = {0X84, 0xCC, 0xA8, 0x7A, 0x0A, 0x5C};
//84:CC:A8:7B:CF:90

//aircll darsakud _ 94:B9:7E:C0:66:CC
//aircall prueba _ 84:CC:A8:7A:0A:5C

        
void inicio() {
  Serial.println("Iniciando");
  digitalWrite(LedR, HIGH);
  delay(500);
  digitalWrite(LedR, LOW);
  delay(500);
  digitalWrite(LedR, HIGH);
  delay(500);
  digitalWrite(LedV, HIGH);
  delay(500);
  digitalWrite(LedV, LOW);
  delay(500);
  digitalWrite(LedV, HIGH);
  delay(500);
  digitalWrite(LedR, LOW);
  digitalWrite(LedV, LOW);
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};




class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      
      if (rxValue.length() > 0) {
        Serial.println("***");
        Serial.print("Received Value: ");

          for (int i = 0; i < rxValue.length(); i++) {
          Serial.println(rxValue[1]);             
        }
        
        Serial.println("---rxValues--");
        Serial.println(rxValue.length());
        Serial.println(rxValue[0]);
        Serial.println(rxValue[1]);
        Serial.println("--------------");
       Serial.println("*********");
//
//        // Do stuff based on the command received from the app
//    
        if (rxValue.find("X") != -1) {
          digitalWrite(RELAYS,HIGH);
          digitalWrite(LedR, HIGH);
          Serial.println("Llamada de subida");
          ESP.restart();
   
        }else{
          digitalWrite(RELAYS,LOW);
          digitalWrite(LedR, LOW);
        }

        if (rxValue.find("Y") != -1) {
          digitalWrite(RELAYB, HIGH);
          digitalWrite(LedV, HIGH);
          Serial.println("Llamada de bajada");
          ESP.restart();
        }else{
          digitalWrite(RELAYB, LOW);
          digitalWrite(LedV, LOW);
        }

        Serial.println();
        Serial.println("***");
      }
    }
};

void setup() {
  Serial.begin(115200);
  //delay(10);
  
  pinMode(RELAYS,OUTPUT);
  pinMode(RELAYB,OUTPUT);
  pinMode(LASER,OUTPUT);
    pinMode(LedR,OUTPUT);
  pinMode(LedV,OUTPUT);
  
  digitalWrite(RELAYS,LOW);
  digitalWrite(RELAYB,LOW); 
  digitalWrite(LASER,HIGH); 
  SetDistance();
 
  // Create the BLE Device
  BLEDevice::init("FOOTCALL SV"); // Give it a name

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify.");
}


void(* resetFunc) (void) = 0; //declare reset function @ address 0

 void SetDistance(){
  Serial.println("-------------------------FOOTCALL AIR-------------------------------");
  Serial.println("-----------------------------V. 1.0.0-------------------------------");
  inicio();
  float dist_subida = (0.034027/2)*sonar2.ping_median(10);
  float dist_bajada = (0.034027/2)*sonar3.ping_median(10);
  Serial.println("DISTANCIA INICIAL:: ");
  Serial.print("SUBIDA: ");
  Serial.println(dist_subida);
  Serial.print("BAJADA: ");
  Serial.println(dist_bajada);
  dist_set_subida = dist_subida - 5;
  dist_set_bajada = dist_bajada - 9;
  Serial.println("EL BOTON ACTIVA A ESTA DISTANCIA: ");
  Serial.print("SUBIDA: ");
  Serial.println(dist_set_subida);
  Serial.print("BAJADA: ");
  Serial.println(dist_set_bajada);
  Serial.println("--------------------------------------------------------------");
  Serial.println("--------------------------------------------------------------");
}

void loop(){

   Serial.print("Contador de reset: ");
   Serial.println(contador1);
   contador1++;
  
  if(contador1 > 300){
 Serial.println("RESETEANDO DE SOFTWARE");
    resetFunc();
  }
  
    distancia2 = (0.034027/2)*sonar2.ping_median(10);
    Serial.print("Distancia 2: ");
    Serial.println(distancia2);
    
    if (distancia2 == 0 || distancia2 > dist_set_subida){
      Serial.println("NADA SUBIDA");
       digitalWrite(RELAYS,LOW);
      digitalWrite(LASER,HIGH);
      contador2 = 0;
    }

        distancia3 = (0.034027/2)*sonar3.ping_median(10);
    Serial.print("Distancia 3: ");
    Serial.println(distancia3);
     
     if (distancia3 == 0 || distancia3 > dist_set_bajada){
      Serial.println("NADA BAJADA");
       digitalWrite(RELAYB,LOW);
       contador3 = 0;
    }

     if ( distancia2 > 0 && distancia2 < dist_set_subida && contador2 < 35){
          digitalWrite(RELAYS,HIGH);
          digitalWrite(LASER,HIGH);
          digitalWrite(LedR,HIGH);
          delay(50);
          contador2++;
          Serial.println("ACTIVADO SENSOR DE SUBIDA");
          // Serial.println(contador2);
    }


   if ( distancia2 > 0 && distancia2 < dist_set_subida && contador2 >= 35){
       Serial.println("BOTON DE SUBIDA TRABADO");
       //resetFunc();  //call reset
       digitalWrite(RELAYS,LOW);
       digitalWrite(LASER,HIGH);
       digitalWrite(LedR, HIGH);
       delay(500);
       digitalWrite(LedR,LOW);
       delay(50);
       contador2++;
    }

    // Si lleva mas de 1 minuto, se resetea el modulo
    if ( contador2 >= 70 ){
        digitalWrite(LASER,LOW);
        Serial.println("RESETEANDO MODULO");
        delay(2000);
       resetFunc();  //call reset
    }  
   

//  Sensor Bajando
      if ( distancia3 > 0 && distancia3 < dist_set_bajada && contador3 < 35){
      digitalWrite(RELAYB,HIGH);
      digitalWrite(LedR, LOW);
      digitalWrite(LedV,HIGH);
      delay(500);
      digitalWrite(LedV, LOW);
      delay(50);
      contador2++;
      Serial.println("ACTIVADO SENSOR DE BAJADA");
      Serial.println(contador2);
    }

//Verificar si el boton leva mas de 30 segundos activado

    if ( distancia3 > 0 && distancia3 < dist_set_bajada && contador3 >= 35){
      Serial.println("BOTON DE BAJADA TRABADO");
      //resetFunc();  //call reset
      digitalWrite(RELAYB,LOW);
      digitalWrite(LASER,HIGH); 
      digitalWrite(LedV,HIGH);
      delay(50);
      contador3++;
    }

// Si lleva mas de 1 minuto, se resetea el modulo
    if ( contador3 >= 70 ){
      digitalWrite(LASER,LOW); 
      Serial.println("RESETEANDO POR BOTON BAJADA");
      delay(2000);
      resetFunc();  //call reset
    }
    Serial.println("-------------------------------------------------------------");
    Serial.println("-------------------------------------------------------------");
}
//    if ( ! mfrc522.PICC_IsNewCardPresent())
//{
  //Serial.println("new card prtesent");
//   if(contador_rfid > reseteo_rfid){
//        ESP.restart();
//        contador_rfid = 0;
//        Serial.println(" reseteando");
//    }
//    contador_rfid++;
//  //Serial.print("contador");
//    Serial.println(contador_rfid);
//    delay(1000);
//    return;
//}

  // Select one of the cards
// if ( ! mfrc522.PICC_ReadCardSerial()) 
//  {
//   // Serial.println(" Access denied");
//    return;
//  }
//  
//  //Show UID on serial monitor
//  Serial.print("UID tag :");
//  String content= "";
//  byte letter;
//  for (byte i = 0; i < mfrc522.uid.size; i++) 
//  {
//     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
//     Serial.print(mfrc522.uid.uidByte[i], HEX);
//     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
//     content.concat(String(mfrc522.uid.uidByte[i], HEX));
//  }
//  Serial.println();
//  Serial.print("Message : ");
//  content.toUpperCase();
////LLAVE MAESTRA ----------------------------
//     if (content.substring(1) == "94 94 5C 33") //change here the UID of the card/cards that you want to give access UID tag : 94 94 5C 33
//  {
//    myData.msj = 6;
//    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
//    Serial.println("Activando Piso 1");
//    digitalWrite(RELAYB,HIGH);
//    Serial.println();
//    delay(500);
//  }
//     if (content.substring(1) == "47 33 43 34") //change here the UID of the card/cards that you want to give access UID tag : 94 94 5C 33
//  {
//    myData.msj = 9;
//    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
//    Serial.println("Activando Piso 1");
//    digitalWrite(RELAYB,HIGH);
//    Serial.println();
//    delay(500);
//  }
//  if (content.substring(1) == "96 74 0F 9B") //change here the UID of the card/cards that you want to give access UID tag : 94 94 5C 33
//  {
//    myData.msj = 3;
//    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
//    Serial.println("Activando Piso 1");
//    digitalWrite(RELAYB,HIGH);
//    Serial.println();
//    delay(500);
//  }
//  
//  
//  else{
//    Serial.println(" Access denied");
//    delay(500);  
//  }
//  Serial.println(contador_rfid); 
//
//  //----------------------------------------------------------------------------------------
//  delay(restraso);
