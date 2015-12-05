
//Debug
//#define DEBUG
#define DEBUG_SAMPLE            25

//ADC
#define V_GAIN                  14.34
#define T_GAIN                  488.28125
#define V_THRESHOLD             250
#define T_THRESHOLD             2000

//Volt steps
#define VOLT_OFF                1261
#define VOLT_LOW                3226
#define VOLT_NORMAL             3958
#define VOLT_HIGH               5177
#define VOLT_TURBO              6969

//Temp steps
#define TEMP_OFF                20000
#define TEMP_MIN                25000
#define TEMP_MEAN               30000
#define TEMP_MAX                35000

//Timers
#define CHANGE_SPEED_DELAY      2000
#define CHANGE_MODE_DELAY       5000
#define SAMPLE_DELAY            250
#define THRESHOLD_DELAY         2000
#define ADC_GUARD_TIME          1

//EEPROM address
#define SAVED_POWER             1
#define SAVED_LIGHT             2
#define SAVED_MODE              3
#define SAVED_RED               4
#define SAVED_GREEN             5
#define SAVED_BLUE              6

//States
#define POWER_OFF               0
#define POWER_ON                1
#define LIGHT_OFF               0
#define LIGHT_ON                1
#define LIGHT_RGB               1
#define LIGHT_HSV               2
#define LIGHT_GET_R             3
#define LIGHT_GET_G             4
#define LIGHT_GET_B             5
#define MODE_LOW                0
#define MODE_NORMAL             1
#define MODE_HIGH               2
#define MODE_TURBO              3
#define MODE_AUTO               4
#define MODE_MANUAL             5
#define FAN_OFF                 0
#define FAN_LOW_SPEED           1
#define FAN_NORMAL_SPEED        2
#define FAN_HIGH_SPEED          3
#define FAN_TURBO_SPEED         4
#define SET_WAIT                0
#define SET_HUE                 1
#define SET_SAT                 2
#define SET_BRI                 3
#define SET_WRITE               4
#define DISPLAY_OFF             0
#define DISPLAY_RED             1
#define DISPLAY_GREEN           2
#define DISPLAY_YELLOW          3
#define DISPLAY_BLUE            4
#define DISPLAY_WHITE           5
#define DISPLAY_FAST_BLINK_W    6
#define DISPLAY_FAST_BLINK_R    7
#define DISPLAY_HUE_LOOP        8
#define DISPLAY_SAT_LOOP        9
#define DISPLAY_BRI_LOOP        10
#define DISPLAY_AUTO_OFFSET     11
#define DISPLAY_GREY_LOOP       11
#define DISPLAY_BLUE_LOOP       12
#define DISPLAY_GREEN_LOOP      13
#define DISPLAY_YELLOW_LOOP     14
#define DISPLAY_RED_LOOP        15
#define DISPLAY_SINE_OFFSET     16
#define DISPLAY_SINE_QUIET      16
#define DISPLAY_SINE_FAST       17
#define DISPLAY_SINE_VFAST      18
#define DISPLAY_SINE_UFAST      19
#define DISPLAY_SINE_SFAST      20

// I/O Pins
#define IO_FAN_OFF              13
#define IO_FAN_LOW              2
#define IO_FAN_NORMAL           7
#define IO_FAN_HIGH             8
#define IO_FAN_TURBO            12
#define IO_LIGHT_R              9
#define IO_LIGHT_G              10
#define IO_LIGHT_B              11
#define IO_DISPLAY_R            6
#define IO_DISPLAY_G            5
#define IO_DISPLAY_B            3
#define IO_TEMP                 A0
#define IO_POT                  A1
#define IO_VOLT_FB              A2
#define IO_BUTTON               A3

#include <EEPROM.h>

unsigned char SetLightColor(unsigned char ModeVal = 0, unsigned char StCh = 0, unsigned char NdCh = 0, unsigned char RdCh = 0);

