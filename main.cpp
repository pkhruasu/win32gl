
#include <windows.h>
#include <tchar.h>
#include "resource.h"
#include <stdio.h>
#include <string.h>
//#include <gl\GL.h>
//#include <gl\GLU.h>
#include <gl\glut.h>	//already include gl and glu
/*
	LPSTR  = char*     LP = long pointer
*/
#define MAX_BUFFER_LENGTH 65536


const char g_szClassName[] = "myWIndowClass";
HWND chatbox_cntrl;            //handle to text box

static UINT uTimer  = 0;
float theta = 0.0f;
/*
float tDelta = 0.0;
float tLast = 0.0f;

*/
LARGE_INTEGER ticksPerSecond;
LARGE_INTEGER tNew;
LARGE_INTEGER tOld;

BOOL CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{		
		case WM_INITDIALOG:
			{
			// This is where we set up the dialog box, and initialise any default values				
				SetDlgItemText(hwnd, IDC_TEXT, "This is a string");
				SetDlgItemInt(hwnd,IDC_NUMBER,5,FALSE);
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_ADD://add button
					{
						//when user click add button, we get the number of they entered
						BOOL bSuccess;
						int nTimes = GetDlgItemInt(hwnd,IDC_NUMBER,&bSuccess,FALSE);
						if (bSuccess)
						{
							//if success we get string they entered
							//First we need to find out how long it is so that we can allocate some meory

							int len = GetWindowTextLength(GetDlgItem(hwnd,IDC_TEXT));
							if (len > 0)
							{// allocate and put string in buffer
								int i;
								char* buf;
								
								buf = (char*)GlobalAlloc(GPTR,len+1);
								GetDlgItemText(hwnd,IDC_TEXT,buf,len+1);

								//Now we add the string to the list box however many times
								//the user asked us too.
								for(i=0; i < nTimes; i++)
								{
									int index = SendDlgItemMessage(hwnd,IDC_LIST, LB_ADDSTRING,0,(LPARAM)buf);	//add message to the end
																												//automatically
									SendDlgItemMessage(hwnd,IDC_LIST,LB_SETITEMDATA,(WPARAM)index,(LPARAM)nTimes);	//add more information to it
								}
								//free the memory
								GlobalFree((HANDLE)buf);
							}
							else
							{
								MessageBox(hwnd,"You didn't ener anything!","Warning",MB_OK);
							}
						}
						else
						{
							MessageBox(hwnd,"Couldn't translate number!","Warning",MB_OK);
						}
					}
					break;
				case IDC_REMOVE:
					{
						//When user clicks remove button, we first get the number of selected item
						HWND hList = GetDlgItem(hwnd,IDC_LIST);
						int count = SendMessage(hList,LB_GETSELCOUNT,0,0);
						if(count != LB_ERR)
						{
							if(count != 0)
							{
								//allocate room to store the list of selected items
								int i;
								int *buf = (int*)GlobalAlloc(GPTR,sizeof(int)*count);	//allocate array of index
								SendMessage(hList,LB_GETSELITEMS,(WPARAM)count,(LPARAM)buf);	//fill in the array

								//Now we loop through the list and remove each item that was selected
								//backward
								for(i=count; i >=0; i--)
								{
									SendMessage(hList,LB_DELETESTRING,(WPARAM)buf[i],0);
								}
								GlobalFree(buf);
							}
							else
							{
								MessageBox(hwnd,"No item selected","warning",MB_OK);
							}
						}
						else
						{
							MessageBox(hwnd,"Error counting item","Warning",MB_OK);
						}
					}
					break;
				case IDC_CLEAR:
					//clear button
					SendDlgItemMessage(hwnd,IDC_LIST, LB_RESETCONTENT,0,0);
					break;
				case IDC_LIST:
					//list control, when user select content
					switch(HIWORD(wParam))
					{
						case LBN_SELCHANGE:
						{
							//Get the number of item selected
							HWND hList = GetDlgItem(hwnd,IDC_LIST);	//get handle
							int count = SendMessage(hList,LB_GETSELCOUNT,0,0);	//count how many selected
							if(count != LB_ERR)
							{
								if(count==1)
								{
									int index;
									int err = SendMessage(hList,LB_GETSELITEMS,(WPARAM)1,(LPARAM)&index);	//send msg to get select item
									if(err != LB_ERR)
									{
										//get the data we associate with the above, this match with when we add it
										int data = SendMessage(hList, LB_GETITEMDATA,(WPARAM)index,0);
										SetDlgItemInt(hwnd,IDC_SHOWCOUNT,data,FALSE);	//set the "how many times " label
									}
									else
									{
										MessageBox(hwnd,"Error getting selected item","Warning",MB_OK);
									}
								}//if
								else
								{

									SetDlgItemText(hwnd,IDC_SHOWCOUNT,"-");
								}//else
							}
							else
							{
								//no item selected, or more than one
								MessageBox(hwnd,"Error counting selected item","Warning",MB_OK);
								
							}
							break;
						}
					}
					break;				
			}
			break;
			case WM_CLOSE:
				EndDialog(hwnd,0);
			default:
				return FALSE;
	}
	return TRUE;
}
/*
 Callback dialog
*/
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{	
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case IDOK:
				EndDialog(hwnd,IDOK);
				break;
			case IDCANCEL:
				EndDialog(hwnd,IDCANCEL);
				break;
		}
		break;
	default:
			return FALSE;
	}
	return TRUE;
}

