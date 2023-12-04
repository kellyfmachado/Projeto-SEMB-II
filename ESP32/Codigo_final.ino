  #include <WebServer.h>
  #include <WebSocketsServer.h>
  #include <WiFi.h>
  #include <esp32cam.h>

  //Parâmetros da ponte H e microcontrolador
  #define ENA  2
  #define ENB  4
  #define MOTORA_PIN_1  12
  #define MOTORA_PIN_2  13
  #define MOTORB_PIN_1   1
  #define MOTORB_PIN_2   3

  //Parâmetros do Wifi
  const char* ssid = "ESP32" ;
  const char* password = "SEMSENHA";
  
  //Parâmetros dos servidores
  WebServer server(80);
  WebSocketsServer webSocket = WebSocketsServer(81);

  //Led indicador de movimento
  const int ledPin = 4;

  //Resolução da câmera 
  static auto lowRes = esp32cam::Resolution::find(320, 240);

  //Função que recebe os comandos de movimento do site
  void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    Serial.print("Recebido comando: ");
    Serial.println((char*)payload);
    if (type == WStype_TEXT) {
      
      if (strcmp((char*)payload, "frente") == 0) {
        digitalWrite(MOTORA_PIN_1, LOW);
        digitalWrite(MOTORA_PIN_2, HIGH);
        digitalWrite(MOTORB_PIN_1, HIGH);
        digitalWrite(MOTORB_PIN_2, LOW);
        
        analogWrite(ENA, 127);
        analogWrite(ENB, 127);
        
        analogWrite(ledPin, HIGH);
        
      } else if (strcmp((char*)payload, "tras") == 0) {
        digitalWrite(MOTORA_PIN_1, HIGH);
        digitalWrite(MOTORA_PIN_2, LOW);
        digitalWrite(MOTORB_PIN_1, LOW);
        digitalWrite(MOTORB_PIN_2, HIGH);
  
        analogWrite(ENA, 127);
        analogWrite(ENB, 127);
        
        analogWrite(ledPin, HIGH);
      }
      else if (strcmp((char*)payload, "esquerda") == 0) {
        digitalWrite(MOTORA_PIN_1, LOW);
        digitalWrite(MOTORA_PIN_2, HIGH);
        digitalWrite(MOTORB_PIN_1, LOW);
        digitalWrite(MOTORB_PIN_2, HIGH);
  
        analogWrite(ENA, 127);
        analogWrite(ENB, 127);
        
        analogWrite(ledPin, HIGH);
      }
      else if (strcmp((char*)payload, "direita") == 0) {
        digitalWrite(MOTORA_PIN_1, HIGH);
        digitalWrite(MOTORA_PIN_2, LOW);
        digitalWrite(MOTORB_PIN_1, HIGH);
        digitalWrite(MOTORB_PIN_2, LOW);
  
        analogWrite(ENA, 127);
        analogWrite(ENB, 127);
        
        analogWrite(ledPin, HIGH);
      } 
      else if (strcmp((char*)payload, "parar") == 0) {
        digitalWrite(MOTORA_PIN_1, LOW);
        digitalWrite(MOTORA_PIN_2, LOW);
        digitalWrite(MOTORB_PIN_1, LOW);
        digitalWrite(MOTORB_PIN_2, LOW);
  
        analogWrite(ENA, 0);
        analogWrite(ENB, 0);
        
        analogWrite(ledPin, LOW);
      } 
    }
  }

  //Função que envia os frames pro site
  void enviaImagem()
  {
    auto frame = esp32cam::capture();
    if (frame == nullptr) {
      Serial.println("Falha na captura da imagem!");
      server.send(503, "", "");
      return;
    }
    Serial.printf("Captura realizada com sucesso: %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                  static_cast<int>(frame->size()));
   
    server.setContentLength(frame->size());
    server.send(200, "image/jpeg");
    WiFiClient client = server.client();
    frame->writeTo(client);
  }

  //Função de mudança de resoluação da câmera
  void resolucao()
  {
    if (!esp32cam::Camera.changeResolution(lowRes)) {
      Serial.println("Falha ao configurar a resolução da imagem!");
    }
    enviaImagem();
  }
   
  void  setup(){
    Serial.begin(115200);

    //Inicialização dos pinos
    pinMode(ledPin, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(MOTORA_PIN_1, OUTPUT);
    pinMode(MOTORA_PIN_2, OUTPUT);
    pinMode(MOTORB_PIN_1, OUTPUT);
    pinMode(MOTORB_PIN_2, OUTPUT);

    //Configuração da câmera
    Serial.println();
    {
      using namespace esp32cam;
      Config cfg;
      cfg.setPins(pins::AiThinker);
      cfg.setResolution(lowRes);
      cfg.setBufferCount(2);
      cfg.setJpeg(80);
   
      bool ok = Camera.begin(cfg);
      Serial.println(ok ? "Câmera pronta" : "Falha na câmera");
    }

    //Configurações do Wifi
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Conectando ao WiFi...");
    }
    
    Serial.println("Conectado ao WiFi");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());

    //Configuração dos servidores
    server.on("/cam-lo.jpg", resolucao);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    
  }
   
  void loop(){
      webSocket.loop();
      server.handleClient();
  }
