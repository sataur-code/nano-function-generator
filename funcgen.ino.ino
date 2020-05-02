#include <VT100.h>
#include <SimpleCLI.h>

// Simple function generator
// Supports
// 1. square wave (through PWM)
// 2. sin wave
// 3. triangle wave
// 4. saw
// 5. User-defined sequence

// Available pins
// PINs: 3,5,6,9,10,11

// MENU
// C - select pin
// F - select function
// R - execute sequence

#define FALSE         0
#define TRUE          1

#define FNC_NONE      0
#define FNC_SIN       1
#define FNC_SQUARE    2
#define FNC_TRIANGLE  3
#define FNC_SAWTOOTH  4
#define FNC_USERDEF   5
#define PINS_NUM      6
#define USERDEF_STEPS_MAX 10
#define FUNCTIONS_MAX     PINS_NUM


typedef struct 
{
    int level;
    int duration;
} USERDEF_STEP;


typedef struct
{
    int fnc;
    int freq;
    int mod;
    int numSteps;
    USERDEF_STEP steps[USERDEF_STEPS_MAX];
} FUNCTION;

typedef struct
{
    long usStep;
    long usPeriod;
} FUNCTION_STATUS;

const int SIN_TABLE[] = {
  0x80,0x86,0x8c,0x92,0x98,0x9e,0xa5,0xaa,
  0xb0,0xb6,0xbc,0xc1,0xc6,0xcb,0xd0,0xd5,
  0xda,0xde,0xe2,0xe6,0xea,0xed,0xf0,0xf3,
  0xf5,0xf8,0xfa,0xfb,0xfd,0xfe,0xfe,0xff,
  0xff,0xff,0xfe,0xfe,0xfd,0xfb,0xfa,0xf8,
  0xf5,0xf3,0xf0,0xed,0xea,0xe6,0xe2,0xde,
  0xda,0xd5,0xd0,0xcb,0xc6,0xc1,0xbc,0xb6,
  0xb0,0xaa,0xa5,0x9e,0x98,0x92,0x8c,0x86,
  0x80,0x79,0x73,0x6d,0x67,0x61,0x5a,0x55,
  0x4f,0x49,0x43,0x3e,0x39,0x34,0x2f,0x2a,
  0x25,0x21,0x1d,0x19,0x15,0x12,0xf,0xc,
  0xa,0x7,0x5,0x4,0x2,0x1,0x1,0x0,
  0x0,0x0,0x1,0x1,0x2,0x4,0x5,0x7,
  0xa,0xc,0xf,0x12,0x15,0x19,0x1d,0x21,
  0x25,0x2a,0x2f,0x34,0x39,0x3e,0x43,0x49,
  0x4f,0x55,0x5a,0x61,0x67,0x6d,0x73,0x79
};

const int TRIANGLE_TABLE[] = {
  0x4,0x8,0xc,0x10,0x14,0x18,0x1c,0x20,
  0x24,0x28,0x2c,0x30,0x34,0x38,0x3c,0x40,
  0x44,0x48,0x4c,0x50,0x54,0x58,0x5c,0x60,
  0x64,0x68,0x6c,0x70,0x74,0x78,0x7c,0x80,
  0x83,0x87,0x8b,0x8f,0x93,0x97,0x9b,0x9f,
  0xa3,0xa7,0xab,0xaf,0xb3,0xb7,0xbb,0xbf,
  0xc3,0xc7,0xcb,0xcf,0xd3,0xd7,0xdb,0xdf,
  0xe3,0xe7,0xeb,0xef,0xf3,0xf7,0xfb,0xff,
  0xfb,0xf7,0xf3,0xef,0xeb,0xe7,0xe3,0xdf,
  0xdb,0xd7,0xd3,0xcf,0xcb,0xc7,0xc3,0xbf,
  0xbb,0xb7,0xb3,0xaf,0xab,0xa7,0xa3,0x9f,
  0x9b,0x97,0x93,0x8f,0x8b,0x87,0x83,0x80,
  0x7c,0x78,0x74,0x70,0x6c,0x68,0x64,0x60,
  0x5c,0x58,0x54,0x50,0x4c,0x48,0x44,0x40,
  0x3c,0x38,0x34,0x30,0x2c,0x28,0x24,0x20,
  0x1c,0x18,0x14,0x10,0xc,0x8,0x4,0x0,
};

const int SAWTOOTH_TABLE[] = {
  0x2,0x4,0x6,0x8,0xa,0xc,0xe,0x10,
  0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,0x20,
  0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,
  0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,0x40,
  0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,
  0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,0x60,
  0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,
  0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,0x80,
  0x81,0x83,0x85,0x87,0x89,0x8b,0x8d,0x8f,
  0x91,0x93,0x95,0x97,0x99,0x9b,0x9d,0x9f,
  0xa1,0xa3,0xa5,0xa7,0xa9,0xab,0xad,0xaf,
  0xb1,0xb3,0xb5,0xb7,0xb9,0xbb,0xbd,0xbf,
  0xc1,0xc3,0xc5,0xc7,0xc9,0xcb,0xcd,0xcf,
  0xd1,0xd3,0xd5,0xd7,0xd9,0xdb,0xdd,0xdf,
  0xe1,0xe3,0xe5,0xe7,0xe9,0xeb,0xed,0xef,
  0xf1,0xf3,0xf5,0xf7,0xf9,0xfb,0xfd,0xff,
};