void setup() {
    unsigned char ButtonPressed = 0;
    unsigned char ButtonDebounced = 0;
    unsigned char ButtonFlag = 0;
    unsigned char SetState = 0;
    unsigned char ChangeStateFlag = 1;
    unsigned char HueVal = 0;
    unsigned char SatVal = 0;
    unsigned char BriVal = 0;
    unsigned int CurrentPot = 0;
    unsigned int PrevPot = 0;
    int DiffPot = 0;
    unsigned long DebounceTime = 0;
    unsigned long WriteTime = 0;
    #ifdef DEBUG
    static unsigned long DebugTime = 0;
    Serial.begin(115200);
    #endif

    pinMode(IO_FAN_OFF, OUTPUT);
    digitalWrite(IO_FAN_OFF, HIGH);
    pinMode(IO_FAN_LOW, OUTPUT);
    pinMode(IO_FAN_NORMAL, OUTPUT);
    pinMode(IO_FAN_HIGH, OUTPUT);
    pinMode(IO_FAN_TURBO, OUTPUT);
    pinMode(IO_BUTTON, INPUT);
    pinMode(IO_DISPLAY_R, OUTPUT);
    pinMode(IO_DISPLAY_G, OUTPUT);
    pinMode(IO_DISPLAY_B, OUTPUT);
    pinMode(IO_LIGHT_R, OUTPUT);
    pinMode(IO_LIGHT_G, OUTPUT);
    pinMode(IO_LIGHT_B, OUTPUT);
    //Setup mode
    if ((digitalRead(IO_BUTTON) == HIGH) && (analogRead(IO_POT) > 1000)) {
        while(digitalRead(IO_BUTTON));
        do {
            CurrentPot = analogRead(IO_POT);
            if (digitalRead(IO_BUTTON) == HIGH) {
                if (ButtonPressed) {
                    if (ButtonDebounced == 0) if ((DebounceTime + 50) < millis()) ButtonDebounced = 1;
                }
                else {
                    ButtonPressed = 1;
                    DebounceTime = millis();
                }
            }
            else {
                ButtonFlag = 0;
                if ((ButtonPressed) && (ButtonDebounced)) {
                    DebounceTime = millis() - DebounceTime;
                    if ((DebounceTime > 100) && (DebounceTime < 500)) ButtonFlag = 1;
                }
                ButtonPressed = 0;
                ButtonDebounced = 0;
            }
            switch (SetState) {
              case SET_WAIT:
                if (ChangeStateFlag) PrevPot = analogRead(IO_POT);
                ChangeStateFlag = 0;
                SetLightColor(LIGHT_OFF);
                SetDisplayRGB(DISPLAY_FAST_BLINK_W);
                DiffPot = PrevPot - CurrentPot;
                DiffPot = abs(DiffPot);
                if (DiffPot > 10) {
                    SetState = SET_HUE;
                    ChangeStateFlag = 1;
                }
                break;
              case SET_HUE:
                SetDisplayRGB(DISPLAY_HUE_LOOP);
                HueVal = map(CurrentPot, 0, 1023, 0, 255);
                SetLightColor(LIGHT_HSV, HueVal, 255, 255);
                if (ButtonFlag) {
                    SetState = SET_SAT;
                    ChangeStateFlag = 1;
                }
                break;
              case SET_SAT:
                SetDisplayRGB(DISPLAY_SAT_LOOP);
                SatVal = map(CurrentPot, 0, 1023, 0, 255);
                SetLightColor(LIGHT_HSV, HueVal, SatVal, 255);
                if (ButtonFlag) {
                    SetState = SET_BRI;
                    ChangeStateFlag = 1;
                }
                break;
              case SET_BRI:
                SetDisplayRGB(DISPLAY_BRI_LOOP);
                BriVal = map(CurrentPot, 0, 1023, 0, 255);
                SetLightColor(LIGHT_HSV, HueVal, SatVal, BriVal);
                if (ButtonFlag) {
                    SetState = SET_WRITE;
                    ChangeStateFlag = 1;
                }
                break;
              case SET_WRITE:
                if (ChangeStateFlag) {
                    WriteTime = millis();
                    EEPROM.update(SAVED_RED, SetLightColor(LIGHT_GET_R, HueVal, SatVal, BriVal));
                    EEPROM.update(SAVED_GREEN, SetLightColor(LIGHT_GET_G, HueVal, SatVal, BriVal));
                    EEPROM.update(SAVED_BLUE, SetLightColor(LIGHT_GET_B, HueVal, SatVal, BriVal));
                }
                ChangeStateFlag = 0;
                SetDisplayRGB(DISPLAY_FAST_BLINK_R);
                if (WriteTime + 1500 < millis()) {
                    SetState = SET_WAIT;
                    ChangeStateFlag = 1;
                }
            }
        } while(1);
    }
}

