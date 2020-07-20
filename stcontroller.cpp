#include "stcontroller.h"


// List of constants for node output pins
enum OutputList {
	OUTPUT_LEDS,
	OUTPUT_IRCUT,
	OUTPUT_IRLED,
	OUTPUT_LENS,
	OUTPUT_TEMP,
	OUTPUT_METER
};


nBlock_StController::nBlock_StController(PinName pinTX, PinName pinRX): _ser(pinTX, pinRX) {
	// Reset values for LEDs
	for (int i=0; i<(NUM_PIXELS*3); i++) NeoPixelValues[i] = 0;
	
	// Configures the serial port
	_ser.baud ( 115200 );
	// configures the terminal interpreter
	terminal_pi = new uTerminal(&_ser);
	terminal_pi->ModeManual();    return;
}
void nBlock_StController::triggerInput(uint32_t inputNumber, uint32_t value) { // inputNumber is ignored
    char * string_buf = (char *)(value);
	
	terminal_pi->print(string_buf);
}
void nBlock_StController::endFrame(void) {
	//output[0] = <data>
	//available[0] = 1;
    
	// Do we have a complete message to process?
	if (terminal_pi->Process()) {
		// Interpret it
		processCmd();
	}
    return;
}

/*******************************************************************//**
*  COMMAND PROCESSING
***********************************************************************/

  

enum Error {
    ERR_UNKNOWN_CMD,
    ERR_DATA_LEN,
    ERR_PARAM_LEN,
    ERR_INVALID_VALUE
};

/** \brief Converts hex digit ('0' - 'F') into a decimal value (0-15) */
uint8_t HexToDec(char hex) {
    if ((hex >= '0') && (hex <= '9')) return hex - '0';
    else if ((hex >= 'A') && (hex <= 'F')) return 10 + (hex - 'A');
    else if ((hex >= 'a') && (hex <= 'f')) return 10 + (hex - 'a');
    else return 0; // fail safe defaults to 0
}

/**
 *  \brief Interprets the command available in terminal buffer.
 *  Called by endFrame() when a command is already available.
 */
