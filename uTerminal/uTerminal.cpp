#include "uTerminal.h"
 
uTerminal::uTerminal(PinName PinTX, PinName PinRX) {
    is_usb = 0;
    _serial = new Serial(PinTX, PinRX);
    _serial->attach(this, &uTerminal::_rx_irq, Serial::RxIrq);
    init();
}
 
uTerminal::uTerminal(Serial * serial_object) {
    is_usb = 0;
    _serial = serial_object;
    _serial->attach(this, &uTerminal::_rx_irq, Serial::RxIrq);
    init();
}
 
    
void uTerminal::init() {
    _callback = 0;
    cursor = 0;
    param_cursor = 0;
    buffer_len = 0;
    params_len = 0;
    has_command = 0;
    NumParams = 0;
    ParamLen = 0;
    auto_mode = 0;
}
 
bool uTerminal::readable() {
    return _serial->readable();
}
unsigned char uTerminal::getc() {
    return _serial->getc();
}
 
void uTerminal::ModeAuto() {
    auto_mode = 1;
}
 
void uTerminal::ModeManual() {
    auto_mode = 0;
}
 
void uTerminal::attach( void (*callback)() ) {
    _callback = callback;
}
 
void uTerminal::_rx_irq() {
    unsigned char val;
    if(readable()) {
        val = getc();
        if ((val == 10) || (val == 13)) {
            // Received a return character (Linux = 10, Windows = 13,10)
            // When #13#10 is received, the #10 will trigger this "if" with empty command. Ignore it.
            if (cursor > 0) {
                // Ok, we indeed have a command.
                has_command = 1; // 1-> Successful command (return). 2->Buffer full
                _buffer[cursor++] = 0x00; // make a null-terminated safe string
                buffer_len = cursor; // Includes terminating null
                cursor = 0;
                transfer_buffers(); // prepare the Command and Value buffers
            }
        }
        else {
            has_command = 0; // If had unread command in buffer, cancel it. Sorry, you lost it.
            _buffer[cursor++] = val;
            if (cursor >= UTERMINAL_BUF_SIZE) {
                // Even if it's not a return, we must not continue since buffer is full
                // Consider input done, but with result=2 instead of 1, to tell it's buffer size
                has_command = 2;
                buffer_len = UTERMINAL_BUF_SIZE; // all valid characters
                _buffer[cursor] = 0x00; // make a null-terminated safe string
                cursor = 0; // reset cursor to avoid overflow
                transfer_buffers(); // prepare the Command and Value buffers
            }
        }
    }
}
 
void uTerminal::transfer_buffers() {
    int i;
    int j;
    char c;
    
    // transfer cmd buffer
    i = 0;
    j = 0;
    while (i < buffer_len) {
        c = _buffer[i++];
        if ((c == '=') || (c == 0)) {
            Command[j] = 0; // null-terminated string
            break;
        }
        else {
            Command[j++] = c;
        }
    }
    if (i >= buffer_len) Command[j] = 0; // null-terminated safe
    
    // transfer param buffer
    if (c == '=') { // if we have parameters
        NumParams = 1;
        j = 0;
        while (i < buffer_len) {
            c = _buffer[i++];
            Value[j++] = c;
            if (c == ',') NumParams++;
            if (c == 0) break;
        }
        params_len = j; // length includes terminating null
    }
    else {
        Value[0] = 0; // empty string
        NumParams = 0;
        params_len = 0;
    }
    
    Param[0] = 0; // Param always begins as empty string
    ParamLen = 0;
    
    param_cursor = 0;
    
    // If in auto-mode, invoke the callback
    if (auto_mode) Process();
}
 
char* uTerminal::GetParam() {
    int i;
    char c;
    
    i = 0;
    while (param_cursor < params_len) {
        c = Value[param_cursor];
        if (c == ',') {
            Param[i] = 0;
            param_cursor++;
            break;
        }
        if (c == 0) {
            Param[i] = 0;
            break;
        }
        else {
            Param[i++] = c;
            param_cursor++;
        }
    }
    if (param_cursor >= params_len) Param[i] = 0; // null-terminated safe
    
    ParamLen = i; // does not include terminating null
    
    return Param;
}
 
 
int uTerminal::Process() {
    // Be ready for next message in the background
    int _has_cmd = has_command;
    has_command = 0;
 
    // invoke the callback
    if ((_has_cmd > 0) && (_callback != 0)) _callback();
    
    // Return status
    return _has_cmd;
}
 
void uTerminal::print(char* str) {
    _serial->printf(str);
}
 
void uTerminal::print(const char* str) {
    print((char*)str);
}