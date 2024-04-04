
//#include "dll.h"
#include "Windows.h"
#ifdef _WIN64
#define DLL_NAME "steam_api64.dll"
#else
#define DLL_NAME "steam_api.dll"
#endif

extern "C" __declspec( dllexport )  void *CreateInterface( const char *pName, int *pReturnCode )
{
    //PRINT_DEBUG("steamclient CreateInterface %s\n", pName);
    HMODULE steam_api = LoadLibraryA(DLL_NAME);
    void *(__stdcall* create_interface)(const char*) = (void * (__stdcall *)(const char*))GetProcAddress(steam_api, "SteamInternal_CreateInterface");

    return create_interface(pName);
}