void openFile(HWND hwnd)
{
	//http://msdn.microsoft.com/en-us/library/windows/desktop/ms646839(v=vs.85).aspx

    OPENFILENAME ofn;       // common dialog box structure
    char szFile[MAX_BUFFER_LENGTH];       // buffer for file name
    //HWND hwnd = NULL;              // owner window
    HANDLE hf = NULL;              // file handle
 	char readBuffer[MAX_BUFFER_LENGTH] = {0};
	DWORD  dwBytesRead = 0;

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);	//The length, in bytes, of the structure. Use sizeof (OPENFILENAME) for this parameter.
    ofn.hwndOwner = hwnd;			/*	A handle to the window that owns the dialog box. 
										This member can be any valid window handle, or it can be NULL if the dialog box has no owner.
									*/
    ofn.lpstrFile = szFile;			/*	The file name used to initialize the File Name edit control. 
										The first character of this buffer must be NULL if initialization is not necessary. 
										When the GetOpenFileName or GetSaveFileName function returns successfully, 
										this buffer contains the drive designator, path, file name, and extension of the selected file.
									*/
    //
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    //
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);			/*	The size, in characters, of the buffer pointed to by lpstrFile. 
												The buffer must be large enough to store the path and file name string or strings, 
												including the terminating NULL character. 
											*/
    ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0DOC,DOCX\0*.DOC;*.DOCX\0";	/*
														A buffer containing pairs of null-terminated filter strings. 
														The last string in the buffer must be terminated by two NULL characters.
														The first string in each pair is a display string that describes the filter 
														(for example, "Text Files"), 
														and the second string specifies the filter pattern (for example, "*.TXT"). 
														To specify multiple filter patterns for a single display string, 
														use a semicolon to separate the patterns (for example, "*.TXT;*.DOC;*.BAK").
													*/
    ofn.nFilterIndex = 2;							/*	The index of the currently selected filter in the File Types control. 
														The buffer pointed to by lpstrFilter contains pairs of strings that define the filters.
														The first pair of strings has an index value of 1, the second pair 2, and so on
													*/
    ofn.lpstrFileTitle = NULL;					//The file name and extension (without path information) of the selected file. This member can be NULL.
    ofn.nMaxFileTitle = 0;						//The size, in characters, of the buffer pointed to by lpstrFileTitle. This member is ignored if lpstrFileTitle is NULL.
    ofn.lpstrInitialDir = NULL;					//The initial directory. The algorithm for selecting the initial directory varies on different platforms.
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;	/*
															A set of bit flags you can use to initialize the dialog box. 
															When the dialog box returns, it sets these flags to indicate the user's input. 
															This member can be a combination of the following flags.
														*/
 
    // Display the Open dialog box. 
 
    if (GetOpenFileName(&ofn)==TRUE) 
        hf = CreateFile(ofn.lpstrFile,							// name of the write
						GENERIC_READ,							// open for reading
						0,										// do not share
						(LPSECURITY_ATTRIBUTES) NULL,			// default security
						OPEN_EXISTING,							//open existing only
						FILE_ATTRIBUTE_NORMAL,					// normal file	
						(HANDLE) NULL);							// no attr. template
	//do something before closin
	if(hf == INVALID_HANDLE_VALUE)
	{
        //DisplayError(TEXT("CreateFile"));
        _tprintf(TEXT("Terminal failure: unable to open file \"%s\" for read.\n"), ofn.lpstrFile);
        return; 
	}

	if( FALSE == ReadFile(hf, readBuffer,MAX_BUFFER_LENGTH-1, &dwBytesRead,NULL))
	{
		printf("Terminal failure: Unable to read from file.\n");
        return; 
	}

	if( dwBytesRead > 0 && dwBytesRead <= MAX_BUFFER_LENGTH -1)
	{
		readBuffer[dwBytesRead] = '\0';	//NULL terminate character		
		SetWindowText(chatbox_cntrl,readBuffer);
	}

	CloseHandle(hf);
    return;
	//MessageBox(hwnd,"I save file","information",MB_OK);
}
void saveFile(HWND hwnd)
{
	//http://msdn.microsoft.com/en-us/library/windows/desktop/ms646839(v=vs.85).aspx

    OPENFILENAME ofn;       // common dialog box structure
    char szFile[MAX_BUFFER_LENGTH];       // buffer for file name
    HANDLE hf = NULL;              // file handle
	BOOL bErrorFlag = FALSE;	//write flag
 	DWORD dwBytesRead = 0;	
    DWORD dwBytesWritten = 0;

	char* readBuffer;
	

	int len = 0;
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);	//The length, in bytes, of the structure. Use sizeof (OPENFILENAME) for this parameter.
    ofn.hwndOwner = hwnd;			/*	A handle to the window that owns the dialog box. 
										This member can be any valid window handle, or it can be NULL if the dialog box has no owner.
									*/
    ofn.lpstrFile = szFile;			/*	The file name used to initialize the File Name edit control. 
										The first character of this buffer must be NULL if initialization is not necessary. 
										When the GetOpenFileName or GetSaveFileName function returns successfully, 
										this buffer contains the drive designator, path, file name, and extension of the selected file.
									*/
    //
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    //
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);			/*	The size, in characters, of the buffer pointed to by lpstrFile. 
												The buffer must be large enough to store the path and file name string or strings, 
												including the terminating NULL character. 
											*/
    ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0DOC\0*.DOC\0";	/*
														A buffer containing pairs of null-terminated filter strings. 
														The last string in the buffer must be terminated by two NULL characters.
														The first string in each pair is a display string that describes the filter 
														(for example, "Text Files"), 
														and the second string specifies the filter pattern (for example, "*.TXT"). 
														To specify multiple filter patterns for a single display string, 
														use a semicolon to separate the patterns (for example, "*.TXT;*.DOC;*.BAK").
													*/
    ofn.nFilterIndex = 2;							/*	The index of the currently selected filter in the File Types control. 
														The buffer pointed to by lpstrFilter contains pairs of strings that define the filters.
														The first pair of strings has an index value of 1, the second pair 2, and so on
													*/
    ofn.lpstrFileTitle = NULL;					//The file name and extension (without path information) of the selected file. This member can be NULL.
    ofn.nMaxFileTitle = 0;						//The size, in characters, of the buffer pointed to by lpstrFileTitle. This member is ignored if lpstrFileTitle is NULL.
    ofn.lpstrInitialDir = NULL;					//The initial directory. The algorithm for selecting the initial directory varies on different platforms.
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;	/*
															A set of bit flags you can use to initialize the dialog box. 
															When the dialog box returns, it sets these flags to indicate the user's input. 
															This member can be a combination of the following flags.
														*/
 
    // Display the Open dialog box. 
 
    if (GetSaveFileName(&ofn)==TRUE) 
        hf = CreateFile(ofn.lpstrFile, GENERIC_WRITE,
            0, (LPSECURITY_ATTRIBUTES) NULL,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
            (HANDLE) NULL);
