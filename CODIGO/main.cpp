#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define sensorPin 32
#define relePin1 25
#define relePin2 26
#define relePin3 27
#define COLUMS 16
#define ROWS 2

#define EEPROM_SIZE 512

Servo myServo1;
Servo myServo2;

LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);

// Caracteres para display
int sleepyFace[2][16]={
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,0}
};
int happyFace[2][16]={
 {0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0},
 {0,0,1,0,0,1,0,0,0,0,0,1,0,0,1,0}
};
int angryFace[2][16]={
 {0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0},
 {0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0}
};
int normalFace[2][16]={
 {0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,0},
 {0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,0}
};

const unsigned long inactiveTime = 60000; // 1 minuto en milisegundos
unsigned long lastMotionTime = 0;
bool isMotionDetected = false;
bool isControlActive = false;
WebServer server(80);

//Creamos nuestra propia red wifi AP
const char* apSsid = "AP_SSID"; 
const char* apPassword = "AP_Password"; 

// Funcion para imprimir caras en el display
void printFace(int emo[][16]){
  lcd.clear();
  for(int i=0;i<2;i++){
    for(int j=0; j<16;j++){
      lcd.setCursor(j, i);    
      if(emo[i][j]==1){
        lcd.write(255);
      }
    }
  }
}
void neutral(){
  printFace(normalFace);
}
void happy(){
  printFace(happyFace);
}
void angry(){
  printFace(angryFace);
}
void sleepy(){
  printFace(sleepyFace);
}

//Constructores
void controlarSecuencia(int rele1, int rele2, int rele3, int servo1Pos, int servo2Pos, String mood);
void handleRoot();
void handleWifi();
void handleM1();
void handleM2();
void handleM3();
void handleM4();
void handleM5();
void handleM6();
void handleM7();
void handleM8();
void handleM9();
void handleA1();
void handleA2();
void handleA3();

void initAP(const char* apSsid,const char* apPassword);
void initWebServer();
//GestionarCredencialesMemoria
void saveCredentials(const char* ssid, const char* password) {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < 32; ++i) {
    EEPROM.write(50 + i, ssid[i]);
    EEPROM.write(100 + i, password[i]);
  }
  EEPROM.commit();
}
void loadCredentials(char* ssid, char* password) {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < 32; ++i) {
    ssid[i] = EEPROM.read(50 + i);
    password[i] = EEPROM.read(100 + i);
  }
  ssid[32] = '\0';
  password[32] = '\0';
}
void clearCredentials() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 50; i < 132; ++i) { // Limpiar las posiciones de la EEPROM donde se almacenan las credenciales
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

void setup(){
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  pinMode(relePin1, OUTPUT);
  pinMode(relePin2, OUTPUT);
  pinMode(relePin3, OUTPUT);

  // Configura los relés y servos en el estado inicial
  digitalWrite(relePin1, HIGH);
  digitalWrite(relePin2, HIGH);
  digitalWrite(relePin3, HIGH);

  myServo1.attach(2);
  myServo2.attach(15);
  myServo1.write(80); // Posición inicial
  myServo2.write(80); // Posición inicial

  while (lcd.begin(COLUMS, ROWS) != 1) //colums - 20, rows - 4
  {
    Serial.println(F("PCF8574 is not connected or lcd pins declaration is wrong. Only pins numbers: 4,5,6,16,11,12,13,14 are legal."));
    delay(5000);   
  }
  lcd.print(F("PCF8574 is OK..."));    //(F()) saves string to flash & keeps dynamic memory free
  delay(2000);

  lcd.clear();
  lcd.noDisplay();
  lcd.noBacklight();

  char ssid[33];
  char password[33];
  loadCredentials(ssid, password);
  Serial.print("SSID almacenado: ");
  Serial.println(ssid);
  Serial.print("Contraseña almacenada: ");
  Serial.println(password);
  clearCredentials();

  // Verificacion Credenciales Vacias inicializar AP
  if (strlen(ssid) == 0 || strlen(password) == 0) {
    Serial.println("No se han leído credenciales válidas, iniciando modo AP");
    initAP("SSID", "Password");
    return;
  }
  Serial.print("SSID almacenado: ");
  Serial.println(ssid); 

  //WiFi en modo STA intenta conectarse a la red (Preexistente)
  WiFi.begin(ssid, password);
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED && cnt < 8) {
    delay(1000);
    Serial.print(".");
    cnt++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conexión establecida");
    Serial.println(WiFi.localIP());
    initWebServer();
  } else {
    Serial.println("Conexión no establecida, iniciando modo AP");
    clearCredentials(); // Borrar credenciales almacenadas si la conexión falla
    initAP("SSID", "Password");  //Si falla vuelve modo AP
  }  

}
void loop(){
  server.handleClient();
  // Control del sensor infrarrojo
  int sensorState = digitalRead(sensorPin); 

  if (sensorState == LOW) {
    lastMotionTime = millis(); // Actualiza el tiempo de la última detección de movimiento
    isMotionDetected = true;
    isControlActive = true; // El control está activo
    controlarSecuencia(LOW, HIGH, HIGH, 80, 80, "neutral"); // Ejemplo de secuencia de movimiento detectado
  } else {
    isMotionDetected = false;
  }

  // Si no hay movimiento durante un minuto, vuelve al estado predeterminado
  if (!isMotionDetected && (millis() - lastMotionTime >= inactiveTime)) {
    isControlActive = false; // El control ya no está activo
    controlarSecuencia(HIGH, HIGH, HIGH, 80, 80, "off"); // Vuelve al estado predeterminado
  }
}

