import _buddy
import time
import ctypes

def test_seq_adc():
	print 'test_seq_adc'

def test_seq_dac():
	print 'test_seq_dac'

	# build array looping from 0 to fullscale four times
	for k in range(0, 4095 * 4):
		out_buf = [_buddy.USBHID_OUT_DATA_ID, _buddy.APP_CODE_DAC]
		out_index = 2

		for i in range(0, _buddy.DAC_CHAN_7):
			lo_byte = (k & 0x00FF)
			hi_byte = ((k & 0xF00) >> 8)

			out_buf.append(lo_byte)
			out_buf.append(hi_byte)

		#print tuple(out_buf)

		_buddy.usbhid_write_packet(tuple(out_buf))
		time.sleep(0.001)

_buddy.usbhid_buddy_init(0x01)
test_seq_dac()
_buddy.usbhid_buddy_cleanup()

if __name__ == '__main__':
	print 'buddy test application'