void loop() {
    static unsigned char ButtonPressed = 0;
    static unsigned char ButtonDebounced = 0;
    static unsigned char ButtonMode = 0;
    static unsigned char DisplayBusy = 0;
    static unsigned char ModeState = EEPROM.read(SAVED_MODE);
    static unsigned char LightState = EEPROM.read(SAVED_LIGHT);
    static unsigned char PowerState = EEPROM.read(SAVED_POWER);
    static unsigned char CurrentSpeed = FAN_OFF;
    static unsigned char ChangeModeFlag = 0;
    unsigned char ManualSpeed = 0;
    unsigned long CurrentTemp = 0;
    static unsigned long ButtonTime = 0;
    static unsigned long SpeedTime = 0;
    static unsigned long ChangeModeTime = 0;
    #ifdef DEBUG
    static unsigned long DebugTime = 0;
    #endif

    if (PowerState > POWER_ON) {
        EEPROM.update(SAVED_POWER, POWER_ON);
        PowerState = EEPROM.read(SAVED_POWER);
    }
    if (LightState > LIGHT_ON) {
        EEPROM.update(SAVED_LIGHT, LIGHT_ON);
        LightState = EEPROM.read(SAVED_LIGHT);
    }
    if (ModeState > MODE_MANUAL) {
        EEPROM.update(SAVED_MODE, MODE_AUTO);
        ModeState = EEPROM.read(SAVED_MODE);
    }
    if (digitalRead(IO_BUTTON) == HIGH) {
        DisplayBusy = 1;
        if (ButtonPressed) {
            if (ButtonDebounced == 0) {
                if ((ButtonTime + 50) < millis()) ButtonDebounced = 1;
            }
            else {
                if ((ButtonTime + 110 < millis()) && (ButtonTime + 185 > millis())) SetDisplayRGB(DISPLAY_WHITE);
                else if ((ButtonTime + 760 < millis()) && (ButtonTime + 835 > millis())) SetDisplayRGB(DISPLAY_WHITE);
                else if ((ButtonTime + 2510 < millis()) && (ButtonTime + 2585 > millis())) SetDisplayRGB(DISPLAY_WHITE);
                else if (ButtonTime + 7010 < millis()) SetDisplayRGB(DISPLAY_WHITE);
                else SetDisplayRGB(DISPLAY_OFF);
            }
        }
        else {
            ButtonTime = millis();
            ButtonPressed = 1;
        }
    }
    else {
        DisplayBusy = 0;
        if ((ButtonPressed) && (ButtonDebounced)) {
            ButtonTime = millis() - ButtonTime;
            if ((ButtonTime > 2500) && (ButtonTime < 7000)) ButtonMode = 3;
            else if ((ButtonTime > 750) && (ButtonTime < 2000)) ButtonMode = 2;
            else if ((ButtonTime > 100) && (ButtonTime < 500)) ButtonMode = 1;
        }
        ButtonPressed = 0;
        ButtonDebounced = 0;
    }
    if (ButtonMode) {
        if (ButtonMode == 1) {
            ModeState++;
            if (ModeState > MODE_MANUAL) ModeState = MODE_LOW;
            ChangeModeFlag = 1;
            ChangeModeTime = millis();
        }
        else if (ButtonMode == 2) {
            LightState = !LightState;
            EEPROM.update(SAVED_LIGHT, LightState & 0x01);
        }
        else if (ButtonMode == 3) {
            PowerState = !PowerState;
            EEPROM.update(SAVED_POWER, PowerState & 0x01);
        }
        ButtonMode = 0;
    }
    if ((ChangeModeFlag) && ((ChangeModeTime + CHANGE_MODE_DELAY) < millis())) {
        ChangeModeFlag = 0;
        EEPROM.update(SAVED_MODE, ModeState);
    }
    if (PowerState) {
        switch (ModeState) {
          case MODE_LOW:
            SetFanSpeed(FAN_LOW_SPEED);
            if (!DisplayBusy) SetDisplayRGB(DISPLAY_BLUE);
            break;
          case MODE_NORMAL:
            SetFanSpeed(FAN_NORMAL_SPEED);
            if (!DisplayBusy) SetDisplayRGB(DISPLAY_GREEN);
            break;
          case MODE_HIGH:
            SetFanSpeed(FAN_HIGH_SPEED);
            if (!DisplayBusy) SetDisplayRGB(DISPLAY_YELLOW);
            break;
          case MODE_TURBO:
            SetFanSpeed(FAN_TURBO_SPEED);
            if (!DisplayBusy) SetDisplayRGB(DISPLAY_RED);
            break;
          case MODE_AUTO:
            if (SpeedTime + CHANGE_SPEED_DELAY < millis()) {
                SpeedTime = millis();
                ADCToTemp(analogRead(IO_TEMP));
                delay(ADC_GUARD_TIME);
                CurrentTemp = ADCToTemp(analogRead(IO_TEMP));
                if ((CurrentTemp > (TEMP_OFF + T_THRESHOLD)) && (CurrentSpeed == FAN_OFF)) CurrentSpeed = FAN_LOW_SPEED;
                else if ((CurrentTemp < (TEMP_OFF - T_THRESHOLD)) && (CurrentSpeed == FAN_LOW_SPEED)) CurrentSpeed = FAN_OFF;
                else if ((CurrentTemp > (TEMP_MIN + T_THRESHOLD)) && (CurrentSpeed == FAN_LOW_SPEED)) CurrentSpeed = FAN_NORMAL_SPEED;
                else if ((CurrentTemp < (TEMP_MIN - T_THRESHOLD)) && (CurrentSpeed == FAN_NORMAL_SPEED)) CurrentSpeed = FAN_LOW_SPEED;
                else if ((CurrentTemp > (TEMP_MEAN + T_THRESHOLD)) && (CurrentSpeed == FAN_NORMAL_SPEED)) CurrentSpeed = FAN_HIGH_SPEED;
                else if ((CurrentTemp < (TEMP_MEAN - T_THRESHOLD)) && (CurrentSpeed == FAN_HIGH_SPEED)) CurrentSpeed = FAN_NORMAL_SPEED;
                else if ((CurrentTemp > (TEMP_MAX + T_THRESHOLD)) && (CurrentSpeed == FAN_HIGH_SPEED)) CurrentSpeed = FAN_TURBO_SPEED;
                else if ((CurrentTemp < (TEMP_MAX - T_THRESHOLD)) && (CurrentSpeed == FAN_TURBO_SPEED)) CurrentSpeed = FAN_HIGH_SPEED;
                SetFanSpeed(CurrentSpeed);
            }
            if (!DisplayBusy) SetDisplayRGB(DISPLAY_AUTO_OFFSET + CurrentSpeed);
            break;
          case MODE_MANUAL:
            ManualSpeed = map(analogRead(IO_POT), 0, 1023, FAN_OFF, FAN_TURBO_SPEED + 1);
            if (ManualSpeed > FAN_TURBO_SPEED) ManualSpeed = FAN_TURBO_SPEED;
            SetFanSpeed(ManualSpeed);
            if (!DisplayBusy) SetDisplayRGB(DISPLAY_SINE_OFFSET + ManualSpeed);
            break;
        }
        if (LightState) {
            SetLightColor(LIGHT_RGB, EEPROM.read(SAVED_RED), EEPROM.read(SAVED_GREEN), EEPROM.read(SAVED_BLUE));
        }
        else SetLightColor(LIGHT_OFF);
    }
    else {
        SetFanSpeed(FAN_OFF);
        SetLightColor(LIGHT_OFF);
    }
#ifdef DEBUG
if ((DebugTime + DEBUG_SAMPLE) < millis()) {
    //Serial.println(CurrentTemp);
    DebugTime = millis();
}
#endif
}

