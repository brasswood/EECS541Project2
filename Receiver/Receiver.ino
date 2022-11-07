#include "TimerOne.h"
#include "RingBuf.h"

const uint16_t PERIOD_MICROS = 450;

const uint16_t RECEIVE_PIN = 2;
const uint16_t DEBUG_PIN = 3;

const size_t RING_BUF_SIZE = 1024;

enum State {
  IDLE,
  SIZE,
  READ,
};

volatile State state = State::IDLE;
volatile bool flag = false;
RingBuf<char, RING_BUF_SIZE> buf;
volatile uint8_t bit_idx = 7; // for the size only
volatile char current_byte = 0;
volatile uint8_t start_bits_seen = 0;
volatile uint8_t msg_size = 0;


void handleBit(uint8_t bit) {
  switch (state) {
    case IDLE:
      if (bit) {
        ++start_bits_seen;
        if (start_bits_seen == 3) {
          state = State::SIZE;
          start_bits_seen = 0;
        }
      } else {
        start_bits_seen = 0;
      }
      break;
    case SIZE:
      msg_size |= (bit << bit_idx);
      if (bit_idx == 0) {
        bit_idx = 7;
        state = State::READ;
      } else {
        --bit_idx;
      }
      break;
    case READ:
      current_byte |= (bit << bit_idx);
      if (bit_idx == 0) {
        buf.push(current_byte);
        current_byte = 0;
        bit_idx = 7;
        --msg_size;
        if (msg_size == 0)
          state = State::IDLE;
      } else {
        --bit_idx;
      }
      break;
  }
}

void onFallingEdge() {
  flag = true;
}

void onTimeout() {
  digitalWrite(DEBUG_PIN, LOW);
  if (flag)
    handleBit(1);
  else
    handleBit(0);
  flag = false;
  digitalWrite(DEBUG_PIN, HIGH);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(RECEIVE_PIN, INPUT);
  pinMode(DEBUG_PIN, OUTPUT);
  digitalWrite(DEBUG_PIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(RECEIVE_PIN), onFallingEdge, FALLING);
  Timer1.initialize(PERIOD_MICROS);
  Timer1.attachInterrupt(onTimeout);
  Timer1.start();
}

void loop() {
  char c;
  if (buf.lockedPop(c)) {
    Serial.print(c);
    // for (uint8_t mask = 1 << 7; mask != 0; mask >>= 1)
    //   Serial.print((c & mask) ? "1" : "0");
    // Serial.println();
  }
}
