// Modulo Master, RTC, Sensor ultrasónico, Comunicación puerto COM (RS232), Comunicación I2C RTC, Wireless data
// ICE. Oscar Aguilar Cruz
//
// Comunicación serial/inalambrica con módulo esclavo NRF24 de 100 mW
// Comunicación serial RS234 con aplicación en PC
// RTC para obtener tiempo real sin depender de PC
// Interrupts: Timer, External, Data
// No LCD, datos de monitoreo en aplicación desktop
// Sensor Ultrasónico.
//
//=========================> LIBRERIAS <=========================================================================================================
////////////////////////////
#include <TimerOne.h>     // Timer1
//#include <Wire.h>         // I2C
#include <SPI.h>            // SPI
#include <RH_NRF24.h>       // Transceiver
#include <DS1302.h>         // RTC DS1302
//////////////////////////// Transceiver
//=========================> VARIABLES <==========================================================================================================
///////////////////////////////////////////////////////////////// RTC - RS232 <-------------------------------------------------------------------
String d_serial = "";                                          // String que obtiene información RS232
byte se = 0, mi = 0, ho = 0, wd = 0, di = 0, me = 0, an = 0;   //Variables para tiempo
const int DS1307 = 0x68;                                       // Direccion de RTC
const char* days[] = {"Domingo", "Lunes", "Martes", "MiErcoles", "Jueves", "Viernes", "Sabado"};
const char* mes[] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};

DS1302 rtc(8,7,6);         // Inicializa DS1302 rtc([CE/RST], [I/O], [CLOCK]);    
const char* dias[] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};    
const int cienAnio = 2000;   
/////////////////////////////////// SENSOR ULTRASONICO  <-----------------------------------------------------------------------------------------
const byte ledPin = 5;          // Pin LED rojo
const byte Ultrasonico = 3;      // Pin que activa Sensor Ultrasonico
const byte interruptPin = 2;     // Pin Interrupción de Sensor Ultrasonico
volatile byte state = LOW;       // Variable de estado de led Rojo
volatile long echo_start = 0;    // Guarda inicio de interrupción Ultrasonico
volatile long echo_end = 0;      // Guarda final de interrupción Ultrasonico
volatile long echo_dura = 0;     // Duración - Diferencia entre final e inicio
volatile int tiempoTrigger = 0;  // Count down counter to trigger pulse time
#define TIMER_uS 50              // 50 muS duración overflow timer1
#define TICK_CONT 2000           // 100 mS total de timer1 ticks 
bool estadoUltrasonico = true;   // Set estado de sensor ultrasonico
///////////////////////////////////
RH_NRF24 nrf24(9, 10);           // Instancia de transceiver
String radioData = "";           // Datos de transceiver
uint8_t checkBien[] = ":", changeSensor[] = "s";

