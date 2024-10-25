#include <IRremote.h>
//#include <IRsend.h>
//#include <codes.h>
//#include <hvac.h>
  
#define SEND_RATE_KHZ 38 // Assume 38 KHz
#define CHIGO_HEADER              "FF00FF00"
#define CHIGO_FOOTER              "54AB"
#define CHIGO_CMD_TEMP_UP         "BF40"
#define CHIGO_CMD_TEMP_DOWN       "3FC0"
#define CHIGO_CMD_MODE            "7F80"
#define CHIGO_CMD_SLEEP           "6F90"
#define CHIGO_CMD_POWER           "FF00"
#define CHIGO_PARAM_POWEROFF_SWING_0 "E010"
#define CHIGO_PARAM_POWEROFF_SWING_1 "F000"
#define CHIGO_PARAM_POWEROFF_SWING_2 "D020"
#define CHIGO_TIMER_NEW_1h        "7A85"
#define CHIGO_TIMER_OLD_1h        "7E81"
#define CHIGO_TIMER_NEW_2h        "BA45"
#define CHIGO_TIMER_OLD_2h        "BE41"
#define CHIGO_TIMER_NEW_3h        "3AC5"
#define CHIGO_TIMER_OLD_3h        "3EC1"
#define CHIGO_TIMER_NEW_4h        "DA25"
#define CHIGO_TIMER_OLD_4h        "DE21"
#define CHIGO_PARAM_MODE_AUTO        "0F00"
#define CHIGO_PARAM_MODE_COOL        "0B04"
#define CHIGO_PARAM_MODE_COOL_ALT    "030C"
#define CHIGO_PARAM_MODE_HEAT        "0E01"
#define CHIGO_PARAM_MODE_HEAT_ALT    "0609"
#define CHIGO_PARAM_MODE_DRY         "0D02"
#define CHIGO_PARAM_MODE_FAN         "0906"
#define CHIGO_PARAM_MODE_FAN_ALT     "010E"
#define CHIGO_PARAM_SPEED_SLOW       "0906" // AirFlow off
#define CHIGO_PARAM_SPEED_MEDIUM     "0D02" // AirFlow off
#define CHIGO_PARAM_SPEED_FAST       "0B04" // AirFlow off
#define CHIGO_PARAM_SPEED_SMART      "0F00" // AirFlow off
#define CHIGO_PARAM_SPEED_AF_SLOW    "010E" // AirFlow on
#define CHIGO_PARAM_SPEED_AF_MEDIUM  "050A" // AirFlow on
#define CHIGO_PARAM_SPEED_AF_FAST    "030C" // AirFlow on
#define CHIGO_PARAM_SPEED_AF_SMART   "0708" // AirFlow on
#define CHIGO_PARAM_SWING_0          "A050" // Sleep Mode off
#define CHIGO_PARAM_SWING_1          "B040" // Sleep Mode off
#define CHIGO_PARAM_SWING_2          "9060" // Sleep Mode off
#define CHIGO_PARAM_SWING_SLEEP_0    "20D0" // Sleep Mode on
#define CHIGO_PARAM_SWING_SLEEP_1    "30C0" // Sleep Mode on
#define CHIGO_PARAM_SWING_SLEEP_2    "10E0" // Sleep Mode on
#define CHIGO_PARAM_TEMP_16          "F000"
#define CHIGO_PARAM_TEMP_17          "7080"
#define CHIGO_PARAM_TEMP_18          "B040"
#define CHIGO_PARAM_TEMP_19          "30C0"
#define CHIGO_PARAM_TEMP_20          "D020"
#define CHIGO_PARAM_TEMP_21          "50A0"
#define CHIGO_PARAM_TEMP_22          "9060"
#define CHIGO_PARAM_TEMP_23          "10E0"
#define CHIGO_PARAM_TEMP_24          "E010"
#define CHIGO_PARAM_TEMP_25          "6090"
#define CHIGO_PARAM_TEMP_26          "A050"
#define CHIGO_PARAM_TEMP_27          "20D0"
#define CHIGO_PARAM_TEMP_28          "C030"
#define CHIGO_PARAM_TEMP_29          "40B0"
#define CHIGO_PARAM_TEMP_30          "8070"
#define CHIGO_PARAM_TEMP_31          "00F0"
#define CHIGO_PARAM_TEMP_32          "F000"

