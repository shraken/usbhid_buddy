/* usbhid_buddy.i */

 %module buddy
 %{
#define SWIG_FILE_WITH_INIT
#include "usbhid_buddy.h"
#include "buddy.h"

 %}

int FREQUENCY_TO_NSEC(int);

%include "stdint.i"
%include "carrays.i"
%include <windows.i>
%include "usbhid_buddy.h"
%include "buddy.h"

%array_functions(float, float_ptr);
%array_functions(double, double_ptr);
%array_functions(int8_t, int8_t_ptr);
%array_functions(uint8_t, uint8_t_ptr);
%array_functions(int16_t, int16_t_ptr);
%array_functions(uint16_t, uint16_t_ptr);
%array_functions(int32_t, int32_t_ptr);
%array_functions(uint32_t, uint32_t_ptr);
%array_functions(char *, char_ptr);