/*
	//append file type
	strcpy_s(str,MAX_BUFFER_LENGTH,ofn.lpstrFile);			
	switch(ofn.nFilterIndex)
	{		
		case 2:		//.txt
			strcat_s(str,MAX_BUFFER_LENGTH,".txt");
			break;
		case 3:		//.doc/docx
			strcat_s(str,MAX_BUFFER_LENGTH,".doc");
			break;
		default:
			//do nothing
			break;
	}
	ofn.lpstrFile = (LPSTR)str;	//convert back to LPSTR
*/			
	//get text from edit control and put in buffer

	len = GetWindowTextLength(chatbox_cntrl);	
									
	readBuffer = (char*)GlobalAlloc(GPTR,len+1);
	//GetDlgItemText(hwnd,chatbox_cntrl,buf,len+1);
	GetWindowText(chatbox_cntrl,readBuffer,MAX_BUFFER_LENGTH);

	bErrorFlag = WriteFile(hf,					//write file handle
							readBuffer,			//start of data to write
							len,				//number of bytes to write
							&dwBytesWritten,	//number of bytes written
							NULL);				//no overlapped structure	
	//free the memory
	GlobalFree((HANDLE)readBuffer);
	//close handle
	CloseHandle(hf);
    return;
}

void EnableGL(HWND hwnd, HDC* hDC ,HGLRC* hRC )
{
	PIXELFORMATDESCRIPTOR pfd;
	int iFormat;

	*hDC = GetDC(hwnd);

	ZeroMemory(&pfd,sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	iFormat = ChoosePixelFormat(*hDC, &pfd);
	SetPixelFormat(*hDC,iFormat,&pfd);

	//create and enable the render context (RC)
	*hRC = wglCreateContext(*hDC);
	wglMakeCurrent(*hDC,*hRC);

	//start timer	
	char *argv[] = {"foo","bar"};	//fake argument, first is app name
	int argc = 2;

	//init glut including time
	glutInit(&argc,argv);
		
	//get cpu frequency
	
	QueryPerformanceFrequency((LARGE_INTEGER*)&ticksPerSecond);
	QueryPerformanceCounter(&tOld);
	return;
}

void disableGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL,NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hwnd,hDC);

	//destroy timer
	KillTimer(hwnd,uTimer);
	return;
}

