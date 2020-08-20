#include "precomp.h"

#include <commctrl.h>
#include <assert.h>

#include "swatch.h"

LRESULT CALLBACK SwatchControl_WndProc(HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData)
{
  switch (uMsg)
  {
    case WM_NCHITTEST:
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
      break;
    case WM_LBUTTONDOWN:
      {
        MessageBox(NULL, L"Created!", L"Info", MB_OK);
        uint8_t* custColors = calloc(16, sizeof(uint32_t));

        CHOOSECOLOR cc = {};
        cc.lStructSize = sizeof(CHOOSECOLOR);
        cc.hwndOwner = hWnd;
        cc.lpCustColors = (LPDWORD)custColors;
        cc.rgbResult = RGB(255, 0, 0);
        cc.lpfnHook = NULL;
        BOOL bResult = ChooseColor(&cc);
        assert(bResult);
      }
      return TRUE;
      break;
    case WM_PAINT:
      {
        PAINTSTRUCT ps = {};
        HDC hDC = BeginPaint(hWnd, &ps);
        Rectangle(hDC, 0, 0, 16, 16); 
        EndPaint(hWnd, &ps);
      }
      return TRUE;
      break;
  }

  return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

HWND SwatchControl_Create(uint32_t color, int x, int y, int width,
    int height, HWND hParent)
{
  HWND hWnd = CreateWindowEx(0, WC_STATIC, NULL,
      SS_SIMPLE | WS_VISIBLE | WS_CHILD, x, y, width, height, hParent, NULL,
      GetModuleHandle(NULL), NULL);

  assert(hWnd);
  SetWindowSubclass(hWnd, SwatchControl_WndProc, 0, 0);

  return hWnd;
}