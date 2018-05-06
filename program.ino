#include <DS1302.h>
#include <Time.h>
#include <stdio.h>

#define A1 11 // blue
#define B2 10 // green
#define C3 9  // orange
#define D4 6  // red
#define E5 5  // yellow
#define F6 3  // brown

#define DATAPIN 2
#define LATCHPIN 4
#define CLOCKPIN 7

#define DOTSPIN 12

#define TIMECHANGEPIN 16

#define SWITCHPOS1 17
#define SWITCHPOS2 18

// CE, IO, CLK
DS1302 RTC(13, 14, 15);

int segments[29][2] = {
    {A1, B2},
    {B2, A1},
    {B2, C3},
    {C3, B2},
    {C3, D4},
    {D4, C3},
    {D4, E5},
    // -----
    {E5, D4},
    {E5, F6},
    {F6, E5},
    {A1, C3},
    {C3, A1},
    {B2, D4},
    {D4, B2},
    // -----
    {C3, E5},
    {E5, C3},
    {D4, F6},
    {F6, D4},
    {A1, D4},
    {D4, A1},
    {B2, E5},
    // -----
    {E5, B2},
    {C3, F6},
    {F6, C3},
    {A1, E5},
    {E5, A1},
    {B2, F6},
    {F6, B2}};

int digit_1[7][2] = {{A1, B2}, {B2, A1}, {B2, C3}, {C3, B2}, {C3, D4}, {D4, C3}, {D4, E5}};
int digit_2[7][2] = {{E5, D4}, {E5, F6}, {F6, E5}, {A1, C3}, {C3, A1}, {B2, D4}, {D4, B2}};
int digit_3[7][2] = {{C3, E5}, {E5, C3}, {D4, F6}, {F6, D4}, {A1, D4}, {D4, A1}, {B2, E5}};
int digit_4[7][2] = {{E5, B2}, {C3, F6}, {F6, C3}, {A1, E5}, {E5, A1}, {B2, F6}, {F6, B2}};

int lastSegment[2] = {A1, B2};

void setup()
{
    Serial.begin(9600);

    pinMode(SWITCHPOS1, INPUT_PULLUP);
    pinMode(SWITCHPOS2, INPUT_PULLUP);

    pinMode(LATCHPIN, OUTPUT);
    pinMode(CLOCKPIN, OUTPUT);
    pinMode(DATAPIN, OUTPUT);

    pinMode(DOTSPIN, OUTPUT);

    pinMode(TIMECHANGEPIN, INPUT_PULLUP);

    RTC.writeProtect(false);
    RTC.halt(false);

    // Sunday, September 25, 2018 at 13:30:50.
    Time t(2018, 9, 25, 13, 30, 50, Time::kTuesday);
    RTC.time(t);
}

int seconds = 0;

int hoursDigit1 = 0;
int hoursDigit2 = 0;
int minutesDigit1 = 0;
int minutesDigit2 = 0;
int secondsDigit1 = 0;
int secondsDigit2 = 0;

int switchPosition = 0;

void loop()
{
    getTime();

    readTimeChangeButton();

    displayTime();
    blinkDots();
    displaySeconds();
}

void displayTime()
{
    bool hoursDigit1Positions[7];
    getDigitPositions(hoursDigit1, hoursDigit1Positions);
    light_digit(digit_1, hoursDigit1Positions);

    bool hoursDigit2Positions[7];
    getDigitPositions(hoursDigit2, hoursDigit2Positions);
    light_digit(digit_2, hoursDigit2Positions);

    bool minutesDigit1Positions[7];
    getDigitPositions(minutesDigit1, minutesDigit1Positions);
    light_digit(digit_3, minutesDigit1Positions);

    bool minutesDigit2Positions[7];
    getDigitPositions(minutesDigit2, minutesDigit2Positions);
    light_digit(digit_4, minutesDigit2Positions);
}

void blinkDots()
{
    if (seconds % 2 == 0)
    {
        digitalWrite(DOTSPIN, HIGH);
    }
    else
    {
        digitalWrite(DOTSPIN, LOW);
    }
}