//funcion controlar secunecia
void controlarSecuencia(int rele1, int rele2, int rele3, int servo1Pos, int servo2Pos, String mood) {
  // Controla los relés
  digitalWrite(relePin1, rele1);
  digitalWrite(relePin2, rele2);
  digitalWrite(relePin3, rele3);

  // Controla los servos
  myServo1.write(servo1Pos);
  myServo2.write(servo2Pos);

  // Actualiza el estado del LCD
  if (mood == "happy") {
    happy();
  } else if (mood == "angry") {
    angry();
  } else if (mood == "neutral") {
    lcd.display();
    lcd.backlight();
    neutral();
  } else if (mood == "sleepy") { 
    sleepy();
  }else if (mood == "off")
  {
    lcd.noDisplay();
    lcd.noBacklight();
  }
  
}

//Funcion para manejar la raiz
void handleRoot() {
  String html = "<html><body>";
  html += "<form method='POST' action='/wifi'>";
  html += "Red Wi-Fi: <input type='text' name='ssid'><br>";
  html += "Contraseña: <input type='password' name='password'><br>";
  html += "<input type='submit' value='Conectar'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

//Funcion para manejar la conexion wifi
void handleWifi() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  Serial.print("Conectando a la red Wi-Fi ");
  Serial.println(ssid);
  Serial.print("Clave Wi-Fi ");
  Serial.println(password);
  Serial.print("...");

  WiFi.disconnect(); // Desconectar la red Wi-Fi anterior, si se estaba conectado
  WiFi.begin(ssid.c_str(), password.c_str(),6);
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED and cnt < 8) {
    delay(1000);
    Serial.print(".");
    cnt++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conexión establecida");
    Serial.println(WiFi.localIP());
    saveCredentials(ssid.c_str(), password.c_str());
    server.send(200, "text/plain", "Conexión establecida " + WiFi.localIP().toString());
    WiFi.mode(WIFI_STA); // Desactivar modo AP
  } else {
    Serial.println("Conexión no establecida");
    server.send(200, "text/plain", "Conexión no establecida");
    // Volver al modo AP si no se puede conectar
    initAP(apSsid, apPassword);
  }
}

