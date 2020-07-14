//// MODIFIED VERSION TO REMOVE THE USB
//// ORIGINAL IS AT https://os.mbed.com/users/fbcosentino/code/uTerminal/
 
/** 
 *  @file uTerminal.h
 *  @brief uTerminal is a simple AT-command style terminal processor,
 *  simplifying the work to send and parse COMMAND=value pairs to a
 *  microcontroller.
 * 
 *  You can use COMMAND=value syntax, as well as
 *  COMMAND=value1,value2,value3,... to sendo multiple parameters.
 *  Commands and values can be any string, as long as they do not contain
 *  the following chars: '=', '\n', '\r' or ','.
 *
 *  @author  Fernando Cosentino
 */
 
#ifndef _UTERMINAL
#define _UTERMINAL
 
#include "mbed.h"
 
#define UTERMINAL_BUF_SIZE 512
 
/** uTerminal processes messages from a serial port,
 * parsing as a "COMMAND=value" pair,
 * or "COMMAND=value1,value2,value3,..." set.
 */
class uTerminal {
public:
 
    /** Instances an uTerminal object.
     * @param PinTX TX pin for the serial port.
     * @param PinRX RX pin for the serial port.
     */
    uTerminal(PinName PinTX, PinName PinRX);
    /** Instances an uTerminal object.
     * @param serial_object An instance of a Serial object
     */
    uTerminal(Serial * serial_object);
    
    
    /** Attaches an external function to be called whenever a new command
     * is received. Format for the callback is: void function_name()
     * @param callback The callback function to attach to
     */
    void attach( void (*callback)() );
    
    /** Process the internal buffer and check for complete messages.
     *  Invokes the callback as appropriate.
     *  @returns
     *    0 if command not yet present
     *    1 if a command is present ended by a new line
     *    2 if there is content filling the entire buffer
     */
    int Process();
    
    /** Sets automatic mode: whenever a message is ready, Process() is
     *  automatically called. Use in conjunction with your own interrupt
     * service routine, attached as callback via attach().
     */
    void ModeAuto();
 
    /** Sets manual mode: whenever a message is ready, you have to call 
     *  Process() to prepare the message before acting on it. This way
     *  you can decide the best time to poll Process() (useful in
     *  time-consuming code).
     *  This is the default mode.
     */
    void ModeManual();
 
    /** Retrieves the first or next parameter from the received message.
     * In the first call, fetches the first parameter. All subsequent calls
     * will return the next parameter.
     * @returns pointer to first char (that is, a C string)
     */
    char* GetParam();
    
    /** When a new message is received, NumParams contains the
     * number of parameters available to be read. If it is zero,
     * the received message is just a command
     */
     int NumParams;
     
    /** String buffer (array of char) containing the received command
     *  (in a message like "BAUD=9600,1" the command would be "BAUD")
     */
    char Command[UTERMINAL_BUF_SIZE+1];
    
    /** String buffer (array of char) containing the received value
     *  (in a message like "BAUD=9600,1" the value would be "9600,1")
     */
    char Value[UTERMINAL_BUF_SIZE+1];
 
    /** String buffer (array of char) containing the current parameter taken
     * from the value. (In a message like "BAUD=9600,1", after the first call to
     * GetParam() Param would be "9600", and after the second call Param 
     * would be "1".)
     */
    char Param[UTERMINAL_BUF_SIZE+1];
    
    /** Length of useful data in Param.
     * Param is actually safe to be used as a null-terminated string,
     * but if the length of actual data in it is required this is it.
     */
    int ParamLen;
    
    /** Prints a string to serial port.
     *  If you want to use printf formatting functionalities, use sprintf
     *  to output your formatted data into a string buffer and then call
     * this method passing your buffer as parameter.
     * @param str String to be sent to serial port
     */
    void print(char* str);
    void print(const char* str);
      
private:
    Serial * _serial;
    void init();
    bool readable();
    unsigned char getc();
    void (*_callback)();
    void _rx_irq();
    void transfer_buffers();
    
    char _buffer[UTERMINAL_BUF_SIZE+1];
    int buffer_len;
    int params_len;
    int cursor;
    int param_cursor;
    int has_command;
    int auto_mode;
    int is_usb;
};
 
#endif