int INDEX_TO_PIN[PINS_NUM] = { 3, 5, 6, 9, 10, 11 };

FUNCTION functions[PINS_NUM];
FUNCTION_STATUS status[PINS_NUM];

// Create CLI Object
SimpleCLI cli;

// Commands
Command cmdChannel; 
Command cmdFunction;
Command cmdRun;

int running = FALSE;
long runStart;
int currChannel = 0;

int pinToIndex(int pin)
{
  for (int i=0; i<PINS_NUM; i++)
  {
    if (INDEX_TO_PIN[i] == pin)
      return i;
  }

  return -1;
}

void initFunctionStatus()
{
  for (int i=0; i<PINS_NUM; i++)
  {
    FUNCTION* pf = &functions[i];
    FUNCTION_STATUS* ps = &status[i];
    if ((pf->fnc == FNC_SIN) || (pf->fnc == FNC_TRIANGLE) || (pf->fnc == FNC_SAWTOOTH))
    {
      ps->usPeriod = 1000000 / pf->freq;
      ps->usStep = ps->usPeriod >> 7;  
    }    
  }
}

// configured functions
void printConfiguration()
{
  int row = 10;
  
  for (int i=0; i<PINS_NUM; i++, row++)
  {
    FUNCTION* pf = &functions[i];
    if (pf->fnc == FNC_NONE)
      continue;

    // print pin number
    int pin = INDEX_TO_PIN[i];

    VT100.setCursor(row, 3);
    VT100.formatText(VT_BRIGHT);
    Serial.print(pin, DEC);
    VT100.formatText(VT_RESET);

    if (pf->fnc == FNC_SIN)
    {
      // sin wave
      Serial.print(" SIN freq: ");
      Serial.print(pf->freq);
      continue;
    }
            
    if (pf->fnc == FNC_SQUARE)
    {
      // square wave
      Serial.print(" SQUARE freq: ");
      Serial.print(pf->freq);
      Serial.print(", Mod. ");
      Serial.print(pf->mod);
      continue;
    }

    if (pf->fnc == FNC_TRIANGLE)
    {
      // triangle waveform
      Serial.print(" TRIANGLE freq: ");
      Serial.print(pf->freq);
      continue;
    }

    if (pf->fnc == FNC_USERDEF)
    {
      // user defined waveform
      Serial.print(" USERDEF ");
      for (int s=0; s<pf->numSteps; s++)
      {
        USERDEF_STEP* step = &pf->steps[s];
        Serial.print(" Lev. ");
        Serial.print((step->level == 1)? "1" : "0");
        Serial.print(" ");
        Serial.print(step->duration, DEC);
        Serial.print(" ms; ");
      }
      
      continue;
    }
  }
}

void printConfigMenu()
{
  VT100.clearScreen();

  // header
  VT100.setCursor(10, 1);
  VT100.formatText(VT_BRIGHT);
  Serial.println("---< FUNCTION GENERATOR >---");

  // channel selection
  VT100.setCursor(1, 3);
  VT100.formatText(VT_BRIGHT);
  Serial.print(" -C- ");
  VT100.formatText(VT_RESET);
  Serial.print(" Select channel ");

  if (currChannel != 0)
  {
    Serial.print(" [ ");
    Serial.print(currChannel, DEC);
    Serial.print(" ]");
  }

  // function selection
  VT100.setCursor(2, 3);
  VT100.formatText(VT_BRIGHT);
  Serial.print(" -F- ");
  VT100.formatText(VT_RESET);
  Serial.print(" Select function ");

  // execute
  VT100.setCursor(3, 3);
  VT100.formatText(VT_BRIGHT);
  Serial.print(" -R- ");
  VT100.formatText(VT_RESET);
  Serial.print(" Run! ");
}

void printRunMenu()
{
  VT100.clearScreen();

  // header
  VT100.setCursor(10, 1);
  VT100.formatText(VT_BRIGHT);
  Serial.println("---< FUNCTION GENERATOR >---");
}

void channelCallback(cmd* c) 
{
  Command cmd(c); // Create wrapper object

  // Get first (and only) Argument
  Argument arg = cmd.getArgument(0);

  // Get value of argument
  String argVal = arg.getValue(); 

  currChannel = argVal.toInt();

  printConfigMenu();
  printConfiguration();
}

