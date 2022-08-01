/* 
    Adapt an NES Classic or Nunchuk controller to be used on an NES
    Reads controller via I2C and emulates a shift register like the original NES controller
    Joe Ostrander
    2022.07.31

    NES Classic Library:
    https://github.com/dmadison/NintendoExtensionCtrl

*/

#include <NintendoExtensionCtrl.h>

#define PIN_CLOCK 2
#define PIN_LATCH 3
#define PIN_DATA  4

volatile bool reading = false;
volatile uint8_t data = 0;
volatile uint8_t pos = 0;

static void clock_changed(void);
static void latch_changed(void);

typedef enum
{
    BUTTON_A = 0,
    BUTTON_B,
    BUTTON_SELECT,
    BUTTON_START,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_COUNT
} controller_button_t;

ClassicController classic;

void setup(void) 
{
    Serial.begin(115200);
	classic.begin();

	while (!classic.connect()) 
    {
		Serial.println("Classic Controller not detected!");
		delay(1000);
	}

  pinMode(PIN_CLOCK, INPUT);
  pinMode(PIN_LATCH, INPUT);
  pinMode(PIN_DATA, OUTPUT);

  digitalWrite(PIN_DATA, HIGH);

  attachInterrupt(digitalPinToInterrupt(PIN_CLOCK), clock_changed, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_LATCH), latch_changed, CHANGE);
}

void loop(void)
{
	bool success = classic.update();  // Get new data from the controller

	if (success) 
    {
		//classic.printDebug();  // Print all of the values!
        uint8_t new_data = 0xFF;
        if (classic.buttonA())      {new_data &= ~(1<<BUTTON_A);}
        if (classic.buttonB())      {new_data &= ~(1<<BUTTON_B);}
        if (classic.buttonSelect()) {new_data &= ~(1<<BUTTON_SELECT);}
        if (classic.buttonStart())  {new_data &= ~(1<<BUTTON_START);}
        if (classic.dpadUp())       {new_data &= ~(1<<BUTTON_UP);}
        if (classic.dpadDown())     {new_data &= ~(1<<BUTTON_DOWN);}
        if (classic.dpadLeft())     {new_data &= ~(1<<BUTTON_LEFT);}
        if (classic.dpadRight())    {new_data &= ~(1<<BUTTON_RIGHT);}
        
        //Serial.println(new_data, BIN);
        data = new_data;
	}
	else 
    {  // Data is bad :(
		Serial.println("Controller Disconnected!");
		delay(1000);
		classic.connect();
	}
}

static void clock_changed(void)
{
    if (reading)
    {
        if (digitalRead(PIN_CLOCK))
        {
            pos++;
            pos = pos >= 8 ? 0 : pos;
        }
        else
        {
            digitalWrite(PIN_DATA, ((data >> pos) & 1));
        }
    }
}

static void latch_changed(void)
{
    if (digitalRead(PIN_LATCH))
    {
        reading = false;
    }
    else
    {
        reading = true;
        pos = 0;
    }
}