void displaySeconds()
{
    // 12 o'clock led
    // 01110100
    write_to_shift_register(116);

    int firstColumnValue = 0;
    int allColumnsValue = 0;
    int currendColumnValue = 0;

    // first col
    int secondsDigit = secondsDigit2;
    if (secondsDigit > 4)
    {
        secondsDigit = secondsDigit - 5;
    }

    if (secondsDigit == 1)
    {
        // 01111000
        firstColumnValue = 120;
    }
    else if (secondsDigit == 2)
    {
        // 00111000
        firstColumnValue = 56;
    }
    else if (secondsDigit == 3)
    {
        // 00011000
        firstColumnValue = 24;
    }
    else if (secondsDigit == 4)
    {
        // 00001000
        firstColumnValue = 8;
    }

    // fourth
    if (seconds >= 40)
    {
        // 00000110
        allColumnsValue = 6;
        if (seconds >= 55)
        {
            // 00000001
            currendColumnValue = 1;
        }
        else if (seconds >= 50)
        {
            // 00010001
            currendColumnValue = 17;
        }
        else if (seconds >= 45)
        {
            // 00110001
            currendColumnValue = 49;
        }
        else
        {
            // 01110001
            currendColumnValue = 113;
        }
    }
    // third
    else if (seconds >= 20)
    {
        // 00000100
        allColumnsValue = 4;
        if (seconds >= 35)
        {
            // 00000010
            currendColumnValue = 2;
        }
        else if (seconds >= 30)
        {
            // 00010010
            currendColumnValue = 18;
        }
        else if (seconds >= 25)
        {
            // 00110010
            currendColumnValue = 50;
        }
        else
        {
            // 01110010
            currendColumnValue = 114;
        }
    }
    //second col
    else if (seconds >= 5)
    {
        if (seconds >= 15)
        {
            // 00000100
            currendColumnValue = 4;
        }
        else if (seconds >= 10)
        {
            // 00010100
            currendColumnValue = 20;
        }
        else
        {
            // 00110100
            currendColumnValue = 52;
        }
    }
    writeSecondsDisplayValue(firstColumnValue, allColumnsValue, currendColumnValue);
}

void writeSecondsDisplayValue(int firstColumnValue, int allColumnsValue, int currendColumnValue)
{
    write_to_shift_register(firstColumnValue);
    write_to_shift_register(allColumnsValue);
    write_to_shift_register(currendColumnValue);

    write_to_shift_register(firstColumnValue);
    write_to_shift_register(allColumnsValue);
    write_to_shift_register(currendColumnValue);

    write_to_shift_register(allColumnsValue);
    write_to_shift_register(firstColumnValue);
}

/*
  
  Positions

      0
-------------
|           |
|1         2|
|     3     |
-------------
|           |
|4         5|
|     6     |
-------------
*/

void getDigitPositions(int digit, bool positions[7])
{
    positions[0] = false;
    positions[1] = false;
    positions[2] = false;
    positions[3] = false;
    positions[4] = false;
    positions[5] = false;
    positions[6] = false;

    if (digit == 0)
    {
        positions[0] = true;
        positions[1] = true;
        positions[2] = true;
        positions[4] = true;
        positions[5] = true;
        positions[6] = true;
    }

    if (digit == 1)
    {
        positions[2] = true;
        positions[5] = true;
    }

    if (digit == 2)
    {
        positions[0] = true;
        positions[2] = true;
        positions[3] = true;
        positions[4] = true;
        positions[6] = true;
    }

    if (digit == 3)
    {
        positions[0] = true;
        positions[2] = true;
        positions[3] = true;
        positions[5] = true;
        positions[6] = true;
    }

    if (digit == 4)
    {
        positions[1] = true;
        positions[2] = true;
        positions[3] = true;
        positions[5] = true;
    }

    if (digit == 5)
    {
        positions[0] = true;
        positions[1] = true;
        positions[3] = true;
        positions[5] = true;
        positions[6] = true;
    }

    if (digit == 6)
    {
        positions[0] = true;
        positions[1] = true;
        positions[3] = true;
        positions[4] = true;
        positions[5] = true;
        positions[6] = true;
    }

    if (digit == 7)
    {
        positions[0] = true;
        positions[2] = true;
        positions[5] = true;
    }

    if (digit == 8)
    {
        positions[0] = true;
        positions[1] = true;
        positions[2] = true;
        positions[3] = true;
        positions[4] = true;
        positions[5] = true;
        positions[6] = true;
    }

    if (digit == 9)
    {
        positions[0] = true;
        positions[1] = true;
        positions[2] = true;
        positions[3] = true;
        positions[5] = true;
    }
}