unsigned int ADCToVolt(unsigned int Quantify) {
    return(Quantify * V_GAIN);
}

unsigned long ADCToTemp(unsigned int Quantify) {
    return(Quantify * T_GAIN);
}

void SetFanSpeed(unsigned char SpeedVal) {
    unsigned int CurrentVolt = 0;
    static unsigned long SampleTime = 0;
    static unsigned long ThresholdTime = 0;

    switch(SpeedVal) {
      case FAN_OFF:
        digitalWrite(IO_FAN_OFF, HIGH);
        digitalWrite(IO_FAN_LOW, LOW);
        digitalWrite(IO_FAN_NORMAL, LOW);
        digitalWrite(IO_FAN_HIGH, LOW);
        digitalWrite(IO_FAN_TURBO, LOW);
        break;
      case FAN_LOW_SPEED:
        digitalWrite(IO_FAN_LOW, HIGH);
        digitalWrite(IO_FAN_OFF, LOW);
        digitalWrite(IO_FAN_NORMAL, LOW);
        digitalWrite(IO_FAN_HIGH, LOW);
        digitalWrite(IO_FAN_TURBO, LOW);
        break;
      case FAN_NORMAL_SPEED:
        digitalWrite(IO_FAN_NORMAL, HIGH);
        digitalWrite(IO_FAN_OFF, LOW);
        digitalWrite(IO_FAN_LOW, LOW);
        digitalWrite(IO_FAN_HIGH, LOW);
        digitalWrite(IO_FAN_TURBO, LOW);
        break;
      case FAN_HIGH_SPEED:
        digitalWrite(IO_FAN_HIGH, HIGH);
        digitalWrite(IO_FAN_OFF, LOW);
        digitalWrite(IO_FAN_LOW, LOW);
        digitalWrite(IO_FAN_NORMAL, LOW);
        digitalWrite(IO_FAN_TURBO, LOW);
        break;
      case FAN_TURBO_SPEED:
        digitalWrite(IO_FAN_TURBO, HIGH);
        digitalWrite(IO_FAN_OFF, LOW);
        digitalWrite(IO_FAN_LOW, LOW);
        digitalWrite(IO_FAN_NORMAL, LOW);
        digitalWrite(IO_FAN_HIGH, LOW);
    }
    if (SampleTime + SAMPLE_DELAY < millis()) {
        SampleTime = millis();
        ADCToVolt(analogRead(IO_VOLT_FB));
        delay(ADC_GUARD_TIME);
        CurrentVolt = ADCToVolt(analogRead(IO_VOLT_FB));
        if ((CurrentVolt < (VOLT_OFF - V_THRESHOLD)) || (CurrentVolt > (VOLT_TURBO + V_THRESHOLD))) {
            if (ThresholdTime == 0) ThresholdTime = millis() + THRESHOLD_DELAY;
        }
        if (((CurrentVolt < (VOLT_OFF - V_THRESHOLD)) || (CurrentVolt > (VOLT_TURBO + V_THRESHOLD))) && ((millis() > ThresholdTime) && (ThresholdTime))) {
            digitalWrite(IO_FAN_OFF, HIGH);
            digitalWrite(IO_FAN_LOW, HIGH);
            digitalWrite(IO_FAN_NORMAL, HIGH);
            digitalWrite(IO_FAN_HIGH, HIGH);
            digitalWrite(IO_FAN_TURBO, HIGH);
            digitalWrite(IO_LIGHT_G, LOW);
            digitalWrite(IO_DISPLAY_G, LOW);
            digitalWrite(IO_LIGHT_B, LOW);
            digitalWrite(IO_DISPLAY_B, LOW);
            do {
                digitalWrite(IO_LIGHT_R, millis() & 0x40);
                digitalWrite(IO_DISPLAY_R, millis() & 0x40);
            } while(1);
        }
        else ThresholdTime = 0;
    }
}

