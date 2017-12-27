#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <hidapi.h>
#include <simple.h>
#include <buddy.h>
#include <usbhid_buddy.h>
#include <support.h>
#include <utility.h>

#include <sys/timeb.h>

extern char *fw_info_dac_type_names[FIRMWARE_INFO_DAC_TYPE_LENGTH];

static hid_device* hid_handle;
static bool active = false;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

BOOL my_exit_handler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		case CTRL_C_EVENT:
			printf("Ctrl-C detected\r\n");
			active = false;
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

void display_usb_info(buddy_hid_info_t *hid_info)
{
	printf("\nUSB Manufacturer String: %s\n", hid_info->str_mfr);
	printf("USB Product String: %s\n", hid_info->str_product);
	printf("USB Serial Number String: %s\n", hid_info->str_serial);
	printf("USB Indexed String 1: %s\n\n", hid_info->str_index_1);
}

void display_fw_info(firmware_info_t *fw_info)
{
	char res[128];
	struct tm *timeinfo;
	time_t flashtime;

	printf("Firmware Serial Number: %d (0x%x)\n", 
		fw_info->serial, fw_info->serial);
	printf("Firmware Revision: %d.%d.%d\n",
		fw_info->fw_rev_major, fw_info->fw_rev_minor, fw_info->fw_rev_tiny);
	printf("Firmware Bootload Revision: %d.%d.%d\n",
		fw_info->bootl_rev_major, fw_info->bootl_rev_minor, fw_info->bootl_rev_tiny);
	
	flashtime = fw_info->flash_datetime;
	timeinfo = localtime( &flashtime );

	if (strftime(res, sizeof(res), "%Y-%m-%d %H:%M:%S", timeinfo) > 0) {
		printf("Firmware Flash datetime: %s\n", res);
	} else {
		printf("Firmware Flash datetime: none\n");
	}

	if (fw_info->type_dac < FIRMWARE_INFO_DAC_TYPE_LENGTH) {
		printf("Firmware DAC type: %d - %s\n",
			fw_info->type_dac, fw_info_dac_type_names[fw_info->type_dac]);
	} else {
		printf("Firmware DAC type: %d\n",
			fw_info->type_dac);
	}
}

int8_t test_seq_pwm_freq(hid_device* handle, firmware_info_t *fw_info,
	float sample_rate, bool streaming)
{
	static int test_seq_pwm_count = 0;
	general_packet_t packet;
	ctrl_general_t general_settings = { 0 };
	ctrl_timing_t timing_settings = { 0 };
	ctrl_runtime_t runtime_settings = { 0 };
	int k;
	int i;
	int f;

	active = true;
	general_settings.function = GENERAL_CTRL_PWM_ENABLE;
	general_settings.mode = (streaming ? MODE_CTRL_STREAM : MODE_CTRL_IMMEDIATE);
	general_settings.channel_mask = BUDDY_CHAN_0_MASK;
	general_settings.resolution = RESOLUTION_CTRL_SUPER;
	//general_settings.resolution = RESOLUTION_CTRL_HIGH;
	//general_settings.resolution = RESOLUTION_CTRL_LOW;

	timing_settings.period = (uint32_t)FREQUENCY_TO_NSEC(sample_rate);

	runtime_settings.pwm_mode = RUNTIME_PWM_MODE_FREQUENCY;
	runtime_settings.pwm_timebase = RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_12;

	if (buddy_configure(handle, &general_settings, &runtime_settings, &timing_settings) != BUDDY_ERROR_CODE_OK) {
		printf("test_seq_pwm_freq: could not buddy_init\n");
		return -1;
	}
	short_sleep(100);

	//for (k = 20000; k <= 20000; k++) {
	for (k = 100000; k <= 100000; k++) {
		if (!active) {
			return 0;
		}

		for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
			packet.channels[i] = k;
		}

		printf("test_seq_pwm_freq, sending %d packet with value %d (%x)\r\n", test_seq_pwm_count++, k, k);

		if (buddy_send_pwm(handle, &packet, streaming) != BUDDY_ERROR_CODE_OK) {
			printf("test_seq_pwm_freq: buddy_send_pwm call failed\n");
			return -1;
		}

		// if streaming is not enabled then wait explicitly but otherwise
		// call to send the next packet and let the encoder work
		if (!streaming) {
			short_sleep((int)((1.0 / sample_rate) / 1e-3));
		}
	}

	short_sleep(10);
	// flush the stream frame buffer if packets are in it but did not fill whole frame
	// to send packet, then wait fixed period to allow MCU to process frame
	buddy_flush(handle);

	return 0;
}

