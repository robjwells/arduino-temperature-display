#define TEMP_INPUT A0
#define SHIFT_DATA 2
#define SHIFT_CLOCK 3
#define STORE 4

const unsigned long DELAY = 20000;

// Measured voltage (in mV) of the INTERNAL analog voltage reference.
const float INTERNAL_VOLTAGE_MV = 1053.0;

const byte segments[10] = {
    // Bit order: EDC.BAFG
    // Active low.

    // 0: ABCDEF
    0b00010001,
    // 1: BC
    0b11010111,
    // 2: ABGED
    0b00110010,
    // 3: ABGCD
    0b10010010,
    // 4: FGBC
    0b11010100,
    // 5: AFGCD
    0b10011000,
    // 6: AFEDCG
    0b00011000,
    // 7: ABC
    0b11010011,
    // 8: ABCDEFG
    0b00010000,
    // 9: ABCDFG
    0b10010000,
};

byte setDecimalPoint(byte pattern) {
    // Segments are active low.
    byte pointOnly = 0b11101111;
    return pattern & pointOnly;
}

void setup() {
  // Use the internal ~1.1 volt reference
  // Actually 1.053v on my Metro Mini.
  analogReference(INTERNAL);
  pinMode(TEMP_INPUT, INPUT);
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLOCK, OUTPUT);
  pinMode(STORE, OUTPUT);
  digitalWrite(STORE, LOW);
}

int readTemperatureInTenthsC() {
  int raw = analogRead(TEMP_INPUT);
  float mv = raw * (INTERNAL_VOLTAGE_MV / 1023.0);
  float tenths = mv - 500;
  return (int) tenths;
}

void loop() {
  int tenthsC = readTemperatureInTenthsC();
  byte tensPlace = tenthsC / 100;
  byte onesPlace = tenthsC / 10 % 10;
  byte tenthsPlace = tenthsC % 10;
  
  // Write out the digits in order left-right.
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, LSBFIRST, segments[tensPlace]);
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, LSBFIRST, setDecimalPoint(segments[onesPlace]));
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, LSBFIRST, segments[tenthsPlace]);

  // Pulse the shift registers' store clock.
  digitalWrite(STORE, HIGH);
  delay(1);
  digitalWrite(STORE, LOW);
  
  delay(DELAY);
}
