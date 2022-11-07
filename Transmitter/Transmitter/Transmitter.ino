#include "TimerOne.h"
#include "RingBuf.h"

const uint16_t PERIOD_MICROS = 450;
const uint16_t TIME_ON_MICROS = 45;

const uint16_t TRANSMIT_PIN = 8;
const uint16_t DEBUG_PIN = 9;

const size_t RING_BUF_SIZE = 1024;

volatile uint8_t count = 0;
RingBuf<bool, RING_BUF_SIZE> buf;

void time() {
  if (count == 0) {
    bool bit;
    if (buf.pop(bit)) {
      digitalWrite(TRANSMIT_PIN, bit);
      digitalWrite(DEBUG_PIN, HIGH);
    }
  } else {
    digitalWrite(TRANSMIT_PIN, LOW);
    digitalWrite(DEBUG_PIN, LOW);
  }
  ++count;
  count %= PERIOD_MICROS/TIME_ON_MICROS;
}

void sendMessage(String message) {
  const uint8_t start_byte = (1 << 3) - 1;
  sendRawByte(start_byte);
  sendRawByte((uint8_t) message.length());
  for (size_t i = 0; i < message.length(); ++i)
    sendRawByte(message[i]);
}

void sendRawByte(char byte) {
  for (uint8_t mask = 1 << 7; mask != 0; mask >>= 1)
    buf.lockedPush(byte & mask);
  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(TRANSMIT_PIN, OUTPUT);
  pinMode(DEBUG_PIN, OUTPUT);
  Timer1.initialize(TIME_ON_MICROS);
  Timer1.attachInterrupt(time);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    String message = Serial.readString();
    Serial.println("Sending message");
    sendMessage(message);
  }
}
