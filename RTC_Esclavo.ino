// Módulo esclavo, Sensor Ultrasónico, Wireless data
// ICE. Oscar Aguilar Cruz
// 
// Comunicación serial/inalambrica con módulo esclavo NRF24 de 100 mW
// Interrupts: Timer, External, Data
// Sensor Ultrasónico.

#include <SPI.h>
#include <RH_NRF24.h>
#include <TimerOne.h>     // Timer1

RH_NRF24 nrf24(9, 10);    // Instancia del radio y declaracion de pines de datos
String radioData = "";
uint8_t respBien[] = ":", activa[] = "a", desact[] = "d", pasoBici[] = "u";

/////////////////////////////////// SENSOR ULTRASONICO  <-----------------------------------------------------------------------------------------
const byte Ultrasonico = 4;      // Pin que activa Sensor Ultrasonico
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

void setup()
{
  nrf24.init();
  pinMode(Ultrasonico, OUTPUT);          // Pin salida Ultrasonico
  pinMode(interruptPin, INPUT_PULLUP);   // Pin entrada con PullUp para Ultrasonico
}

void loop()
{
  if (nrf24.waitAvailableTimeout(500))
  {
    uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (nrf24.recv(buf, &len))
    {
      radioData = (char*)buf;

      if (radioData == ":") {
        enviarDatos(respBien);
        radioData = "";
      }
      if (radioData == "s") {
        adSensor();
        radioData = "";
      }
    }
  }
}
void blink() {                             //Función llamada en interrupción externa
  switch (digitalRead(interruptPin)) {
    case HIGH:                                      // High so must be the start of the echo pulse
      echo_end = 0;                                 // Clear the end time
      echo_start = micros();                        // Save the start time
      break;

    case LOW:                                       // Low so must be the end of hte echo pulse
      echo_end = micros();                          // Save the end time
      echo_dura = echo_end - echo_start;            // Calculate the pulse duration
      if (echo_dura < 3000) {
        adSensor();
        enviarDatos(pasoBici);
      }
      break;
  }
}
void flash() {                             //Función llamada en iterrupción Timer2
  static volatile int estado = 0;

  if (!(--tiempoTrigger))                        // Cuenta hata 100mS
  {
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

void enviarDatos(uint8_t texto[]) {
  nrf24.send(texto, sizeof(texto));
  nrf24.waitPacketSent();
}
void adSensor() {
  if (estadoUltrasonico) {
    Timer1.initialize(TIMER_uS);                        // 50uF periodo
    Timer1.attachInterrupt(flash);                      // Ligar overfow a la funcion flash
    attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);     //Asignar como interrupción
    enviarDatos(activa);
  } else {
    detachInterrupt(digitalPinToInterrupt(interruptPin));
    Timer1.stop();
    enviarDatos(desact);
  }
  estadoUltrasonico = !estadoUltrasonico;
}
