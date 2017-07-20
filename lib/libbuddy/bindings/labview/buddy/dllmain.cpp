// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <hidapi.h>
#include <usbhid_buddy.h>
#include <utility.h>

extern "C" int debug_init(char *output_file);
extern "C" void debug_cleanup(void);
extern "C" void debug_write(char *message);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		#if defined(DEBUG_FILE)
		debug_init("C:\\Users\\shraken\\foo.txt");
		debugf("DllMain entered\n");
		#endif
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		debugf("DllMain exited\n");
		debug_cleanup();
		break;
	}
	return TRUE;
}

