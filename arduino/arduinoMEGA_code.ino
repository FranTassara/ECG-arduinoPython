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
volatile bool bufferFull = false;

void swapBuffers() {
  uint16_t* temp = currentBuffer;
  currentBuffer = sendBuffer;
  sendBuffer = temp;
}

void setup() {
  Serial.begin(BAUD_RATE);

  // Configurar Timer3 para ~250Hz (cada 4ms aprox)
  cli();
  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3 = 0;
  //OCR3A = 999;             // 16 MHz / 64 / (999 + 1) = 250 Hz
  OCR3A = 499;  // Para 500 Hz con prescaler 64
  TCCR3B |= (1 << WGM32);   // Modo CTC
  TCCR3B |= (1 << CS31) | (1 << CS30); // prescaler 64
  TIMSK3 |= (1 << OCIE3A);  // Habilitar interrupción Timer3 compare
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

//void loop() {
//  if (bufferFull) {
//    noInterrupts();
//    swapBuffers();
//    index = 0;
//    bufferFull = false;
//    interrupts();
//
//    // Enviar datos como texto al Serial Plotter
//    for (int i = 0; i < BUFFER_SIZE; i++) {
//      Serial.println(sendBuffer[i]);  // Formato compatible con Serial Plotter
//    }
//  }
//}

ISR(TIMER3_COMPA_vect) {
  if (index < BUFFER_SIZE) {
    currentBuffer[index++] = analogRead(ECG_PIN);
//    if (index >= BUFFER_SIZE) {
//      bufferFull = true;
//    }
  }
}