unsigned char SetLightColor(unsigned char ModeVal, unsigned char StCh, unsigned char NdCh, unsigned char RdCh) {
    const unsigned char cie[256] = {
        0, 0, 0, 0, 0, 1, 1, 1,                     1, 1, 1, 1, 1, 1, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 3,                     3, 3, 3, 3, 3, 3, 3, 4,
        4, 4, 4, 4, 4, 5, 5, 5,                     5, 5, 6, 6, 6, 6, 6, 7,
        7, 7, 7, 8, 8, 8, 8, 9,                     9, 9, 10, 10, 10, 10, 11, 11,
        11, 12, 12, 12, 13, 13, 13, 14,             14, 15, 15, 15, 16, 16, 17, 17, 
        17, 18, 18, 19, 19, 20, 20, 21,             21, 22, 22, 23, 23, 24, 24, 25,
        25, 26, 26, 27, 28, 28, 29, 29,             30, 31, 31, 32, 32, 33, 34, 34,
        35, 36, 37, 37, 38, 39, 39, 40,             41, 42, 43, 43, 44, 45, 46, 47,
        47, 48, 49, 50, 51, 52, 53, 54,             54, 55, 56, 57, 58, 59, 60, 61,
        62, 63, 64, 65, 66, 67, 68, 70,             71, 72, 73, 74, 75, 76, 77, 79,
        80, 81, 82, 83, 85, 86, 87, 88,             90, 91, 92, 94, 95, 96, 98, 99,
        100, 102, 103, 105, 106, 108, 109, 110,     112, 113, 115, 116, 118, 120, 121, 123,
        124, 126, 128, 129, 131, 132, 134, 136,     138, 139, 141, 143, 145, 146, 148, 150,
        152, 154, 155, 157, 159, 161, 163, 165,     167, 169, 171, 173, 175, 177, 179, 181,
        183, 185, 187, 189, 191, 193, 196, 198,     200, 202, 204, 207, 209, 211, 214, 216,
        218, 220, 223, 225, 228, 230, 232, 235,     237, 240, 242, 245, 247, 250, 252, 255 };
    static unsigned char IndexRed = 0;
    static unsigned char IndexGreen = 0;
    static unsigned char IndexBlue = 0;
    unsigned char ValR = 0;
    unsigned char ValG = 0;
    unsigned char ValB = 0;
    unsigned char ret = 0;
    unsigned int i = 0;
    static unsigned long RefreshTime = 0;
    float ValH = 0.0;
    float ValS = 0.0;
    float ValV = 0.0;
    float ValP = 0.0;
    float ValQ = 0.0;
    float ValT = 0.0;

    switch(ModeVal) {
      case 0: //OFF
        ret = SetLightColor(LIGHT_RGB);
        break;
      case 1: //RGB
        if (RefreshTime + 5 < millis()) {
            RefreshTime = millis();
            StCh = cie[StCh];
            NdCh = cie[NdCh];
            RdCh = cie[RdCh];
            if (IndexRed < StCh) IndexRed++;
            else if (IndexRed > StCh) IndexRed--;
            if (IndexGreen < NdCh) IndexGreen++;
            else if (IndexGreen > NdCh) IndexGreen--;
            if (IndexBlue < RdCh) IndexBlue++;
            else if (IndexBlue > RdCh) IndexBlue--;
            analogWrite(IO_LIGHT_R, cie[IndexRed]);
            analogWrite(IO_LIGHT_G, cie[IndexGreen]);
            analogWrite(IO_LIGHT_B, cie[IndexBlue]);
        }
        ret = 1;
        break;
      case 2: //HSV
        //adapted from https://github.com/ratkins/RGBConverter
        ValH = StCh / 255.0;
        ValS = NdCh / 255.0;
        ValV = RdCh / 255.0;
        i = int(ValH * 6);
        ValP = ValV * (1 - ValS);
        ValQ = ValV * (1 - (ValH * 6 - i) * ValS);
        ValT = ValV * (1 - (1 - (ValH * 6 - i)) * ValS);

        switch(i % 6){
          case 0:
            ValR = int(ValV * 255);
            ValG = int(ValT * 255);
            ValB = int(ValP * 255);
            break;
          case 1:
            ValR = int(ValQ * 255);
            ValG = int(ValV * 255);
            ValB = int(ValP * 255);
            break;
          case 2:
            ValR = int(ValP * 255);
            ValG = int(ValV * 255);
            ValB = int(ValT * 255);
            break;
          case 3:
            ValR = int(ValP * 255);
            ValG = int(ValQ * 255);
            ValB = int(ValV * 255);
            break;
          case 4:
            ValR = int(ValT * 255);
            ValG = int(ValP * 255);
            ValB = int(ValV * 255);
            break;
          case 5:
            ValR = int(ValV * 255);
            ValG = int(ValP * 255);
            ValB = int(ValQ * 255);
        }
        SetLightColor(LIGHT_RGB, ValR, ValG, ValB);
        ret = 1;
        break;
      case LIGHT_GET_R:
        ValH = StCh / 255.0;
        ValS = NdCh / 255.0;
        ValV = RdCh / 255.0;
        i = int(ValH * 6);
        ValP = ValV * (1 - ValS);
        ValQ = ValV * (1 - (ValH * 6 - i) * ValS);
        ValT = ValV * (1 - (1 - (ValH * 6 - i)) * ValS);

        switch(i % 6){
          case 0:
            ret = int(ValV * 255);
            break;
          case 1:
            ret = int(ValQ * 255);
            break;
          case 2:
            ret = int(ValP * 255);
            break;
          case 3:
            ret = int(ValP * 255);
            break;
          case 4:
            ret = int(ValT * 255);
            break;
          case 5:
            ret = int(ValV * 255);
        }
        break;
      case LIGHT_GET_G:
        ValH = StCh / 255.0;
        ValS = NdCh / 255.0;
        ValV = RdCh / 255.0;
        i = int(ValH * 6);
        ValP = ValV * (1 - ValS);
        ValQ = ValV * (1 - (ValH * 6 - i) * ValS);
        ValT = ValV * (1 - (1 - (ValH * 6 - i)) * ValS);

        switch(i % 6){
          case 0:
            ret = int(ValT * 255);
            break;
          case 1:
            ret = int(ValV * 255);
            break;
          case 2:
            ret = int(ValV * 255);
            break;
          case 3:
            ret = int(ValQ * 255);
            break;
          case 4:
            ret = int(ValP * 255);
            break;
          case 5:
            ret = int(ValP * 255);
        }
        break;
      case LIGHT_GET_B:
        ValH = StCh / 255.0;
        ValS = NdCh / 255.0;
        ValV = RdCh / 255.0;
        i = int(ValH * 6);
        ValP = ValV * (1 - ValS);
        ValQ = ValV * (1 - (ValH * 6 - i) * ValS);
        ValT = ValV * (1 - (1 - (ValH * 6 - i)) * ValS);

        switch(i % 6){
          case 0:
            ret = int(ValP * 255);
            break;
          case 1:
            ret = int(ValP * 255);
            break;
          case 2:
            ret = int(ValT * 255);
            break;
          case 3:
            ret = int(ValV * 255);
            break;
          case 4:
            ret = int(ValV * 255);
            break;
          case 5:
            ret = int(ValQ * 255);
        }
    }
    return(ret);
}

