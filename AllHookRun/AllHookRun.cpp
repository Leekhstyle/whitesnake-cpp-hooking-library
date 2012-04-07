#include <windows.h>

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass=TEXT("AllHookRun");
char str[128];

#define SCREEN_X 100
#define SCREEN_Y 100
#define SCREEN_WIDTH 300
#define SCREEN_HEIGHT 70

//-------------------------------------------------------
// Memory Map 을 이용한 메모리 공유.
//-------------------------------------------------------
struct HOOKVAR
{
	HINSTANCE	g_hDllInstance;
	HWND		g_hCallWnd;
	HHOOK		g_hkAll;
};
HOOKVAR *pHKVAR = NULL;
TCHAR szName[]=TEXT("AllHookDll");
void ReadFileMap();
//-------------------------------------------------------

//-------------------------------------------------------
// AllHookDll 임포트.
//-------------------------------------------------------

// 암시적 임포트.(프로그램 시작시 임포트된다. lib파일이 필요하다.)
//extern "C" __declspec( dllimport ) void InstallHook(HWND hWnd);
//extern "C" __declspec( dllimport ) void UnInstallHook();

// 명시적 임포트.(원할때 임포트 할 수 있다. lib파일이 필요없다.)
HINSTANCE hinstDll;				// HOOK DLL Instance
void (*HookInstall) (HWND);		// HOOK Install Function()
void (*HookUnInstall) ();		// HOOK UnInstall Function()

bool MyDllImport();				// Dll Import
void MyDllFree();				// Dll Free
void MyHookStart();				// HOOK Start
void MyHookEnd();				// HOOK END
//-------------------------------------------------------

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance
	  ,LPSTR lpszCmdParam,int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst=hInstance;
	
	WndClass.cbClsExtra=0;
	WndClass.cbWndExtra=0;
	WndClass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor=LoadCursor(NULL,IDC_ARROW);
	WndClass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	WndClass.hInstance=hInstance;
	WndClass.lpfnWndProc=(WNDPROC)WndProc;
	WndClass.lpszClassName=lpszClass;
	WndClass.lpszMenuName=NULL;
	WndClass.style=CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd=CreateWindow(lpszClass,lpszClass,WS_OVERLAPPEDWINDOW,
		SCREEN_X,SCREEN_Y,SCREEN_WIDTH,SCREEN_HEIGHT,
		NULL,(HMENU)NULL,hInstance,NULL);
	ShowWindow(hWnd,nCmdShow);
	hWndMain=hWnd;
	
	while(GetMessage(&Message,0,0,0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch(iMessage)
	{
		case WM_CREATE:
			if(MyDllImport()) wsprintf(str, "Dll Import success..");
			else wsprintf(str, "Dll Import failed..");
			return 0;
		case WM_PAINT:
			hdc=BeginPaint(hWnd, &ps);
			TextOut(hdc, 10, 10, str, lstrlen(str));
			EndPaint(hWnd, &ps);
			return 0;
		case WM_KEYDOWN:
			switch(wParam)
			{
			case VK_F1:
				MyHookStart();
				break;

			case VK_F2:
				MyHookEnd();
				break;

			case VK_ESCAPE:
				DestroyWindow(hWnd);
				break;
			}
			return 0;
		case WM_DESTROY:
			MyDllFree();
			PostQuitMessage(0);
			return 0;
	}
	return(DefWindowProc(hWnd,iMessage,wParam,lParam));
}

bool MyDllImport()
{
	hinstDll = LoadLibrary( "AllHookDll.dll" );  
	if( hinstDll == NULL )
	{  
		MessageBox(hWndMain,"AllHookDll.dll", "Dll Import Error", MB_OK);
		return false;
	}

	HookInstall = ( void (*)(HWND) )GetProcAddress( hinstDll, "InstallHook" );
	if( HookInstall == NULL )
	{
		MessageBox(hWndMain,"Could not find InstallHook() Function.", "AllHookDll", NULL);
		FreeLibrary( hinstDll ); 
		return false; 
	}

	HookUnInstall = ( void (*)() )GetProcAddress( hinstDll, "UnInstallHook" );
	if( HookUnInstall == NULL )
	{
		MessageBox(hWndMain,"Could not find UnInstallHook() function.", "AllHookDll", NULL);
		FreeLibrary( hinstDll ); 
		return false; 
	}

	return true;
}

void MyDllFree()
{
	FreeLibrary( hinstDll ); 
}

void MyHookStart()
{
	if(MessageBox(NULL, "Hooking Start?", "AllHook", MB_YESNO) == IDYES)
	{
		HookInstall(hWndMain);
		wsprintf(str, "Hooking Start..");
		InvalidateRect(hWndMain, NULL, TRUE);
	}
}

void MyHookEnd()
{
	if(MessageBox(NULL, "Hooking End?", "AllHook", MB_YESNO) == IDYES)
	{
		HookUnInstall();
		wsprintf(str, "Hooking End..");
		InvalidateRect(hWndMain, NULL, TRUE);
	}
}

void ReadFileMap()
{
   HANDLE hMapFile;

   hMapFile = OpenFileMapping(
                   FILE_MAP_ALL_ACCESS,   // read/write access
                   FALSE,                 // do not inherit the name
                   szName);               // name of mapping object 
 
   if (hMapFile == NULL) 
   { 
      //printf("Could not open file mapping object (%d).\n", GetLastError());
	  MessageBox(NULL, TEXT("Could not open file mapping object"), TEXT("Memory Map"), MB_OK);
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
	  MessageBox(NULL, TEXT("Could not map view of file"), TEXT("Memory Map"), MB_OK);
      return;
   }

   UnmapViewOfFile(pHKVAR);

   CloseHandle(hMapFile);
}