//Handles para peticiones GET
void handleNotFound(){
    server.send(404, "text/plain", "404: Not found");
}
void handleM1(){
  if (isControlActive) {
    controlarSecuencia(LOW, HIGH, HIGH, 80, 80, "neutral");
    lastMotionTime = millis();
    server.send(200, "text/plain", "neutral");
  }else{
    server.send(403, "text/plain", "inactive");
  }
}
void handleM2(){
  if(isControlActive){
    controlarSecuencia(HIGH, LOW, HIGH, 35, 125, "neutral"); 
    lastMotionTime = millis();
    server.send(200, "text/plain", "neutralAngry");
  }else{
    server.send(403, "text/plain", "inactive");
  }
}
void handleM3(){
  if (isControlActive) {
    controlarSecuencia(HIGH, HIGH, LOW, 125, 35, "neutral"); 
    lastMotionTime = millis();
    server.send(200, "text/plain", "neutralWorried");
  }else{
    server.send(403, "text/plain", "inactive");
  }
}
void handleM4(){
  if(isControlActive){
    controlarSecuencia(HIGH, LOW, LOW, 80, 80, "happy");
    lastMotionTime = millis();
    server.send(200, "text/plain", "neutralhappy");
  }else{
    server.send(403, "text/plain", "inactive");
  }
}
void handleM5(){
  if (isControlActive){
    controlarSecuencia(LOW, HIGH, LOW, 125, 35, "happy"); 
    lastMotionTime = millis();
    server.send(200, "text/plain", "happyWorried");
} else{
    server.send(403, "text/plain", "inactive");
  }
}
void handleM6(){
  if (isControlActive){
    controlarSecuencia(HIGH, LOW, HIGH, 35, 125, "angry");
    lastMotionTime = millis();
    server.send(200, "text/plain", "furius");
}else{
    server.send(403, "text/plain", "inactive");
  }
}
void handleM7(){
  if (isControlActive){
    controlarSecuencia(LOW, LOW, HIGH, 35, 125, "angry");
    lastMotionTime = millis();
    server.send(200, "text/plain", "furiusWorried");
}else{
    server.send(403, "text/plain", "inactive");
  }
}
void handleM8(){
  if(isControlActive){
    controlarSecuencia(LOW, HIGH, LOW, 125, 35, "sleepy");
    lastMotionTime = millis();
    server.send(200, "text/plain", "noWay");
}else{
    server.send(403, "text/plain", "inactive");
  }
}
void handleM9(){
  if(isControlActive){
    controlarSecuencia(LOW, LOW, HIGH, 35, 125, "sleepy"); 
    lastMotionTime = millis();
    server.send(200, "text/plain", "dissapointed");
}else{
    server.send(403, "text/plain", "inactive");
  }
}
void handleA1(){if(isControlActive){
    lastMotionTime = millis();
    server.send(200, "text/plain", "secuencia 1");
    controlarSecuencia(LOW, HIGH, HIGH, 80, 80, "neutral");
    delay(3000);  
    controlarSecuencia(HIGH, LOW, HIGH, 35, 125, "neutral");
    delay(3000); 
    controlarSecuencia(HIGH, HIGH, LOW, 125, 35, "neutral"); 
    delay(3000);
}else{
    server.send(403, "text/plain", "inactive");
  }
} 
void handleA2(){ if(isControlActive){
    lastMotionTime = millis();
    server.send(200, "text/plain", "secuencia 2");
    controlarSecuencia(HIGH, LOW, LOW, 80, 80, "happy");
    delay(3000);
    controlarSecuencia(LOW, HIGH, LOW, 125, 35, "happy");
    delay(3000);
    controlarSecuencia(HIGH, LOW, HIGH, 35, 125, "angry");
    delay(3000);
}else{
    server.send(403, "text/plain", "inactive");
  }
}
void handleA3(){if(isControlActive){
    lastMotionTime = millis();
    server.send(200, "text/plain", "secuencia 3");
    controlarSecuencia(LOW, LOW, HIGH, 35, 125, "angry");
    delay(3000);
    controlarSecuencia(LOW, HIGH, LOW, 125, 35, "sleepy");
    delay(3000);
    controlarSecuencia(LOW, LOW, HIGH, 35, 125, "sleepy"); 
    delay(3000);
    
}else{
    server.send(403, "text/plain", "inactive");
  }
}

//Funcion para inicializar el punto de acceso
void initAP(const char* apSsid,const char* apPassword) { // Nombre de la red Wi-Fi y  Contraseña creada por el ESP32
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid, apPassword);
  initWebServer();
  Serial.print("Ip de esp32...");
  Serial.println(WiFi.softAPIP());
  //Serial.println(WiFi.localIP());
}
void initWebServer() {
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/M1", handleM1);
  server.on("/M2", handleM2);
  server.on("/M3", handleM3);
  server.on("/M4", handleM4);
  server.on("/M5", handleM5);
  server.on("/M6", handleM6);
  server.on("/M7", handleM7);
  server.on("/M8", handleM8);
  server.on("/M9", handleM9);
  server.on("/A1", handleA1);
  server.on("/A2", handleA2);
  server.on("/A3", handleA3);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Servidor web iniciado");
}