void nBlock_StController::processCmd(void) {
	// If this method was invoked, we have a message to be interpreted
	
	// Response buffer to be filled with sprintf
	char response[256];
	uint32_t i;
	uint32_t err;
	
	//  "AT"
    if (strcmp(terminal_pi->Command,"AT") == 0) {
        terminal_pi->print("OK\n");
    }
	
	// "LEDS=FF,FF,FF,... ,FF"
	// There should be (3*NUM_PIXELS) arguments of 2 chars (hex value)
	// Since each argument is one color channel and LEDs are RGB
	else if (strcmp(terminal_pi->Command,"LEDS") == 0) {
		// Correct number of parameters?
        if (terminal_pi->NumParams == ( NUM_PIXELS * 3UL )) {
			// For each pixel:
            for (i=0; i<( NUM_PIXELS * 3UL ); i++) {
                err = 0;
				// Retrieve next parameter from buffer string
                terminal_pi->GetParam();                                         // loads first/next parameter into terminal_pi.Param
                
				// A correct parameter must have 2 chars
                if (terminal_pi->ParamLen == 2) {
					
					// Convert the two chars (hex representation) into one byte
					// this byte is one of the channels of one of the LEDs
                    NeoPixelValues[i] = ( HexToDec(terminal_pi->Param[0]) << 4)
                                      + ( HexToDec(terminal_pi->Param[1])     );      // Hex byte to uint8_t
									  
                }
                else {
                    // This parameter has invalid length
                    sprintf(response, "ERR=%d P=%d\n", ERR_PARAM_LEN, i);
                    terminal_pi->print(response);
                    err++;
                }
            }
            if (!err) {
                terminal_pi->print("OK\n");                               // If no errors print OK
				output   [OUTPUT_LEDS] = (uint32_t)(&NeoPixelValues);
				available[OUTPUT_LEDS] = ( NUM_PIXELS * 3UL );
            }
        }
		// Wrong number of parameters
        else {
            // Error on data length, not enough values
            sprintf(response, "ERR=%d\n", ERR_DATA_LEN);
            terminal_pi->print(response);
        }
    }
	
	
	// IRCUT=0 / IRCUT=1
    else if (strcmp(terminal_pi->Command,"IRCUT") == 0) {
        if (terminal_pi->NumParams == 1) {
            // Valid command has 1 parameter
            terminal_pi->GetParam();                                             // loads first/next parameter into terminal_pi.Param
            err = 0;

			// The correct parameter value must be i char ('0' or '1')
            if (terminal_pi->ParamLen == 1) {
                switch (terminal_pi->Param[0]) {
                    case '1':
						output   [OUTPUT_IRCUT] = 1;
						available[OUTPUT_IRCUT] = 1;
						terminal_pi->print("IRCUT: 1\n");
                        break;
                    case '0':
						output   [OUTPUT_IRCUT] = 0;
						available[OUTPUT_IRCUT] = 1;
						terminal_pi->print("IRCUT: 0\n");
                        break;
                    default:
                        err++;
                        break;
                }
                if (!err) {
                    terminal_pi->print("OK\n");                                  // If no errors print OK
                }
                else {
                    // This parameter has invalid value
                    sprintf(response, "ERR=%d\n", ERR_INVALID_VALUE);
                    terminal_pi->print(response);
                }
            }
            else {
                // This parameter has invalid length
                sprintf(response, "ERR=%d\n", ERR_PARAM_LEN);
                terminal_pi->print(response);
            }
        }
        else {
            // Error on data length, not 1 values
            sprintf(response, "ERR=%d\n", ERR_DATA_LEN);
            terminal_pi->print(response);
        }
        
    }
	
	
	
	// IRLED=<8 bit duty cycle in hex 00-FF>
    else if (strcmp(terminal_pi->Command,"IRLED") == 0) {
        if (terminal_pi->NumParams == 1) {
            // Valid command has 1 parameter
            terminal_pi->GetParam();                                             // loads first/next parameter into terminal_pi.Param
            err = 0;

			// A correct parameter must have 2 chars
            if (terminal_pi->ParamLen == 2) {
                
				// Convert the two chars (hex representation) into one byte
				// this byte is the pwm duty cycle in range 0-255
                uint8_t pwm_val = ( HexToDec(terminal_pi->Param[0]) << 4)
                                + ( HexToDec(terminal_pi->Param[1])      );      // Hex byte to uint8_t
								
				// PWM nodes expect 0xFFFF mask range
				output   [OUTPUT_IRLED] = pwm_val << 8; 
				available[OUTPUT_IRLED] = 1;
                
                terminal_pi->print("OK\n");                                  // If no errors print OK

            }
            else {
                // This parameter has invalid length
                sprintf(response, "ERR=%d\n", ERR_PARAM_LEN);
                terminal_pi->print(response);
            }
        
        }
        else {
            // Error on data length, not 1 values
            sprintf(response, "ERR=%d\n", ERR_DATA_LEN);
            terminal_pi->print(response);
        }
        
    }   	
	
	
	
	// LENS=<8 bit value in hex 00-FF>
    else if (strcmp(terminal_pi->Command,"LENS") == 0) {
        if (terminal_pi->NumParams == 1) {
            // Valid command has 1 parameter
            terminal_pi->GetParam();                                             // loads first/next parameter into terminal_pi.Param
            err = 0;

			// A correct parameter must have 2 chars
            if (terminal_pi->ParamLen == 2) {
                
				// Convert the two chars (hex representation) into one byte
				// this byte is the lens voltage intensity in range 0-255
                uint8_t lens_val = ( HexToDec(terminal_pi->Param[0]) << 4 )
                                 + ( HexToDec(terminal_pi->Param[1])      );      // Hex byte to uint8_t
                
				output   [OUTPUT_LENS] = lens_val; // LENS node expects raw byte
				available[OUTPUT_LENS] = 1;                                          

                terminal_pi->print("OK\n");                                  // If no errors print OK
            }
            else {
                // This parameter has invalid length
                sprintf(response, "ERR=%d\n", ERR_PARAM_LEN);
                terminal_pi->print(response);
            }
        
        }
        else {
            // Error on data length, not 1 values
            sprintf(response, "ERR=%d\n", ERR_DATA_LEN);
            terminal_pi->print(response);
        }
        
    }     	
	
	//  "TEMP"
    else if (strcmp(terminal_pi->Command,"TEMP") == 0) {
        terminal_pi->print("OK\n");
		output   [OUTPUT_TEMP] = 1; // TEMP output is a trigger
		available[OUTPUT_TEMP] = 1; 
	}
	
	//  "METER"
    else if (strcmp(terminal_pi->Command,"METER") == 0) {
        terminal_pi->print("OK\n");
		output   [OUTPUT_METER] = 1; // METER output is a trigger
		available[OUTPUT_METER] = 1; 
	}

	// UNKNOWN COMMAND
    else {
        // Unknown command
        sprintf(response, "ERR=%d\n", ERR_UNKNOWN_CMD);
        terminal_pi->print(response);
    }	
	
}