void light_segements_in_sequence()
{
    for (int i = 0; i < 28; i++)
    {
        reset_segment_lights();
        light_segment(segments[i]);
        delay(200);
    }
}

void reset_segment_lights()
{
    for (int i = 0; i < 27; i++)
    {
        pinMode(segments[i][0], INPUT);
        pinMode(segments[i][1], INPUT);
    }
}

void light_next_segment(int segmentPins[2])
{
    // turn off previous segment
    pinMode(lastSegment[0], INPUT);
    pinMode(lastSegment[1], INPUT);

    pinMode(segmentPins[0], OUTPUT);
    analogWrite(segmentPins[0], 255);

    pinMode(segmentPins[1], OUTPUT);
    digitalWrite(segmentPins[1], LOW);

    delayMicroseconds(500);

    // set previous segment
    lastSegment[0] = segmentPins[0];
    lastSegment[1] = segmentPins[1];
}

void light_segment(int segmentPins[2])
{
    pinMode(segmentPins[0], OUTPUT);
    analogWrite(segmentPins[0], 255);

    pinMode(segmentPins[1], OUTPUT);
    digitalWrite(segmentPins[1], LOW);
    delay(1);
}

void light_digit(int digit[7][2], bool positions[7])
{
    for (int i = 0; i < 7; i++)
    {
        if (positions[i])
        {
            light_next_segment(digit[i]);
        }
    }
}

void write_to_shift_register(int data)
{
    digitalWrite(LATCHPIN, LOW);
    shiftOut(DATAPIN, CLOCKPIN, LSBFIRST, data);
    digitalWrite(LATCHPIN, HIGH);
}

void getDigits(int number, int &digit1, int &digit2)
{
    digit2 = number % 10;

    if (number < 10)
    {
        digit1 = 0;
    }
    else
    {
        digit1 = number;
        if (digit1 >= 100000000)
            digit1 /= 100000000;
        if (digit1 >= 10000)
            digit1 /= 10000;
        if (digit1 >= 100)
            digit1 /= 100;
        if (digit1 >= 10)
            digit1 /= 10;
    }
}

void getTime()
{
    Time t = RTC.time();

    seconds = t.sec;

    getDigits(t.hr, hoursDigit1, hoursDigit2);
    getDigits(t.min, minutesDigit1, minutesDigit2);
    getDigits(t.sec, secondsDigit1, secondsDigit2);
}

void increaseHours()
{
    Time t = RTC.time();
    int currentHour = t.hr;
    currentHour++;

    if (currentHour > 23)
    {
        currentHour = 0;
    }

    RTC.writeProtect(false);
    RTC.halt(false);

    Time newTime(t.yr, t.mon, t.date, currentHour, t.min, t.sec, t.day);
    RTC.time(newTime);
}

void increaseMinutes()
{
    Time t = RTC.time();
    int currentMinute = t.min;
    currentMinute++;

    if (currentMinute > 59)
    {
        currentMinute = 0;
    }

    RTC.writeProtect(false);
    RTC.halt(false);

    Time newTime(t.yr, t.mon, t.date, t.hr, currentMinute, t.sec, t.day);
    RTC.time(newTime);
}

void readTimeChangeButton()
{
    if (isTimeChangeButtonDown())
    {
        readSwitchPosition();
        if (switchPosition == 1)
        {
            increaseHours();
        }
        else if (switchPosition == 2)
        {
            increaseMinutes();
        }
        delay(100);
    }
}

bool isTimeChangeButtonDown()
{
    if (digitalRead(TIMECHANGEPIN) == LOW)
    {
        delay(10);
        if (digitalRead(TIMECHANGEPIN) == LOW)
        {
            return true;
        }
    }

    return false;
}

void readSwitchPosition()
{
    if (isSwitchInPosition(SWITCHPOS1))
    {
        switchPosition = 1;
    }
    else if (isSwitchInPosition(SWITCHPOS2))
    {
        switchPosition = 2;
    }
    else
    {
        switchPosition = 3;
    }
}

bool isSwitchInPosition(int position)
{
    if (digitalRead(position) == LOW)
    {
        delay(10);
        if (digitalRead(position) == LOW)
        {
            return true;
            delay(10);
        }
    }

    return false;
}