#define IR_SEND_PIN 11

struct List {
  uint16_t data[197];
  uint16_t counter = 0;
};

char temperatures[17][5] = {
  CHIGO_PARAM_TEMP_16,
  CHIGO_PARAM_TEMP_17,
  CHIGO_PARAM_TEMP_18,
  CHIGO_PARAM_TEMP_19,
  CHIGO_PARAM_TEMP_20,
  CHIGO_PARAM_TEMP_21,
  CHIGO_PARAM_TEMP_22,
  CHIGO_PARAM_TEMP_23,
  CHIGO_PARAM_TEMP_24,
  CHIGO_PARAM_TEMP_25,
  CHIGO_PARAM_TEMP_26,
  CHIGO_PARAM_TEMP_27,
  CHIGO_PARAM_TEMP_28,
  CHIGO_PARAM_TEMP_29,
  CHIGO_PARAM_TEMP_30,
  CHIGO_PARAM_TEMP_31,
  CHIGO_PARAM_TEMP_32
};

enum Mode {
  Auto = 0, Cool, Dry, Heat, Fan
};

enum Speed {
  Slow = 0, Medium, Fast, Smart
};

int RECV_PIN = 10;
int BUILTIN_LED = 13;

float armazenavalor;
bool state = false;
int stateSwing = 0; //  {"horizontal", "fixed", "natural"}
int airSpeed = 1; //[4] = {"slow", "medium", "fast", "smart"}
bool airFlow = false;
bool sleepMode = false;
bool turbo = false;
int stateTemperature = 23;
Mode stateMode = Auto;

IRrecv irrecv(RECV_PIN);  
decode_results results;

//IRsend irsend(IR_SEND_PIN);


void setup()  
{    
  pinMode(BUILTIN_LED, OUTPUT); 
  digitalWrite(BUILTIN_LED, LOW);  
  Serial.begin(9600);  
  
  //irrecv.enableIRIn(); // Inicializa o receptor IR 
  //irrecv.blink13(true); 
  
  //irsend.begin();
  //Serial.println(F("Send IR signals at pin " STR(IR_SEND_PIN)));

  IrSender.begin(IR_SEND_PIN);
  
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("Pronto!"); 
}


void turnOn() {
  state = true;
  sendCode(CHIGO_CMD_POWER, getPowerAsParameter(state));
  //Serial.println("turned ON!");
}

void turnOff() {
  state = false;
  sendCode(CHIGO_CMD_POWER, getPowerAsParameter(state));
  Serial.println("turned OFF!");
}

