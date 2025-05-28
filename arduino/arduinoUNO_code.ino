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

void swapBuffers() {
  uint16_t* temp = currentBuffer;
  currentBuffer = sendBuffer;
  sendBuffer = temp;
}

void setup() {
  Serial.begin(BAUD_RATE);

  // Timer1 configuración para ~250Hz (cada 4ms aprox)
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 999;            // 16MHz / (64 * 1000) = 250 Hz
  //OCR1A = 499;            // 16MHz / (64 * 500) = 500 Hz
  TCCR1B |= (1 << WGM12);  // Modo CTC
  TCCR1B |= (1 << CS11) | (1 << CS10); // prescaler 64
  TIMSK1 |= (1 << OCIE1A); // Enable timer compare interrupt
  sei();
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

ISR(TIMER1_COMPA_vect) {
  if (index < BUFFER_SIZE) {
    currentBuffer[index++] = analogRead(ECG_PIN);
  }
}
