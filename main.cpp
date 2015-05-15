// Remove Building Crash fix for SA-MP 0.3.7 by MyU 
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include "BitStream.h"
#include "RakNetStuff.h"
#include "detours.h"

typedef struct _REMOVE_OBJECT {
	int		  iObjectID;
	float	  fPos[3];
	float	  fRange;
} REMOVE_OBJECT; 

DWORD dw_OrigJMP = NULL;
DWORD dw_Base = NULL;
std::vector<REMOVE_OBJECT> removed_objects;

bool HandleRemove(RPCParameters *rpcParams)
{
	BitStream data ( rpcParams->input, rpcParams->numberOfBitsOfData / 8, false );
	REMOVE_OBJECT removed_obj;
	data.Read ( (char *)&removed_obj, sizeof(REMOVE_OBJECT) );

	size_t size = removed_objects.size();
	for ( size_t it = 0; it != size; it ++ )
		if ( removed_obj.iObjectID == removed_objects[it].iObjectID && removed_obj.fRange == removed_objects[it].fRange && removed_obj.fPos[0] == removed_objects[it].fPos[0] && removed_obj.fPos[1] == removed_objects[it].fPos[1] && removed_obj.fPos[2] == removed_objects[it].fPos[2] )
			return false;
	removed_objects.push_back ( removed_obj );

	return true;
}


void _declspec (naked) RemoveBuilding() // 19b00
{
	static RPCParameters *rpc_params = NULL;

	__asm pushad
		__asm mov rpc_params, esi
		
		HandleRemove(rpc_params);

		__asm test eax,eax
		__asm jmp emerged

	__asm popad

emerged:
	__asm popad
	__asm retn

	__asm jmp [dw_OrigJMP]
}


DWORD WINAPI MainThread (__in LPVOID lpParameter) 
{
	HMODULE hModSAMP;
	while ( true )
	{
		if ( (hModSAMP=GetModuleHandle("samp.dll")) != NULL )
		{
			
			dw_Base = (DWORD)hModSAMP;
			DWORD dw_RemoveBuildingRPC = dw_Base+0x19b00;
			dw_OrigJMP = dw_Base+0x3017A1;
			DetourFunction ( (PBYTE)dw_RemoveBuildingRPC, (PBYTE)RemoveBuilding );	
			break;
		}
		Sleep ( 1000 );
	}
	

	return 1;
}

BOOL APIENTRY DllMain ( HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	UNREFERENCED_PARAMETER ( hinstDLL );
	UNREFERENCED_PARAMETER ( lpvReserved );


	if ( fdwReason == DLL_PROCESS_ATTACH )
	{
		DisableThreadLibraryCalls ( hinstDLL );
		CreateThread ( 0, 0, (LPTHREAD_START_ROUTINE)MainThread, 0, 0, NULL );
	}
	

	return TRUE;
}
