#include <windows.h>
#include <stdio.h>
#include <conio.h>

//-------------------------------------------------------
// PAGE FILE을 이용한 데이터 공유. // 실패..원인불가..;;
//-------------------------------------------------------
//#pragma data_seg(".AllHookDll")
//
//HINSTANCE	g_hDllInstance = NULL;		// DLL Instance
//HWND		g_hCallWnd = NULL;			// AllHookRun HANDLE
//HHOOK		g_hkAll = NULL;				// HOOK HANDLE
//
//#pragma data_seg()
//#pragma comment(linker, "/SECTION:.AllHookDll, RWS")
////#pragma code_seg -> 코드 공유도 있음.
//-------------------------------------------------------

//-------------------------------------------------------
// 전역변수.
//-------------------------------------------------------
struct HOOKVAR
{
	HINSTANCE	g_hDllInstance;
	HWND		g_hCallWnd;
	HHOOK		g_hkAll;
};
HOOKVAR *pHKVAR;

//-------------------------------------------------------
// 함수.
//-------------------------------------------------------
void CreateFileMap();
void CloseFileMap();
extern "C" __declspec( dllexport ) void InstallHook(HWND hWnd);
extern "C" __declspec( dllexport ) void UnInstallHook();
LRESULT CALLBACK AllHookProc( int nCode, WPARAM wParam, LPARAM lParam );
//-------------------------------------------------------

//-------------------------------------------------------
// Memory Map 을 이용한 메모리 공유.
//-------------------------------------------------------
TCHAR szName[]=TEXT("AllHookDll");
TCHAR szMsg[]=TEXT("MemoryMap Area of AllHookDll");

HANDLE hMapFile;

void CreateFileMap()
{
   hMapFile = CreateFileMapping(
                 INVALID_HANDLE_VALUE,    // use paging file
                 NULL,                    // default security 
                 PAGE_READWRITE,          // read/write access
                 0,                       // max. object size 
                 sizeof(HOOKVAR),                // buffer size  
                 szName);                 // name of mapping object
 
   if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) 
   { 
      //printf("Could not create file mapping object (%d).\n", GetLastError());
	  MessageBox(NULL, TEXT("Could not create file mapping object"), TEXT("AllHookDll"), MB_OK);
      return;
   }
   pHKVAR = (HOOKVAR*) MapViewOfFile(hMapFile,   // handle to mapping object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,                   
                        0,                   
                        0);           
 
   if (pHKVAR == NULL) 
   { 
      //printf("Could not map view of file (%d).\n", GetLastError()); 
	  MessageBox(NULL, TEXT("Could not map view of file"), TEXT("HookDll"), MB_OK);
      return;
   }
}

void CloseFileMap()
{
	UnmapViewOfFile(pHKVAR);
	CloseHandle(hMapFile);
}
//-------------------------------------------------------
//-------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:	// DLL이 처음 로드될 때 실행.
		CreateFileMap();
		pHKVAR->g_hDllInstance = hInst;
		break;

	case DLL_PROCESS_DETACH:	// DLL이 종료될 때 실행.
		if(pHKVAR->g_hkAll) UnInstallHook();
		CloseFileMap();
		break;
	}
	return TRUE;
}

extern "C" __declspec( dllexport ) void InstallHook(HWND hWnd)
{
	pHKVAR->g_hCallWnd = hWnd;
	pHKVAR->g_hkAll = SetWindowsHookEx(WH_GETMESSAGE, AllHookProc, pHKVAR->g_hDllInstance, 0);
}

extern "C" __declspec( dllexport ) void UnInstallHook()
{
	UnhookWindowsHookEx(pHKVAR->g_hkAll);
}

LRESULT CALLBACK AllHookProc( int nCode, WPARAM wParam, LPARAM lParam ) 
{
    if( nCode < 0 ) 
        return CallNextHookEx( pHKVAR->g_hkAll, nCode, wParam, lParam );

	MSG *lpMsg = ((MSG*)lParam);

	UINT sendMsg = lpMsg->message;
	WPARAM sendWparam = lpMsg->wParam;
	LPARAM sendLparam = lpMsg->lParam;

//	char str[256];
//	wsprintf(str, "HWND : %d, MSG : %d, WPARAM : %d, LPARAM : %d", 
//			 pHKVAR->g_hCallWnd, sendMsg, sendWparam, sendLparam);
//	if(sendMsg == WM_RBUTTONDOWN) MessageBox(NULL, str, "Dll", MB_OK);

//	SendMessage(pHKVAR->g_hCallWnd, sendMsg, sendWparam, sendLparam);

	switch(sendMsg)
	{
	case WM_KEYDOWN:
		switch(sendWparam)
		{
		case VK_F12:
			MessageBox(NULL, "Hook success..", "DLL", MB_OK);
			break;
		}
		break;
	}

    return CallNextHookEx( pHKVAR->g_hkAll, nCode, wParam, lParam );
}