int8_t test_seq_pwm_duty(hid_device* handle, firmware_info_t *fw_info,
	float sample_rate, bool streaming)
{
	static int test_seq_pwm_count = 0;
	general_packet_t packet;
	ctrl_general_t general_settings = { 0 };
	ctrl_timing_t timing_settings = { 0 };
	ctrl_runtime_t runtime_settings = { 0 };
	int k;
	int i;
	int f;

	active = true;
	general_settings.function = GENERAL_CTRL_PWM_ENABLE;
	general_settings.mode = (streaming ? MODE_CTRL_STREAM : MODE_CTRL_IMMEDIATE);
	general_settings.channel_mask = BUDDY_CHAN_0_MASK;
	general_settings.resolution = RESOLUTION_CTRL_HIGH;
	//general_settings.resolution = RESOLUTION_CTRL_LOW;

	timing_settings.period = (uint32_t)FREQUENCY_TO_NSEC(sample_rate);

	runtime_settings.pwm_mode = RUNTIME_PWM_MODE_DUTY_CYCLE;
	runtime_settings.pwm_timebase = RUNTIME_PWM_TIMEBASE_SYSCLK;

	if (buddy_configure(handle, &general_settings, &runtime_settings, &timing_settings) != BUDDY_ERROR_CODE_OK) {
		printf("test_seq_pwm_duty: could not buddy_init\n");
		return -1;
	}
	short_sleep(100);

	for (k = 32767; k <= 32767; k++) {
	//for (k = 16383; k <= 16383; k++) {
	//for (k = 128; k <= 128; k++) {
		if (!active) {
			return 0;
		}

		for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
			packet.channels[i] = k;
		}

		printf("test_seq_pwm_duty, sending %d packet with value %d (%x)\r\n", test_seq_pwm_count++, k, k);

		if (buddy_send_pwm(handle, &packet, streaming) != BUDDY_ERROR_CODE_OK) {
			printf("test_seq_pwm_duty: buddy_send_pwm call failed\n");
			return -1;
		}

		// if streaming is not enabled then wait explicitly but otherwise
		// call to send the next packet and let the encoder work
		if (!streaming) {
			short_sleep((int)((1.0 / sample_rate) / 1e-3));
		}
	}

	short_sleep(10);
	// flush the stream frame buffer if packets are in it but did not fill whole frame
	// to send packet, then wait fixed period to allow MCU to process frame
	buddy_flush(handle);

	return 0;
}

