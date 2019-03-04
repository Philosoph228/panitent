#ifndef PANITENT_H
#define PANITENT_H

#define IDM_FILE_OPEN   1001
#define IDM_FILE_SAVE   1002
#define IDM_FILE_CLOSE  1003

#define IDM_EDIT_UNDO   1004
#define IDM_EDIT_REDO   1005
#define IDM_EDIT_CLRCANVAS  1006

#define IDM_WINDOW_TOOLS    1007
#define IDM_WINDOW_PALETTE  1008

#define IDM_OPTIONS_SETTINGS    1009

#define IDM_HELP_TOPICS 1010
#define IDM_HELP_ABOUT  1011

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void UnregisterClasses();

#endif // PANITENT_H