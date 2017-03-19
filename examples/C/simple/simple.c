#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <hidapi.h>
#include <simple.h>
#include <usbhid_buddy.h>
#include <support.h>
#include <utility.h>

#include <sys/timeb.h>

static hid_device* hid_handle;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

BOOL my_exit_handler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		case CTRL_C_EVENT:
			printf("Ctrl-C detected\r\n");
			buddy_cleanup(hid_handle);
			exit(1);
			return TRUE;

		default:
			break;
	}

	return FALSE;
}
#else
#include <signal.h>
void my_exit_handler(int s)
{
	printf("my_exit_handler caught\n");
	exit(1);
}
#endif

int8_t test_seq_dac(hid_device** handle, float sample_rate, bool streaming, bool oneshot) {
	static int test_seq_dac_count = 0;
	general_packet_t packet;
	ctrl_general_t general_settings = { 0 };
	ctrl_timing_t timing_settings = { 0 };
	ctrl_runtime_t runtime_settings = { 0 };
	uint8_t ctrl_mode;
	int k;
	int i;

	general_settings.function = GENERAL_CTRL_DAC_ENABLE;
	general_settings.mode = (streaming ? MODE_CTRL_STREAM : MODE_CTRL_IMMEDIATE);
	general_settings.operation = (oneshot ? OPER_CTRL_ONESHOT : OPER_CTRL_CONTINUOUS);
	
	//general_settings.queue = QUEUE_CTRL_WRAP;
	general_settings.queue = QUEUE_CTRL_SATURATE;

	general_settings.channel_mask = BUDDY_CHAN_ALL_MASK;
	//general_settings.channel_mask = BUDDY_CHAN_0_MASK;
	
	general_settings.resolution = CODEC_BIT_WIDTH_12;
	//general_settings.resolution = CODEC_BIT_WIDTH_8;

	timing_settings.period = FREQUENCY_TO_NSEC(sample_rate);
	
	runtime_settings.dac_power = RUNTIME_DAC_POWER_ON;
	
	runtime_settings.dac_ref = RUNTIME_DAC_REF_EXT;
	//runtime_settings.dac_ref = RUNTIME_DAC_REF_INT_1V;
	//runtime_settings.dac_ref = RUNTIME_DAC_REF_INT_2V;

	*handle = buddy_init(&general_settings, &runtime_settings, &timing_settings);
	if (!*handle) {
		printf("test_seq_dac: could not buddy_init\n");
		return -1;
	}

	for (k = 0; k <= ((1 << general_settings.resolution) - 1); k++) {
		for (i = BUDDY_CHAN_7; i <= BUDDY_CHAN_0; i++) {
			packet.channels[i] = k;
		}

		/*
		for (i = BUDDY_CHAN_7; i <= BUDDY_CHAN_0; i++) {
			packet.channels[i] = ((k % 2) == 0) ? 2048 : 0;
		}
		*/

		printf("test_seq_dac, sending %d packet with value %d\r\n", test_seq_dac_count++, k);

		if (buddy_send_dac(*handle, &packet, streaming) != BUDDY_ERROR_OK) {
			printf("test_seq_dac: buddy_send_dac call failed\n");
			return BUDDY_ERROR_GENERAL;
		}

		// if streaming is not enabled then wait explicitly but otherwise
		// call to send the next packet and let the encoder work
		if (!streaming) {
			short_sleep(((1.0 / sample_rate) / 1e-3));
		} else {
			short_sleep(1);
		}
	}

	// flush the stream frame buffer if packets are in it but did not fill whole frame
	// to send packet, then wait fixed period to allow MCU to process frame
	if (!codec_buffer_empty()) {
		printf("flushing the buffer\r\n");
		buddy_flush(*handle);
	}
	short_sleep(100);

	if (oneshot) {
		if (buddy_trigger(*handle) != BUDDY_ERROR_OK) {
			return BUDDY_ERROR_GENERAL;
		}
		short_sleep(1000);
	}

	return 0;
}