int8_t test_seq_dac(hid_device* handle, firmware_info_t *fw_info, 
				float sample_rate, bool streaming)
{
	static int test_seq_dac_count = 0;
	general_packet_t packet;
	ctrl_general_t general_settings = { 0 };
	ctrl_timing_t timing_settings = { 0 };
	ctrl_runtime_t runtime_settings = { 0 };
	int k;
	int i;
	int f;

	active = true;
	general_settings.function = GENERAL_CTRL_DAC_ENABLE;
	general_settings.mode = (streaming ? MODE_CTRL_STREAM : MODE_CTRL_IMMEDIATE);
	//general_settings.channel_mask = BUDDY_CHAN_ALL_MASK;
	general_settings.channel_mask = BUDDY_CHAN_0_MASK;
	general_settings.resolution = RESOLUTION_CTRL_HIGH;
	//general_settings.resolution = RESOLUTION_CTRL_LOW;

	timing_settings.period = (uint32_t) FREQUENCY_TO_NSEC(sample_rate);
	
	runtime_settings.dac_power = RUNTIME_DAC_POWER_ON;
	runtime_settings.dac_ref = RUNTIME_DAC_REF_EXT;
	//runtime_settings.dac_ref = RUNTIME_DAC_REF_INT_1V;
	//runtime_settings.dac_ref = RUNTIME_DAC_REF_INT_2V;

	if 	(buddy_configure(handle, &general_settings, &runtime_settings, &timing_settings) != BUDDY_ERROR_CODE_OK) {
		printf("test_seq_dac: could not buddy_init\n");
		return -1;
	}
	short_sleep(100);

	for (k = 0; k <= 4095; k++) {
		if (!active) {
			return 0;
		}
		
		for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
			packet.channels[i] = k;
		}

		//printf("test_seq_dac, sending %d packet with value %d\r\n", test_seq_dac_count++, (k % 255));
		printf("test_seq_dac, sending %d packet with value %d (%x)\r\n", test_seq_dac_count++, k, k);

		if (buddy_send_dac(handle, &packet, streaming) != BUDDY_ERROR_CODE_OK) {
			printf("test_seq_dac: buddy_send_dac call failed\n");
			return -1;
		}

		// if streaming is not enabled then wait explicitly but otherwise
		// call to send the next packet and let the encoder work
		if (!streaming) {
			short_sleep( (int) ((1.0 / sample_rate) / 1e-3));
		} 
	}

	short_sleep(10);
	// flush the stream frame buffer if packets are in it but did not fill whole frame
	// to send packet, then wait fixed period to allow MCU to process frame
	buddy_flush(handle);

	return 0;
}

int8_t test_seq_adc(hid_device* handle, firmware_info_t *fw_info,
				float sample_rate, bool streaming) {
	static int test_seq_dac_count = 0;
	general_packet_t packet;
	ctrl_general_t general_settings = { 0 };
	ctrl_timing_t timing_settings = { 0 };
	ctrl_runtime_t runtime_settings = { 0 };
	int i;
	int err_code;
	int recv_packets;

	active = true;
	general_settings.function = GENERAL_CTRL_ADC_ENABLE;
	general_settings.mode = (streaming ? MODE_CTRL_STREAM : MODE_CTRL_IMMEDIATE);
	//general_settings.channel_mask = BUDDY_CHAN_ALL_MASK;
	//general_settings.channel_mask = BUDDY_CHAN_6_MASK | BUDDY_CHAN_0_MASK | BUDDY_CHAN_7_MASK;
	general_settings.channel_mask = BUDDY_CHAN_2_MASK;
	general_settings.resolution = RESOLUTION_CTRL_HIGH;
	//general_settings.resolution = RESOLUTION_CTRL_LOW;

	timing_settings.period = (uint32_t) FREQUENCY_TO_NSEC(sample_rate);
	timing_settings.averaging = 1;

	runtime_settings.adc_ref = RUNTIME_ADC_REF_VDD;

	printf("timing_settings.period = %d (0x%x)\n", timing_settings.period, timing_settings.period);

	if 	(buddy_configure(handle, &general_settings, &runtime_settings, &timing_settings) != BUDDY_ERROR_CODE_OK) {
		printf("test_seq_dac: could not buddy_init\n");
		return -1;
	}
	short_sleep(100);

	recv_packets = 0;
	do {
		if (!active) {
			return 0;
		}

		err_code = buddy_read_adc(handle, &packet, streaming);
		if (err_code == BUDDY_ERROR_CODE_OK) {
			printf("test_seq_adc: received packet %d\n", recv_packets);

			for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
				if (general_settings.channel_mask & (1 << i)) {
					printf("packet.channels[%d] = %d\r\n", i, packet.channels[i]);
				}
			}

			recv_packets++;
		} else if (err_code == BUDDY_ERROR_CODE_GENERAL) {
			printf("test_seq_adc: buddy_send_adc call failed\n");
			return BUDDY_ERROR_CODE_GENERAL;
		}
	} while (recv_packets < 500);

	return 0;
}

