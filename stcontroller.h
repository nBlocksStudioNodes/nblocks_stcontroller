#ifndef __NB_STCONTROLLER
#define __NB_STCONTROLLER

#include "nworkbench.h"
#include "uTerminal/uTerminal.h"

#define NUM_PIXELS     12   

class nBlock_StController: public nBlockSimpleNode<4> {
public:
    nBlock_StController(PinName pinTX, PinName pinRX);
    void triggerInput(uint32_t inputNumber, uint32_t value);
    void endFrame(void);
private:
	// Serial port object - raw data
    Serial _ser;
	// Pointer to uTerminal object - AT command style
	// Will be created and assigned in constructor
	uTerminal * terminal_pi;
	
	// Array holding the values for Neopixel LEDs
	uint8_t NeoPixelValues[NUM_PIXELS*3];
	
	// Decodes messages received from uTerminal
	void processCmd(void);
};

#endif