void functionCallback(cmd* c) 
{
  Command cmd(c); // Create wrapper object

  int argNum = cmd.countArgs(); // Get number of arguments
  
  int index = pinToIndex(currChannel);
  if (index == -1)
    return; 

  FUNCTION* fp = &functions[index];

  // first argument is function type
  Argument arg0 = cmd.getArgument(0);
  String fncName = arg0.getValue(); 

  // second parameter is the frequency
  Argument arg1 = cmd.getArgument(1);
  String freq = arg1.getValue(); 

  if (fncName == "sin")
  {
    fp->fnc = FNC_SIN;
    fp->freq = freq.toInt();
  }
  else if (fncName == "square")
  {
    // third parameter is the modulation
    Argument arg2 = cmd.getArgument(2);
    String mod = arg2.getValue(); 

    fp->fnc = FNC_SQUARE;
    fp->freq = freq.toInt();
    fp->mod = mod.toInt();
  }
  else if (fncName == "triangle")
  {
    fp->fnc = FNC_TRIANGLE;
    fp->freq = freq.toInt();
  }
  else if (fncName == "saw")
  {
    fp->fnc = FNC_SAWTOOTH;
    fp->freq = freq.toInt();
  }
}

void runCallback(cmd* c) 
{
  Command cmd(c); // Create wrapper object

  running = TRUE;
  runStart = micros();

  initFunctionStatus();
  printRunMenu();
}

// Callback in case of an error
void errorCallback(cmd_error* e) {
  CommandError cmdError(e); // Create wrapper object

  Serial.print("ERROR: ");
  Serial.println(cmdError.toString());

  if (cmdError.hasCommand()) {
    Serial.print("Did you mean \"");
    Serial.print(cmdError.getCommand().toString());
    Serial.println("\"?");
  }
} 

void runTableFunction(int pin, FUNCTION* pf, FUNCTION_STATUS* ps, const int* table)
{
  long us = micros();  
  long usDelta = us - runStart;

  long lIndex = usDelta / ps->usStep;
  int index = (int)(lIndex & 0x7F);
  int next = (index + 1) % 128;
  int value = (table[index] + table[next]) / 2;

  analogWrite(pin, value);
}

void runSquareFunction(int pin, FUNCTION* pf, FUNCTION_STATUS* ps)
{
  analogWrite(pin, pf->mod);
}

void runUserDefFunction(int pin, FUNCTION* pf, FUNCTION_STATUS* ps)
{
  long us = micros();  
  long usDelta = us - runStart;

  long usFromStart = usDelta / ps->usPeriod;
  long usTotal = 0;
  us = 0;
  for (int i=0; i<pf->numSteps; i++)
  {
    USERDEF_STEP* step = &pf->steps[i];
    
    us = step->duration;
    us = us * 1000;
    usTotal += us;

    if (usFromStart < usTotal)
    {
      digitalWrite(pin, step->level);
      break;
    }
  }
}

void runFunctions()
{
  for (int i=0; i<PINS_NUM; i++)
  {
    FUNCTION* pf = &functions[i];  
    FUNCTION_STATUS* ps = &status[i];
      
    if (pf->fnc == FNC_NONE)
      continue;

    int pin = INDEX_TO_PIN[i];
    
    if (pf->fnc == FNC_SIN)
      runTableFunction(pin, pf, ps, SIN_TABLE);
    else if (pf->fnc == FNC_TRIANGLE)
      runTableFunction(pin, pf, ps, TRIANGLE_TABLE);
    else if (pf->fnc == FNC_SAWTOOTH)
      runTableFunction(pin, pf, ps, SAWTOOTH_TABLE);
    else if (pf->fnc == FNC_SQUARE)
      runSquareFunction(pin, pf, ps);
    else if (pf->fnc == FNC_USERDEF)
      runUserDefFunction(pin, pf, ps);
  }
}

void checkCommands()
{
  // Check if user typed something into the serial monitor
  if (Serial.available()) {
    // Read out string from the serial monitor
    String input = Serial.readStringUntil('\n');

    Serial.print("# ");
    Serial.println(input);

    // Parse the user input into the CLI
    cli.parse(input);
  }

  if (cli.errored()) {
    CommandError cmdError = cli.getError();

    Serial.print("ERROR: ");
    Serial.println(cmdError.toString());

    if (cmdError.hasCommand()) {
      Serial.print("Did you mean \"");
      Serial.print(cmdError.getCommand().toString());
      Serial.println("\"?");
    }
  } 
}

void setup() 
{
  // Startup stuff
  Serial.begin(9600);
  VT100.begin(Serial); 

  cli.setOnError(errorCallback); // Set error Callback

  cmdChannel = cli.addSingleArgCmd("c", channelCallback); 
  cmdFunction = cli.addBoundlessCommand("f", functionCallback);
  cmdRun = cli.addCommand("f", runCallback);

  printConfigMenu();
}

void loop() 
{
  if (running)
  {
    runFunctions();
  }

  checkCommands();
}
