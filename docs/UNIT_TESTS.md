lib/libbuddy/
 - utility(.h/.c):
  * debug_init
  * debug_write
  * debug_cleanup

 - usbhid_buddy(.h/.c):
  x * reset_codec
  x * encode
  x * decode
  * hidapi_init
  * buddy_write_raw
  * buddy_write_packet
  * buddy_read_packet
  * buddy_send_generic
  * buddy_read_generic
  * buddy_read_generic_noblock
  * buddy_send_pwm
  * buddy_send_dac
  * buddy_read_counter
  * buddy_read_adc
  * buddy_read_adc_noblock
  * buddy_clear
  * buddy_flush
  * buddy_count_channels
  * buddy_configure
  * buddy_get_response
  * buddy_reset_device
  * buddy_get_firmware_info
  * buddy_init
  * buddy_trigger
  * buddy_cleanup

 - support(.h/.c):
  x * swap_uint16
  x * swap_int16
  x * swap_uint32
  x * swap_int32
  x * short_sleep