void setup () {
  /////////////////////////// Alimentacion RTC
  /*pinMode(A2, OUTPUT);     // GND directo de Microcontrolador
  digitalWrite(A2, LOW);     //
  pinMode(A3, OUTPUT);       // Vcc directo de Microcontrolador
  digitalWrite(A3, HIGH);    /*/
  pinMode(A2, OUTPUT);       // GND directo de Microcontrolador
  digitalWrite(A2, HIGH);    //
  pinMode(A3, OUTPUT);       // Vcc directo de Microcontrolador
  digitalWrite(A3, LOW);     //
  /////////////////////////////////////////
  pinMode(ledPin, OUTPUT);               // Pin salida LED
  pinMode(Ultrasonico, OUTPUT);          // Pin salida Ultrasonico
  pinMode(interruptPin, INPUT_PULLUP);   // Pin entrada con PullUp para Ultrasonico
  /////////////////////////////////////////
  //Wire.begin();        // Inicia el puerto I2C, 1 address master
  Serial.begin(57600);   // Establece la velocidad de datos del puerto serie
  while (!Serial);       // Esperar Serial            //
  d_serial.reserve(70);  // Reserva memoria para datos RS232
  if (!nrf24.init()) Serial.println("Inicio radio fallido.");

  Serial.println("  --------------------------------------->");        //Mensaje de bienvenida
  Serial.println("         Bienvenido a la carrera!");                 //
  Serial.println("  --------------------------------------->");        //
  Serial.println("Hora actual en sistema:");                           //
  Serial.print(rtc.getDOWStr());
  Serial.print(" ");
  Serial.print(rtc.getDateStr());
  Serial.print(" ");
  Serial.println(rtc.getTimeStr());
  Serial.print('.');
  //printTime();                                                          //

  digitalWrite(ledPin, LOW);
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
}
void loop() {
  serialEvent();
  radioEvent();
}
void serialEvent() {
  //bool ajustando = false;
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    d_serial += inChar;
    switch (inChar) {
      case 's':
        se = (d_serial.substring(0, d_serial.length() - 1)).toInt();
        d_serial = "";
        break;
      case 'm':
        mi = (d_serial.substring(0, d_serial.length() - 1)).toInt();
        d_serial = "";
        break;
      case 'h':
        ho = (d_serial.substring(0, d_serial.length() - 1)).toInt();
        d_serial = "";
        break;
      case 'w':
        wd = (d_serial.substring(0, d_serial.length() - 1)).toInt();
        d_serial = "";
        break;
      case 'd':
        di = (d_serial.substring(0, d_serial.length() - 1)).toInt();
        d_serial = "";
        break;
      case 'M':
        me = (d_serial.substring(0, d_serial.length() - 1)).toInt();
        d_serial = "";
        break;
      case 'y':
        an = (d_serial.substring(0, d_serial.length() - 1)).toInt();
        //an = an + cienAnio;
        d_serial = "";
        //ajustando = setTime((byte)se, (byte)mi, (byte)ho, (byte)wd, (byte)di, (byte)me, (byte)an);
        //if (ajustando) {
          rtc.setDOW(wd-1);
          rtc.setTime(ho,mi,se);
          rtc.setDate(di,me,an+2000);
          Serial.print("Hora ajustada: ");
          Serial.println(p2dig(di) + "-" + p2dig(me) + "-" + p2dig(an) + " " + p2dig(ho) + ":" + p2dig(mi) + ":" + p2dig(se));
          Serial.print('.');
        //}
        break;
      case 'e':
        adSensor();
        d_serial = "";
        break;
      case 'i':
        enviarDatos(changeSensor);
        d_serial = "";
        break;
      case '.':
        Serial.print("Fin OK.\n");
        d_serial = "";
        break;
      case ':':
        Serial.print("Solicitando: ");
        enviarDatos(checkBien);
        d_serial = "";
        break;
    }
  }
}
void radioEvent() {
  if (nrf24.available()) //En espera de mensajes
  {
    uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (nrf24.recv(buf, &len))
    {
      radioData = (char*)buf;
      if (radioData == ":") {
        Serial.println("Equipo de inicio OK.");
        radioData = "";
      }
      if (radioData == "a") {
        Serial.println("Sensor inicio ACTIVADO.");
        radioData = "";
      }
      if (radioData == "d") {
        Serial.println("Sensor inicio DESACTIVADO.");
        radioData = "";
      }
      if (radioData == "u") {
        String infoSend =  String(rtc.getTimeStr())+"<i>";
        Serial.println(infoSend);
        radioData = "";
      }

    }
    else
    {
      Serial.println("Error en lectura de datos.");
    }
  }
}

void enviarDatos(uint8_t texto[]) {
  nrf24.send(texto, sizeof(texto));
  nrf24.waitPacketSent();
}

void adSensor() {
  if (estadoUltrasonico) {
    Timer1.initialize(TIMER_uS);                        // 50uF periodo
    Timer1.attachInterrupt(flash);                      // Ligar overfow a la funcion flash
    attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);     //Asignar como interrupción
    Serial.println("Sensor final ACTIVADO.");
  } else {
    detachInterrupt(digitalPinToInterrupt(interruptPin));
    Timer1.stop();
    Serial.println("Sensor final DESACTIVADO.");
  }
  estadoUltrasonico = !estadoUltrasonico;
}

