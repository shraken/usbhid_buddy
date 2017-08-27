// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <hidapi.h>
#include <usbhid_buddy.h>
#include <utility.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		#if defined(DEBUG_FILE)
		//debug_init("C:\\Users\\shraken\\buddy_labview.log");
		//debugf("DllMain Buddy entered\n");
		#endif
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		//debugf("DllMain Buddy exited\n");
		//debug_cleanup();
		break;
	}
	return TRUE;
}