int8_t test_seq_adc(hid_device** handle, float sample_rate, bool streaming, bool oneshot) {
	static int test_seq_dac_count = 0;
	general_packet_t packet;
	ctrl_general_t general_settings = { 0 };
	ctrl_timing_t timing_settings = { 0 };
	ctrl_runtime_t runtime_settings = { 0 };
	uint8_t ctrl_mode;
	int i;
	int err_code;
	int recv_packets;

	general_settings.function = GENERAL_CTRL_ADC_ENABLE;
	general_settings.mode = (streaming ? MODE_CTRL_STREAM : MODE_CTRL_IMMEDIATE);
	general_settings.operation = (oneshot ? OPER_CTRL_ONESHOT : OPER_CTRL_CONTINUOUS);
	general_settings.queue = QUEUE_CTRL_SATURATE;
	general_settings.channel_mask = BUDDY_CHAN_ALL_MASK;
	//general_settings.channel_mask = BUDDY_CHAN_2_MASK;
	//general_settings.channel_mask = BUDDY_CHAN_0_MASK;
	
	//general_settings.resolution = CODEC_BIT_WIDTH_8;
	//general_settings.resolution = CODEC_BIT_WIDTH_10;
	general_settings.resolution = CODEC_BIT_WIDTH_12;

	timing_settings.period = FREQUENCY_TO_NSEC(sample_rate);
	timing_settings.averaging = 1;

	runtime_settings.adc_ref = RUNTIME_ADC_REF_VDD;

	printf("timing_settings.period = %d (0x%x)\n", timing_settings.period, timing_settings.period);

	*handle = buddy_init(&general_settings, &runtime_settings, &timing_settings);
	if (!*handle) {
		printf("test_seq_adc: could not buddy_init\n");
		return -1;
	}
	short_sleep(100);

	if (oneshot) {
		if (buddy_trigger(*handle) != BUDDY_ERROR_OK) {
			return BUDDY_ERROR_GENERAL;
		}
		short_sleep(1000);
	}

	recv_packets = 0;
	do {
		err_code = buddy_read_adc(*handle, &packet, streaming);
		if (err_code == BUDDY_ERROR_OK) {
			printf("test_seq_adc: received packet %d\n", recv_packets);

			for (i = BUDDY_CHAN_7; i <= BUDDY_CHAN_0; i++) {
				if (general_settings.channel_mask & (1 << i)) {
					printf("packet.channels[%d] = %d\r\n", i, packet.channels[i]);
				}
			}

			recv_packets++;
		} else if (err_code == BUDDY_ERROR_GENERAL) {
			printf("test_seq_adc: buddy_send_adc call failed\n");
			return BUDDY_ERROR_GENERAL;
		}
	} while (recv_packets < 1200);

	return 0;
}

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char *argv[])
{
	int optind;
	int8_t err_code;
	struct timeb time_start, time_end;
	int time_diff;
	bool oneshot_mode = false;
	bool stream_mode = false;
	enum { DAQ_MODE, ADC_MODE, DUAL_MODE } mode = DAQ_MODE;

	#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	#endif
	
	#if defined(_WIN32) || defined(_WIN64)
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)my_exit_handler, TRUE))
	{
		printf("\nERROR: Could not set control handler\r\n");
		return 1;
	}
	#else
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = my_exit_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);
	#endif

    for (optind = 1; optind < argc; optind++) {
		switch (argv[optind][1]) {
			case 'd': mode = DAQ_MODE; break;
			case 'a': mode = ADC_MODE; break;
			case 'p': mode = DUAL_MODE; break;
			case 's': stream_mode = true; break;
			case 'o': oneshot_mode = true; break;
			case 'h': 
			default:
				fprintf(stderr, "Usage: %s [-dapos]\n", argv[0]);
				exit(EXIT_FAILURE);
        } 
    }

	printf("stream_mode = %d\r\n", stream_mode);
	printf("mode = %d\r\n", mode);

	ftime(&time_start);
	if (mode == DAQ_MODE) {
		printf("main: testing with mode = DAQ_MODE\n");
		err_code = test_seq_dac(&hid_handle, BUDDY_TEST_DAC_FREQ, stream_mode, oneshot_mode);
	} else if (mode == ADC_MODE) {
		printf("main: testing with mode = ADC_MODE\n");
		err_code = test_seq_adc(&hid_handle, BUDDY_TEST_ADC_FREQ, stream_mode, oneshot_mode);
	} 
	ftime(&time_end);
	time_diff = (int)(1000.0 * (time_end.time - time_start.time) + (time_end.millitm - time_start.millitm));
	printf("Test took (%f) seconds  to run\n", (float) (time_diff / 1000.0));

	if (err_code == 0) {
		buddy_cleanup(hid_handle);
	} else {
		printf("main: could not complete requested buddy action\n");
	}

	return 0;
}