void blink() {                             //Función llamada en interrupción externa
  switch (digitalRead(interruptPin)) {
    case HIGH:                                      // High so must be the start of the echo pulse
      echo_end = 0;                                 // Clear the end time
      echo_start = micros();                        // Save the start time
      //Serial.println(String(echo_start,DEC) + " s");
      break;

    case LOW:                                       // Low so must be the end of hte echo pulse
      echo_end = micros();                          // Save the end time
      //Serial.println(String(echo_end,DEC) + " e");
      echo_dura = echo_end - echo_start;            // Calculate the pulse duration
      if (echo_dura < 3000) {
        detachInterrupt(digitalPinToInterrupt(interruptPin));
        Timer1.stop();
        digitalWrite(ledPin, LOW);             //Cambiar estado de led
        estadoUltrasonico = !estadoUltrasonico;
        String infoSend =  String(rtc.getTimeStr())+"<f>";
        Serial.println(infoSend);
      }
      break;
  }
  state = !state;
  digitalWrite(ledPin, state);             //Cambiar estado de led
}

void flash() {                             //Función llamada en iterrupción Timer2
  static volatile int estado = 0;

  if (!(--tiempoTrigger))                        // Cuenta hata 100mS
  { // Time out - Inicia pulso Trigger
    tiempoTrigger = TICK_CONT;                   // Recarga
    estado = 1;                                  // Cambiando estado 1, inicia el pulso
  }
  switch (estado)
  {
    case 0:                                      // Estado normal no hace nada
      break;
    case 1:                                      // Inicia el pulso
      digitalWrite(Ultrasonico, HIGH);           // Pone en estado alto el pulso
      estado = 2;                                // Iguala estado a 2
      break;
    case 2:                                      // Completa el pulso
    default:                                     //
      digitalWrite(Ultrasonico, LOW);            // Pone en estado bajo el pulso
      estado = 0;                                // Regresa el estado a normal 0
      break;
  }
}

String p2dig(int number) {                 //Funcion auxiliar para imprimir siempre 2 digitos
  String dato = "";
  if (number >= 0 && number < 10) {
    dato = '0' + String(number, DEC);
  } else dato = String(number, DEC);
  return dato;
}

/*///////////////////////////////////////////////////////////////////  RTC  ///////////////////////////////////////*/
/*bool setTime(byte sec, byte minu, byte hor, byte wdi, byte dia, byte mes, byte ani) {
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.write(decToBcd(sec));
  Wire.write(decToBcd(minu));
  Wire.write(decToBcd(hor));
  Wire.write(decToBcd(wdi));
  Wire.write(decToBcd(dia));
  Wire.write(decToBcd(mes));
  Wire.write(decToBcd(ani));
  Wire.write(byte(0));
  //Wire.endTransmission();
  if (Wire.endTransmission() != 0) return false;
  else return true;
}
void printTime() {
  char buffer[3];
  const char* AMPM = 0;
  readTime();
  Serial.print(days[wd - 1]);
  Serial.print(" ");
  Serial.print(mes[me - 1]);
  Serial.print(" ");
  Serial.print(di);
  Serial.print(", 20");
  Serial.print(an);
  Serial.print(" ");
  if (ho > 12) {
    ho -= 12;
    AMPM = " PM";
  }
  else AMPM = " AM";
  Serial.print(ho);
  Serial.print(":");
  sprintf(buffer, "%02d", mi);
  Serial.print(buffer);
  Serial.println(AMPM);
}
void readTime() {
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.endTransmission();
  Wire.requestFrom(DS1307, 7);
  se = bcdToDec(Wire.read());
  mi = bcdToDec(Wire.read());
  ho = bcdToDec(Wire.read());
  wd = bcdToDec(Wire.read());
  di = bcdToDec(Wire.read());
  me = bcdToDec(Wire.read());
  an = bcdToDec(Wire.read());
}
String cadenaTiempo() {                    //Formato de hora que se entregará
  return p2dig(ho) + ':' + p2dig(mi) + ':' + p2dig(se);
}

///////////////////////////////////////////////////////////////////////////////////////// EXTRAS <-------------------------
byte decToBcd(byte val) {                  //Decimal to BCD
  return ((val / 10 * 16) + (val % 10));   //
}                                          //
byte bcdToDec(byte val) {                  //BCD to decimal
  return ((val / 16 * 10) + (val % 16));   //
}*/