int main(int argc, char *argv[])
{
	buddy_hid_info_t hid_info;
	firmware_info_t fw_info;
	hid_device *hid_handle;

	int optind;
	int8_t err_code = 0;
	struct timeb time_start, time_end;
	int time_diff;
	bool stream_mode = false;
	bool hold_mode = false;
	enum { DAQ_MODE, ADC_MODE, PWM_DUTY_MODE, PWM_FREQ_MODE } mode = DAQ_MODE;

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
			case 'p': mode = PWM_DUTY_MODE; break;
			case 't': mode = PWM_FREQ_MODE; break;
			case 's': stream_mode = true; break;
			case 'l': hold_mode = true; break;
			case 'h': 
			default:
				fprintf(stdout, "Usage: %s [-daptls]\n", argv[0]);
				fprintf(stdout, "\nTest the DAC, ADC, and PWM function of the Buddy DAQ device\n\n");
				fprintf(stdout, "optional arguments:\n");
				fprintf(stdout, " -h,        show this help message and exit\n");
				fprintf(stdout, " -d,        enable DAC(data to analog) mode\n");
				fprintf(stdout, " -a,        enable ADC(analog to digital) mode\n");
				fprintf(stdout, " -p,        enable PWM(pulse width modulation) duty cycle mode\n");
				fprintf(stdout, " -t,        enable PWM(pulse width modulation) frequency mode\n");
				fprintf(stdout, " -s,        enable streaming for higher throughput\n");
				fprintf(stdout, " -l,        hold the last mode and value on exit\n");
				exit(EXIT_FAILURE);
        } 
    }

	// open Buddy HID device and retrieve the USB device and firmware info
	hid_handle = buddy_init(&hid_info, &fw_info);

	if (!hid_handle) {
		printf("main: could not open Buddy HID device\n");
		return -1;
	}

	display_usb_info(&hid_info);
	display_fw_info(&fw_info);

    // check if revision is acceptable
    if ((fw_info.fw_rev_major != 0) || (fw_info.fw_rev_minor != 4)) {
        printf("main: firmware major/minor version is not valid, exiting.\n");
        buddy_cleanup(hid_handle, &hid_info, false);
        return -1;
    }

	ftime(&time_start);
	if (mode == DAQ_MODE) {
		printf("main: testing with mode = DAQ_MODE\n");
		err_code = test_seq_dac(hid_handle, &fw_info, BUDDY_TEST_DAC_FREQ, stream_mode);
	} else if (mode == ADC_MODE) {
		printf("main: testing with mode = ADC_MODE\n");
		err_code = test_seq_adc(hid_handle, &fw_info, BUDDY_TEST_ADC_FREQ, stream_mode);
	} else if (mode == PWM_DUTY_MODE) {
		printf("main: testing with mode = PWM_DUTY_MODE\n");
		err_code = test_seq_pwm_duty(hid_handle, &fw_info, BUDDY_TEST_PWM_FREQ, stream_mode);
	} else if (mode == PWM_FREQ_MODE) {
		printf("main: testing with mode = PWM_FREQ_MODE\n");
		err_code = test_seq_pwm_freq(hid_handle, &fw_info, BUDDY_TEST_PWM_FREQ, stream_mode);
	}

	ftime(&time_end);
	time_diff = (int)(1000.0 * (time_end.time - time_start.time) + (time_end.millitm - time_start.millitm));
	printf("Test took (%f) seconds  to run\n", (float) (time_diff / 1000.0));
	
	short_sleep(1000);

	if (err_code == 0) {
		printf("hold_mode = %d\n", hold_mode);

		if (hold_mode) {
			buddy_cleanup(hid_handle, &hid_info, false);
		} else {
			buddy_cleanup(hid_handle, &hid_info, true);
		}
	} else {
		printf("main: could not complete requested buddy action\n");
	}
	
	return 0;
}