// Pins
#define TEMP_INPUT A0
#define SHIFT_DATA 2
#define SHIFT_CLOCK 3
#define STORE 4

// Update period over which the temperature readings are averaged.
// This helps avoid fluctuations in the displayed temperature.
#define DELAY_SECONDS 60

// Measured voltage (in mV) of the internal analog voltage reference.
// The internal reference is used (rather than the 5V reference) to better
// match the output voltage range of the TMP36 (0.1V to 1.75V).
const float INTERNAL_VOLTAGE_MV = 1053.0;
// Pre-compute the ADC step value.
const float analogStep = INTERNAL_VOLTAGE_MV / 1023.0;

// Active-low bit patterns for decimal numbers on the 7-segment displays.
// Bit order: EDC.BAFG
const byte segments[10] = {
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

// Bit pattern to enable the decimal point.
const byte decimalPoint = 0b11101111;

// Pulse the shift registers' store clock to move the shifted-in data to the
// registers' output stage.
void pulseStoreClock() {
    digitalWrite(STORE, HIGH);
    delay(1);
    digitalWrite(STORE, LOW);
}

// Displays a specific pattern on the 7-segment displays until a real
// temperature reading is available.
//
// The shift register contents are unknown on power-up, so this avoids
// weird-looking garbage that makes it look like there's been a fault.
void writeResetPattern() {
    // X-like pattern
    const byte resetSequence[] = {
        // Bit order: EDC.BAFG
        // DA
        0b10111011,
        // G
        0b11111110,
        // DA
        0b10111011,
    };

    for (int idx = 0; idx < 3; idx += 1) {
        shiftOut(SHIFT_DATA, SHIFT_CLOCK, LSBFIRST, resetSequence[idx]);
    }
    pulseStoreClock();
}

// Read temperature from the TMP36.
int readTemperatureInTenthsC() {
    // milliVolt reading from the TMP36
    float mv = analogRead(TEMP_INPUT) * analogStep;
    // Remove the 500mV offset to produce the temperature value in 1/10 °C
    float tenths = mv - 500;
    return (int)tenths;
}

// Intermediate temperature readings (one per second of delay).
int readingBuffer[DELAY_SECONDS];

// Average the readings in readingBuffer.
//
// Returned value is in 10ths of a degree.
int averageOfReadings() {
    int average = 0;
    for (int idx = 0; idx < DELAY_SECONDS; idx += 1) {
        average += readingBuffer[idx];
    }
    return average / DELAY_SECONDS;
}

void setup() {
    analogReference(INTERNAL);      // Internal ~1.1V analog reference.

    pinMode(TEMP_INPUT, INPUT);     // TMP36 analog input (0.1-2.0V).

    pinMode(SHIFT_DATA, OUTPUT);    // 10ths place shift register data input.
    pinMode(SHIFT_CLOCK, OUTPUT);   // Shift registers' clock input.
    pinMode(STORE, OUTPUT);         // Store clock to move data to output stage.
    digitalWrite(STORE, LOW);       // Ensure store clock is low.

    writeResetPattern();            // Initialise the display to a known pattern.
}

void loop() {
    for (int idx = 0; idx < DELAY_SECONDS; idx += 1) {
        readingBuffer[idx] = readTemperatureInTenthsC();
        delay(1000);
    }

    int tenthsC = averageOfReadings();
    byte tensPlace = tenthsC / 100;
    byte onesPlace = tenthsC / 10 % 10;
    byte tenthsPlace = tenthsC % 10;

    // Write out the digits in order left-right.
    shiftOut(SHIFT_DATA, SHIFT_CLOCK, LSBFIRST, segments[tensPlace]);
    shiftOut(SHIFT_DATA, SHIFT_CLOCK, LSBFIRST, segments[onesPlace] & decimalPoint);
    shiftOut(SHIFT_DATA, SHIFT_CLOCK, LSBFIRST, segments[tenthsPlace]);
    // Move data into output stage to display.
    pulseStoreClock();
}