void SetDisplayRGB(unsigned char DisplayState) {
    const unsigned char FadeIn[36] = {
        0, 2, 3, 6, 10, 15, 22, 31,                 41, 54, 68, 86, 106, 129, 155, 185,
        218, 255, 255, 218, 185, 155, 129, 106,     86, 68, 54, 41, 31, 22, 15, 10,
        6, 3, 2, 0 };
    const unsigned char FadeOut[36] = {
        255, 218, 185, 155, 129, 106, 86, 68,       54, 41, 31, 22, 15, 10, 6, 3,
        2, 0, 0, 2, 3, 6, 10, 15,                   22, 31, 41, 54, 68, 86, 106, 129,
        155, 185, 218, 255 };
    const unsigned char HueShift[60] = {
        255, 255, 255, 255, 255, 255, 172, 109,     64, 33, 14, 5, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,                     5, 14, 33, 64, 109, 172, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255,     255, 255, 172, 109, 64, 33, 14, 5,
        0, 0, 0, 0, 0, 0, 0, 0,                     0, 0, 0, 0 };
    const char Sine[40] = {
        47, 68, 95, 124, 157, 187, 216, 237,        250, 252, 245, 228, 202, 173, 141, 109,
        81, 57, 38, 24, 14, 8, 4, 2,                1, 0, 0, 0, 1, 2, 3, 6,
        11, 18, 30, 47, 68, 95, 124, 157 };
    static unsigned char LastState = 0;
    static unsigned char Index = 0;
    unsigned char RefreshDelay = 0;
    static unsigned long RefreshTime = 0;

    if (DisplayState != LastState) {
        Index = 0;
        LastState = DisplayState;
    }
    else {
        if (DisplayState > DISPLAY_SINE_OFFSET -1) {
            RefreshDelay = 50 / (DisplayState - DISPLAY_SINE_OFFSET + 1);
        }
        else RefreshDelay = 25;
        if (RefreshTime + RefreshDelay < millis()) {
            RefreshTime = millis();
            Index++;
            if (Index > 35) Index = 0;
        }
    }
    switch (DisplayState) {
      case DISPLAY_OFF:
        digitalWrite(IO_DISPLAY_R, LOW);
        digitalWrite(IO_DISPLAY_G, LOW);
        digitalWrite(IO_DISPLAY_B, LOW);
        break;
      case DISPLAY_WHITE:
        digitalWrite(IO_DISPLAY_R, HIGH);
        digitalWrite(IO_DISPLAY_G, HIGH);
        digitalWrite(IO_DISPLAY_B, HIGH);
        break;
     case DISPLAY_RED:
        digitalWrite(IO_DISPLAY_R, HIGH);
        digitalWrite(IO_DISPLAY_G, LOW);
        digitalWrite(IO_DISPLAY_B, LOW);
        break;
      case DISPLAY_YELLOW:
        digitalWrite(IO_DISPLAY_R, HIGH);
        digitalWrite(IO_DISPLAY_G, HIGH);
        digitalWrite(IO_DISPLAY_B, LOW);
        break;
      case DISPLAY_GREEN:
        digitalWrite(IO_DISPLAY_R, LOW);
        digitalWrite(IO_DISPLAY_G, HIGH);
        digitalWrite(IO_DISPLAY_B, LOW);
        break;
      case DISPLAY_BLUE:
        digitalWrite(IO_DISPLAY_R, LOW);
        digitalWrite(IO_DISPLAY_G, LOW);
        digitalWrite(IO_DISPLAY_B, HIGH);
        break;
      case DISPLAY_FAST_BLINK_W:
        digitalWrite(IO_DISPLAY_R, millis() & 0x80);
        digitalWrite(IO_DISPLAY_G, millis() & 0x80);
        digitalWrite(IO_DISPLAY_B, millis() & 0x80);
        break;
      case DISPLAY_FAST_BLINK_R:
        digitalWrite(IO_DISPLAY_R, millis() & 0x80);
        digitalWrite(IO_DISPLAY_G, LOW);
        digitalWrite(IO_DISPLAY_B, LOW);
        break;
      case DISPLAY_HUE_LOOP:
        analogWrite(IO_DISPLAY_R, HueShift[Index]);
        analogWrite(IO_DISPLAY_G, HueShift[Index + 24]);
        analogWrite(IO_DISPLAY_B, HueShift[Index + 12]);
        break;
      case DISPLAY_SAT_LOOP:
        analogWrite(IO_DISPLAY_R, FadeOut[Index]);
        analogWrite(IO_DISPLAY_G, FadeOut[Index]);
        analogWrite(IO_DISPLAY_B, 255);
        break;
      case DISPLAY_BRI_LOOP:
        analogWrite(IO_DISPLAY_R, 0);
        analogWrite(IO_DISPLAY_G, 0);
        analogWrite(IO_DISPLAY_B, FadeIn[Index]);
        break;
      case DISPLAY_GREY_LOOP:
        analogWrite(IO_DISPLAY_R, (FadeIn[Index] >> 4));
        analogWrite(IO_DISPLAY_G, (FadeIn[Index] >> 4));
        analogWrite(IO_DISPLAY_B, (FadeIn[Index] >> 4));
        break;
      case DISPLAY_BLUE_LOOP:
        analogWrite(IO_DISPLAY_R, 0);
        analogWrite(IO_DISPLAY_G, 0);
        analogWrite(IO_DISPLAY_B, FadeIn[Index]);
        break;
      case DISPLAY_GREEN_LOOP:
        analogWrite(IO_DISPLAY_R, 0);
        analogWrite(IO_DISPLAY_G, FadeIn[Index]);
        analogWrite(IO_DISPLAY_B, 0);
        break;
      case DISPLAY_YELLOW_LOOP:
        analogWrite(IO_DISPLAY_R, FadeIn[Index]);
        analogWrite(IO_DISPLAY_G, FadeIn[Index]);
        analogWrite(IO_DISPLAY_B, 0);
        break;
      case DISPLAY_RED_LOOP:
        analogWrite(IO_DISPLAY_R, FadeIn[Index]);
        analogWrite(IO_DISPLAY_G, 0);
        analogWrite(IO_DISPLAY_B, 0);
        break;
      case DISPLAY_SINE_QUIET:
      case DISPLAY_SINE_FAST:
      case DISPLAY_SINE_VFAST:
      case DISPLAY_SINE_UFAST:
      case DISPLAY_SINE_SFAST:
        analogWrite(IO_DISPLAY_R, Sine[(Index + 3)]);
        analogWrite(IO_DISPLAY_G, 0);
        analogWrite(IO_DISPLAY_B, Sine[Index]);
    }
}

