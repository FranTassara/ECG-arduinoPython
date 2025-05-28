#include <Ticker.h>

#define ECG_PIN A0
#define BAUD_RATE 9600
#define BUFFER_SIZE 100
#define HANDSHAKE_CHAR 'S'

volatile uint16_t buffer1[BUFFER_SIZE];
volatile uint16_t buffer2[BUFFER_SIZE];

volatile uint16_t* currentBuffer = buffer1;
volatile uint16_t* sendBuffer = buffer2;

volatile int index = 0;
volatile bool readyToSend = false;
volatile bool sending = false;

Ticker timer;

void swapBuffers() {
  uint16_t* temp = currentBuffer;
  currentBuffer = sendBuffer;
  sendBuffer = temp;
}

void readECG() {
  if (index < BUFFER_SIZE) {
    currentBuffer[index++] = analogRead(ECG_PIN);
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  timer.attach_ms(4, readECG);  // Llama a readECG cada ~4 ms (~250 Hz)
}

void loop() {
  if (Serial.available() > 0) {
    char incoming = Serial.read();
    if (incoming == HANDSHAKE_CHAR && !sending) {
      readyToSend = true;
    }
  }

  if (readyToSend && index >= BUFFER_SIZE) {
    noInterrupts();
    swapBuffers();
    index = 0;
    sending = true;
    readyToSend = false;
    interrupts();

    Serial.write((uint8_t*)sendBuffer, sizeof(uint16_t) * BUFFER_SIZE);

    sending = false;
  }
}