void renderGL(HWND hwnd, HDC hDC, HGLRC hRC)
{	
	float tDelta = 0;
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT );
			
	glPushMatrix();
	glRotatef( theta, 0.0f, 0.0f, 1.0f );
	glBegin( GL_TRIANGLES );
	glColor3f( 1.0f, 0.0f, 0.0f ); glVertex2f( 0.0f, 1.0f );
	glColor3f( 0.0f, 1.0f, 0.0f ); glVertex2f( 0.87f, -0.5f );
	glColor3f( 0.0f, 0.0f, 1.0f ); glVertex2f( -0.87f, -0.5f );
	glEnd();
	glPopMatrix();
			
	SwapBuffers( hDC );

	//float tNew = glutGet(GLUT_ELAPSED_TIME);
	//tDelta =  tNew - tLast;
	//tLast = tNew;

	QueryPerformanceCounter(&tNew);
	tDelta = (tNew.QuadPart-tOld.QuadPart)/(float)ticksPerSecond.QuadPart;
	tOld = tNew;
	theta += tDelta*3;

	if (theta >= 360)
	{
		theta = 0;		
	}

	return;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{	
	case WM_CREATE:
		{            
			/*
			This is how to create control in main dialog
			*/
            chatbox_cntrl = CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                0,-1,240,260,hwnd,(HMENU)IDR_MYMENU,GetModuleHandle(NULL),NULL);
		}
		break;
	case WM_SIZE:
		{
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			
			char str[65536],strRect[10];
			
			strcpy_s(str,50,"height :");
			_snprintf_s(strRect,10,10,"%i",width);		
						
			strcat_s(str,50,strRect);
			_snprintf_s(strRect,10,10,"%i",height);		

			strcat_s(str,50," Width :");
			strcat_s(str,50,strRect);
//			MessageBox(hwnd, str ,"information",MB_OK);
//			ResizeDiaglogProc(
			//HWND textCtrl = GetDlgItem(hwnd,ID
			MoveWindow(chatbox_cntrl,0,0,width,height,TRUE);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case ID_HELP_ABOUT:
				{
					int ret = DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_ABOUT),hwnd,AboutDlgProc);
					if(ret == IDOK)
						MessageBox(hwnd,"Dialog exited with OK","Notice",MB_OK | MB_ICONINFORMATION);
					else if(ret == IDCANCEL)
						MessageBox(hwnd,"Dialog exited with Cancel","Notice", MB_OK | MB_ICONINFORMATION);
					else if(ret == -1)
						MessageBox(hwnd,"Dialog Failed!","Error",MB_OK | MB_ICONEXCLAMATION);					
				}
				break;
			case ID_FILE_OPENFILE:			
				openFile(hwnd);			
				break;
			case ID_FILE_SAVEFILE:
				saveFile(hwnd);
				break;
			case ID_FILE_EXIT:
				PostMessage(hwnd,WM_CLOSE,0,0);
				break;
			case ID_STUFF_GO:
				MessageBox(hwnd,"I go do something.","Do it",MB_OK);
				break;

		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd,msg,wParam,lParam);		//most likely does nothing
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	HDC hDC;
	HGLRC hRC;
	MSG msg;
	BOOL quit = FALSE;
	
	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= 0;
	wc.lpfnWndProc		= WndProc;	
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;

	wc.hIcon			= LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_MYICON));			//icon on taskbar	
	wc.hIconSm			= (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON), IMAGE_ICON, 16, 16, 0);	//icon on top left
	wc.lpszMenuName		= MAKEINTRESOURCE(IDR_MYMENU);	
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);		
	wc.lpszClassName	= g_szClassName;
	
	//register class
	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL,"Window Registration failed!", "Error !",
			MB_ICONEXCLAMATION | MB_OK);
	}

	//create window
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		"PKText",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800,600,
		NULL,NULL,hInstance,NULL);

	//check if creating window fail
	if(hwnd == NULL)
	{
		MessageBox(hwnd,"WIndow creattion failed!","Error !",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwnd,nCmdShow);
	UpdateWindow(hwnd);
		
	//DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DlgProc);
	
	
	EnableGL(hwnd,&hDC,&hRC);
	//
	// do OpenGL stuff here
	//
	
	
	while(!quit)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE) > 0 )
		{
			//handle message
			if(msg.message == WM_QUIT)
			{
				quit = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}else	
		{
			// OpenGL animation code goes here
			renderGL(hwnd,hDC,hRC);			
		}
	}

	while(GetMessage(&msg,NULL,0,0) > 0 )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
    
	//cleanup opengl
	disableGL(hwnd,hDC,hRC);	

    return 0;
}