void dump(decode_results *results) {
  int count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  }
  else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  }
  else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  }
  else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  }
  else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  }
  else if (results->decode_type == LG) {
    Serial.print("Decoded LG: ");
  }
  else if (results->decode_type == JVC) {
    Serial.print("Decoded JVC: ");
  }
  else if (results->decode_type == WHYNTER) {
    Serial.print("Decoded Whynter: ");
  }
  Serial.print(results->value, HEX);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");
 
  for (int i = 1; i < count; i++) {
    if (i & 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    }
    else {
      Serial.write('-');
      Serial.print((unsigned long) results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println();
}


void printData(List data) {
  Serial.print("data[");
  const size_t size = sizeof(data.data) / sizeof(data.data[0]);
  for (int i = 0; i < size; i++) {
        Serial.print(data.data[i]);
        Serial.print(", ");
  }
  Serial.print("]: ");
  Serial.println("sizeOfData: ");
  Serial.print(size);
}

void sendCode(String cmd, char* param) {
  List data;

  addHeaderToData(data);

  addBytesToData("FF00", 4, data);
  addBytesToData("FF00", 4, data);

  addCommandToData(cmd, data);
  addParameterToData(param, data);

  addTemperatureAndModeToData(stateTemperature, getModeAsParameter(stateMode), data);

  addFooterToData(data);
  
  IrSender.sendRaw(data.data, data.counter, SEND_RATE_KHZ);
  //Serial.print(data.data);
  printData(data);
  //Serial.println("Sent raw!");
}

void loop()  
{  
  delay(4000); 

  //Serial.println("Loop init");
  if (!state) {
    digitalWrite(BUILTIN_LED, HIGH);
    turnOn();
    delay(1000);
    digitalWrite(BUILTIN_LED, LOW);
  }

  /*if (irrecv.decode(&results) )  //&& results.value != 0
  { 
    Serial.println("");
    Serial.print("Valor lido : ");  
    Serial.println(results.value, HEX);  
    //armazenavalor = (results.value);
    //Serial.println("");
    //serialPrintUint64(results.value, HEX);
    dump(&results);


    //digitalWrite(BUILTIN_LED, HIGH);
    //delay(500);
    //digitalWrite(BUILTIN_LED, LOW);  
    //if (armazenavalor == 0xFF52AD) //Verifica se a tecla 9 foi acionada  
    //{  
    //  digitalWrite(pinoledvermelho, LOW); //Apaga todos os leds  
    //  digitalWrite(BUILTIN_LED, LOW);  
    //}  
    irrecv.resume(); //Le o próximo valor  
  }*/

}

void addTemperatureAndModeToData(int temp, String mode, List& data) {
  unsigned int realTempIndex = temp - 16;
  char tempAndMode[5] = {0};
  strcpy(tempAndMode, temperatures[realTempIndex]);

  tempAndMode[1] = mode.charAt(1);
  tempAndMode[3] = mode.charAt(3);
  addBytesToData(tempAndMode, 4, data);
}

char* getPowerAsParameter(bool power) {
  char *param = getCompositeSpeedAsParameter();
  if (!power) {
    char *swingMode;
    switch(stateSwing) {
      case 0:
        swingMode = (char*) CHIGO_PARAM_POWEROFF_SWING_0;
      case 1:
        swingMode = (char*) CHIGO_PARAM_POWEROFF_SWING_1;
      case 2:
        swingMode = (char*) CHIGO_PARAM_POWEROFF_SWING_2;
      default:
        swingMode = (char*) CHIGO_PARAM_POWEROFF_SWING_0;
    }
    
    param[0] = swingMode[0];
    param[2] = swingMode[2];
  }
  return param;
}

char* getCompositeSpeedAsParameter() {
  String airSpeedComponent = getSpeedAsParameter(airSpeed, airFlow);
  String swingComponent = getSwingAsParameter(stateSwing, sleepMode);
  char compositeSpeed[5];
  compositeSpeed[0] = swingComponent.charAt(0);
  compositeSpeed[1] = airSpeedComponent.charAt(1);
  compositeSpeed[2] = swingComponent.charAt(2);
  compositeSpeed[3] = airSpeedComponent.charAt(3);
  return compositeSpeed;
}

char* getSpeedAsParameter(Speed airSpeed, bool airFlow) {
  if (airFlow) {
    switch (airSpeed) {
      case Slow:
        return CHIGO_PARAM_SPEED_AF_SLOW;
      case Medium:
        return CHIGO_PARAM_SPEED_AF_MEDIUM;
      case Fast:
        return CHIGO_PARAM_SPEED_AF_FAST;
      case Smart:  
        return CHIGO_PARAM_SPEED_AF_SMART;
    }
  }
  else {
    switch (airSpeed) {
      case Slow:
        return CHIGO_PARAM_SPEED_SLOW;
      case Medium:
        return CHIGO_PARAM_SPEED_MEDIUM;
      case Fast:
        return CHIGO_PARAM_SPEED_FAST;
      case Smart:  
        return CHIGO_PARAM_SPEED_SMART;
    }
  }
}

char* getSwingAsParameter(unsigned swing, bool sleepMode) {
  if (sleepMode) {
    switch (swing) {
      case 0:
        return CHIGO_PARAM_SWING_SLEEP_0;
      case 1:
        return CHIGO_PARAM_SWING_SLEEP_1;
      case 2:
        return CHIGO_PARAM_SWING_SLEEP_2;
    }
  }
  else {
    switch (swing) {
      case 0:
        return CHIGO_PARAM_SWING_0;
      case 1:
        return CHIGO_PARAM_SWING_1;
      case 2:
        return CHIGO_PARAM_SWING_2;
    }
  }
}

char* getModeAsParameter(Mode mode) {
  switch (mode) {
    case Auto:
      return CHIGO_PARAM_MODE_AUTO;
    case Cool:
      if (stateTemperature == 32)
        return CHIGO_PARAM_MODE_COOL_ALT;
      else
        return CHIGO_PARAM_MODE_COOL;
    case Dry:
      return CHIGO_PARAM_MODE_DRY;
    case Heat:
      if (stateTemperature == 32)
        return CHIGO_PARAM_MODE_HEAT_ALT;
      else
        return CHIGO_PARAM_MODE_HEAT;
    case Fan:
      if (stateTemperature == 32)
        return CHIGO_PARAM_MODE_FAN_ALT;
      else
        return CHIGO_PARAM_MODE_FAN;
  }
}

void addToList(List& data, unsigned int value) {
  data.data[data.counter++] = value;
}

void addBytesToData(String bytes, size_t count, List& data) {
  for (int i = 0; i < count; ++i) {
    byteToRawData(hexToByte(bytes[i]), data);
  }
}

void addHeaderToData(List& data) {
  addToList(data, 6234);
  addToList(data, 7392); //7302
}

void addFooterToData(List& data) {
  addBytesToData(CHIGO_FOOTER, 4, data);

  addToList(data, 608);
  addToList(data, 7372);
  addToList(data, 616);
}

void addCommandToData(String command, List& data) {
  addBytesToData(command, 4, data);
}

void addParameterToData(String parameter, List& data) {
  addBytesToData(parameter, 4, data);
}

byte hexToByte(char hex) {
  if (hex >= '0' && hex <= '9') {
    return hex - '0';
  }
  return hex - 'A' + 10;
}

unsigned int highEndRawData[2] = {500, 1570};

void byteToRawData(byte bytee, List& data) {
  for (int i = 3; i >= 0; --i) {
    addToList(data, highEndRawData[0]);
    addToList(data, highEndRawData[bitRead(bytee, i)]);
  }
}

Speed getSpeedFromParameter(String param) {
    // Remove 'swing' component from param
  param[0] = '0';
  param[2] = '0';

  // Interpret speed
  if (param.equalsIgnoreCase(CHIGO_PARAM_SPEED_SLOW) || param.equalsIgnoreCase(CHIGO_PARAM_SPEED_AF_SLOW)) {
    return Slow;
  }
    
  if (param.equalsIgnoreCase(CHIGO_PARAM_SPEED_MEDIUM) || param.equalsIgnoreCase(CHIGO_PARAM_SPEED_AF_MEDIUM)) {
    return Medium;
  }
    
  if (param.equalsIgnoreCase(CHIGO_PARAM_SPEED_FAST) || param.equalsIgnoreCase(CHIGO_PARAM_SPEED_AF_FAST)) {
    return Fast;
  }
    
  if (param.equalsIgnoreCase(CHIGO_PARAM_SPEED_SMART) || param.equalsIgnoreCase(CHIGO_PARAM_SPEED_AF_SMART)) {
    return Smart;
  }
}
