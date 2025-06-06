#include "precomp.h"

#include "win32/application.h"
#include "win32/window.h"

#include "panitentapp.h"
#include "panitentwindow.h"

#include "option_bar.h"
#include "toolbox.h"
#include "workspacecontainer.h"
#include "glwindow.h"
#include "palette_window.h"
#include "layerswindow.h"
#include "resource.h"
#include <time.h>
#include "document.h"
#include "canvas.h"
#include "propgriddialog.h"
#include "sharing/activitysharingmanager.h"
#include "sharing/activitystubdialog.h"
#include "log_window.h"
#include "rbhashmapviz.h"
#include "new.h"
#include "log.h"
#include "binview/binviewdlg.h"
#include "grimstroke/shapecontext.h"
#include "grimstroke/wushapestrategy.h"

#include "grimstroke/pixelbuffer.h"
#include "grimstroke/polygon.h"
#include "util/assert.h"
#include "win32/util.h"
#include "lua_scripting.h"
#include "history.h"
#include "viewport.h"
#include "aboutbox.h"

#include "verifycheck.h"
#include "playground.h"

void PanitentApp_RegisterCommands(PanitentApp* pPanitentApp);

void PanitentApp_Init(PanitentApp* pPanitentApp)
{
    Application_Init(&pPanitentApp->base);

    AppCmd_Init(&pPanitentApp->m_appCmd);
    PanitentApp_RegisterCommands(pPanitentApp);
    pPanitentApp->pPanitentWindow = PanitentWindow_Create(pPanitentApp);

    pPanitentApp->palette = Palette_Create();
    pPanitentApp->m_pActivitySharingManager = ActivitySharingManager_Create();
    pPanitentApp->m_pWorkspaceContainer = WorkspaceContainer_Create();

    NONCLIENTMETRICS ncm = { 0 };
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    pPanitentApp->m_hFont = CreateFontIndirect(&ncm.lfMessageFont);
    pPanitentApp->m_pShapeContext = ShapeContext_Create();
    ShapeContext_SetStrategy(pPanitentApp->m_pShapeContext, WuShapeStrategy_Create());

    // PlaygroundDlg playgroundDlg;
    // PlaygroundDlg_Init(&playgroundDlg);
    // Dialog_CreateWindow(&playgroundDlg, IDD_PLAYGROUND, NULL, TRUE);
}

PanitentApp* PanitentApp_Create()
{
    PanitentApp* pPanitentApp = (PanitentApp*)malloc(sizeof(PanitentApp));
    if (pPanitentApp)
    {
        memset(pPanitentApp, 0, sizeof(PanitentApp));
        PanitentApp_Init(pPanitentApp);
    }

    return pPanitentApp;
}

int WINAPI VerifyCheckThreadProc(PVOID pParam)
{
    PanitentWindow* pPanitentWindow = PanitentApp_GetWindow(PanitentApp_Instance());

    while (TRUE)
    {
        Sleep(5000);
        if (pPanitentWindow)
        {
            VerifyCheckDlg* pVerifyCheckDlg = malloc(sizeof(VerifyCheckDlg));
            VerifyCheckDlg_Init(pVerifyCheckDlg);
        
            Dialog_CreateWindow(pVerifyCheckDlg, IDD_VERIFYCHECK, Window_GetHWND((Window*)pPanitentWindow), TRUE);
            // Window_Show(&verifyCheckDlg, SW_SHOW);

            free(pVerifyCheckDlg);
        }
    }

    return 0;
}

int PanitentApp_Run(PanitentApp* pPanitentApp)
{
    srand((unsigned int)time(NULL));

    Window_CreateWindow((Window*)pPanitentApp->pPanitentWindow, NULL);

    PanitentApp* pPanitentApp2 = PanitentApp_Instance();
    // HANDLE hVerifyCheckThread = CreateThread(NULL, 0, VerifyCheckThreadProc, NULL, 0, NULL);

    return Application_Run(pPanitentApp);
}

TreeNode* CreateToolboxNode(PanitentApp* pPanitentApp, DockHostWindow* pDockHostWindow)
{
    TreeNode* pNodeToolbox = BinaryTree_AllocEmptyNode();
    DockData* pDockDataToolbox = (DockData*)malloc(sizeof(DockData));
    if (pDockDataToolbox)
    {
        memset(pDockDataToolbox, 0, sizeof(DockData));
        pNodeToolbox->data = (void*)pDockDataToolbox;
        wcscpy_s(pDockDataToolbox->lpszName, MAX_PATH, L"Toolbox");
        ToolboxWindow* pToolboxWindow = ToolboxWindow_Create((Application*)pPanitentApp);
        Window_CreateWindow((Window*)pToolboxWindow, NULL);
        DockData_PinWindow(pDockHostWindow, pDockDataToolbox, (Window*)pToolboxWindow);
    }

    return pNodeToolbox;
}

TreeNode* CreateViewportNode(PanitentApp* pPanitentApp, DockHostWindow* pDockHostWindow)
{
    TreeNode* pNodeViewport = BinaryTree_AllocEmptyNode();
    DockData* pDockDataViewport = (DockData*)malloc(sizeof(DockData));
    if (pDockDataViewport)
    {
        memset(pDockDataViewport, 0, sizeof(DockData));
        pNodeViewport->data = (void*)pDockDataViewport;
        wcscpy_s(pDockDataViewport->lpszName, MAX_PATH, L"WorkspaceContainer");
        WorkspaceContainer* pWorkspaceContainer = WorkspaceContainer_Create((Application*)pPanitentApp);
        HWND hWndWorkspaceContainer = Window_CreateWindow((Window*)pWorkspaceContainer, NULL);
        DockData_PinWindow(pDockHostWindow, pDockDataViewport, (Window*)pWorkspaceContainer);
        pPanitentApp->m_pWorkspaceContainer = pWorkspaceContainer;
    }
    
    return pNodeViewport;
}

TreeNode* CreateGLWindowNode(PanitentApp* pPanitentApp, DockHostWindow* pDockHostWindow)
{
    TreeNode* pNodeGLWindow = BinaryTree_AllocEmptyNode();
    DockData* pDockDataGLWindow = (DockData*)malloc(sizeof(DockData));
    if (pDockDataGLWindow)
    {
        memset(pDockDataGLWindow, 0, sizeof(DockData));
        pNodeGLWindow->data = (void*)pDockDataGLWindow;
        wcscpy_s(pDockDataGLWindow->lpszName, MAX_PATH, L"GLWindow");
        GLWindow* pGLWindow = GLWindow_Create((struct Application*)pPanitentApp);
        HWND hwndGLWindow = Window_CreateWindow((Window*)pGLWindow, NULL);
        DockData_PinWindow(pDockHostWindow, pDockDataGLWindow, (Window*)pGLWindow);
    }

    return pNodeGLWindow;
}

TreeNode* CreatePaletteWindowNode(PanitentApp* pPanitentApp, DockHostWindow* pDockHostWindow)
{
    TreeNode* pNodePalette = BinaryTree_AllocEmptyNode();
    DockData* pDockDataPalette = (DockData*)malloc(sizeof(DockData));
    if (pDockDataPalette)
    {
        memset(pDockDataPalette, 0, sizeof(DockData));
        pNodePalette->data = (void*)pDockDataPalette;
        wcscpy_s(pDockDataPalette->lpszName, MAX_PATH, L"Palette");
        PaletteWindow* pPaletteWindow = PaletteWindow_Create(pPanitentApp->palette);
        HWND hwndPalette = Window_CreateWindow((Window*)pPaletteWindow, NULL);
        DockData_PinWindow(pDockHostWindow, pDockDataPalette, (Window*)pPaletteWindow);
    }
    
    return pNodePalette;
}

TreeNode* CreateLayersWindowNode(PanitentApp* pPanitentApp, DockHostWindow* pDockHostWindow)
{
    TreeNode* pNodeLayers = BinaryTree_AllocEmptyNode();
    DockData* pDockDataLayers = (DockData*)malloc(sizeof(DockData));
    if (pDockDataLayers)
    {
        memset(pDockDataLayers, 0, sizeof(DockData));
        pNodeLayers->data = (void*)pDockDataLayers;
        wcscpy_s(pDockDataLayers->lpszName, MAX_PATH, L"Layers");
        LayersWindow* pLayersWindow = LayersWindow_Create((Application*)pPanitentApp);
        HWND hwndLayers = Window_CreateWindow((Window*)pLayersWindow, NULL);
        DockData_PinWindow(pDockHostWindow, pDockDataLayers, (Window*)pLayersWindow);
    }

    return pNodeLayers;
}

TreeNode* CreateOptionBarNode(PanitentApp* pPanitentApp, DockHostWindow* pDockHostWindow)
{
    TreeNode* pNodeOptionBar = BinaryTree_AllocEmptyNode();
    DockData* pDockDataOptionBar = (DockData*)malloc(sizeof(DockData));
    if (pDockDataOptionBar)
    {
        memset(pDockDataOptionBar, 0, sizeof(DockData));
        pNodeOptionBar->data = (void*)pDockDataOptionBar;
        _tcscpy_s(pDockDataOptionBar->lpszName, MAX_PATH, _T("Option Bar"));
        OptionBarWindow* pOptionBarWindow = OptionBarWindow_Create((Application*)pPanitentApp);
        HWND hwndLayers = Window_CreateWindow((Window*)pOptionBarWindow, NULL);
        DockData_PinWindow(pDockHostWindow, pDockDataOptionBar, (Window*)pOptionBarWindow);
    }

    return pNodeOptionBar;
}

TreeNode* CreateSplitNode(PanitentApp* pPanitentApp, LPTSTR pszName, DWORD dwStyle, int iGripPos, TreeNode* pNode1, TreeNode* pNode2)
{
    TreeNode* pNodeY = BinaryTree_AllocEmptyNode();
    DockData* pDockDataY = (DockData*)malloc(sizeof(DockData));

    if (pDockDataY)
    {
        memset(pDockDataY, 0, sizeof(DockData));
        pNodeY->data = (void*)pDockDataY;
        _tcscpy_s(pDockDataY->lpszName, MAX_PATH, pszName);
        pDockDataY->dwStyle = dwStyle;
        pDockDataY->iGripPos = iGripPos;

        pNodeY->node1 = pNode1;
        pNodeY->node2 = pNode2;
    }

    return pNodeY;
}

void PanitentApp_DockHostInit(PanitentApp* pPanitentApp, DockHostWindow* pDockHostWindow, TreeNode* pNodeParent)
{
    DockData* pDockDataParent = DockData_Create(64, DGA_END | DGP_ABSOLUTE | DGD_VERTICAL, FALSE);
    pNodeParent->data = (void*)pDockDataParent;
    wcscpy_s(pDockDataParent->lpszName, MAX_PATH, L"Root");

    /* ======================================================================================== */

    /* Toolbox node */
    TreeNode* pNodeToolbox = CreateToolboxNode(pPanitentApp, pDockHostWindow);

    /* Viewport node */
    TreeNode* pNodeViewport = CreateViewportNode(pPanitentApp, pDockHostWindow);
    
    /* Toolbox/Viewport node */
    TreeNode* pNodeY = CreateSplitNode(pPanitentApp, _T("Toolbox/Viewport"), DGA_START | DGP_ABSOLUTE | DGD_HORIZONTAL, 86, pNodeToolbox, pNodeViewport);

    /* ======================================================================================== */

    /* GLWindow node */
    TreeNode* pNodeGLWindow = CreateGLWindowNode(pPanitentApp, pDockHostWindow);

    /* Create and pin palette window */
    TreeNode* pNodePalette = CreatePaletteWindowNode(pPanitentApp, pDockHostWindow);

    /* Create and pin layers window */
    TreeNode* pNodeLayers = CreateLayersWindowNode(pPanitentApp, pDockHostWindow);

    /* Palette/Layers node */
    TreeNode* pNodeB = CreateSplitNode(pPanitentApp, _T("Palette/Layers"), DGA_START | DGP_ABSOLUTE | DGD_VERTICAL, 256, pNodePalette, pNodeLayers);

    /* right bar node */
    TreeNode* pNodeA = CreateSplitNode(pPanitentApp, _T("Right bar"), DGA_START | DGP_ABSOLUTE | DGD_VERTICAL, 192, pNodeGLWindow, pNodeB);


    /* ======================================================================================== */


    /* Main node */
    TreeNode* pNodeZ = CreateSplitNode(pPanitentApp, _T("Main"), DGA_END | DGP_ABSOLUTE | DGD_HORIZONTAL, 192, pNodeY, pNodeA);

    /* ======================================================================================== */

    TreeNode* pNodeOptionBar = CreateOptionBarNode(pPanitentApp, pDockHostWindow);

    /* OptionBar / Other */
    // TreeNode* pNodeOptionBarOther = CreateSplitNode(pPanitentApp, _T("OptionBar / Other"), DGA_END | DGP_ABSOLUTE | DGD_HORIZONTAL, 192, pNodeZ, pNodeOptionBar);

    pNodeParent->node1 = pNodeZ;
    pNodeParent->node2 = pNodeOptionBar;
}

Palette* PanitentApp_GetPalette(PanitentApp* pPanitentApp)
{
    return pPanitentApp->palette;
}

PanitentApp* PanitentApp_Instance()
{
    static PanitentApp* s_pPanitentApp;
    if (!s_pPanitentApp)
    {
        s_pPanitentApp = PanitentApp_Create();
    }

    return s_pPanitentApp;
}

HFONT PanitentApp_GetUIFont(PanitentApp* pPanitentApp)
{
    return pPanitentApp->m_hFont;
}

PNTSETTINGS* PanitentApp_GetSettings(PanitentApp* pPanitentApp)
{
    &pPanitentApp->m_settings;
}

void PanitentApp_SetActiveViewport(PanitentApp* pPanitentApp, ViewportWindow* pViewportWindow)
{
    pPanitentApp->m_pViewportWindow = pViewportWindow;
}

ViewportWindow* PanitentApp_GetActiveViewport(PanitentApp* pPanitentApp)
{
    return pPanitentApp->m_pViewportWindow;
}

Document* PanitentApp_GetActiveDocument(PanitentApp* pPanitentApp)
{
    ViewportWindow *pViewportWindow = PanitentApp_GetActiveViewport(pPanitentApp);
    if (pViewportWindow == NULL)
        return NULL;

    return ViewportWindow_GetDocument(pViewportWindow);
}

ActivitySharingManager* PanitentApp_GetActivitySharingManager(PanitentApp* pPanitentApp)
{
    return pPanitentApp->m_pActivitySharingManager;
}

ViewportWindow* PanitentApp_GetCurrentViewport(PanitentApp* pPanitentApp)
{
    return WorkspaceContainer_GetCurrentViewport(pPanitentApp->m_pWorkspaceContainer);
}

PanitentWindow* PanitentApp_GetWindow(PanitentApp* pPanitentApp)
{
    return pPanitentApp->pPanitentWindow;
}

WorkspaceContainer* PanitentApp_GetWorkspaceContainer(PanitentApp* pPanitentApp)
{
    return pPanitentApp->m_pWorkspaceContainer;
}

void PanitentApp_SetTool(PanitentApp* pPanitentApp, Tool* pTool)
{
    pPanitentApp->m_pTool = pTool;
}

Tool* PanitentApp_GetTool(PanitentApp* pPanitentApp)
{
    return pPanitentApp->m_pTool;
}

#include "grimstroke/plotter.h"

ShapeContext* PanitentApp_GetShapeContext(PanitentApp* pPanitentApp)
{
    ShapeContext* pShapeContext = pPanitentApp->m_pShapeContext;
    if (!pShapeContext->m_pPlotter)
    {
        Plotter* plotter = (Plotter*)calloc(1, sizeof(Plotter));
        if (plotter == NULL)
        {
            return NULL;
        }

        PlotterData* plotterData = (PlotterData*)calloc(1, sizeof(PlotterData));
        if (plotterData == NULL)
        {
            return NULL;
        }

        plotterData->color = 0xFFFF0000;

        ViewportWindow* pViewportWindow = PanitentApp_GetCurrentViewport(pPanitentApp);

        plotter->userData = plotterData;
        plotter->fn = PixelPlotterCallback;


        pShapeContext->m_pPlotter = plotter;
    }

    return pPanitentApp->m_pShapeContext;
}

void PanitentApp_CmdNewFile(PanitentApp* pPanitentApp)
{
    LogMessage(LOGENTRY_TYPE_INFO, L"MAIN", L"New File menu issued");
    
    if (0)
    {
        PanitentWindow* pPanitentWindow = NULL;
        pPanitentWindow = PanitentApp_GetWindow(pPanitentApp);
        if (pPanitentWindow == NULL)
        {
            LogMessage(LOGENTRY_TYPE_ERROR, L"MAIN", L"Failed to get parent appframe (PanitentWindow* PanitentApp_GetWindow()) during the creation of document setup dialog.");
            return;
        }

        HWND hwndAppFrame = Window_GetHWND((Window*)pPanitentWindow);
        if (hwndAppFrame == NULL || !IsWindow(hwndAppFrame))
        {
            LogMessage(LOGENTRY_TYPE_ERROR, L"MAIN", L"Failed to get appframe (Window_GetHWND((Window*)PanitentWindow*)) HWND during the creation of document setup dialog.");
            return;
        }

        HINSTANCE hInstanceApp = NULL;
        hInstanceApp = (HINSTANCE)GetWindowLongPtr(hwndAppFrame, GWLP_HINSTANCE);
        if (!hInstanceApp)
        {
            LogMessage(LOGENTRY_TYPE_WARNING, L"MAIN", L"AppFrame's GWLP_HINSTANCE is NULL, it's an abnormal behavior. Falling back to the GetModuleHandle(NULL)");
            hInstanceApp = GetModuleHandle(NULL);
        }

        if (!hInstanceApp)
        {
            /* Something is completely broken */
            DebugBreak();
            return;
        }

        BOOL bResult = DocumentSetupDialog_RegisterClass(hInstanceApp);
        if (!bResult)
        {
            LogMessage(LOGENTRY_TYPE_ERROR, L"MAIN", L"Failed to register DocumentSetupDialogV1 class during the creation of document setup dialog.");
            return;
        }

        RECT rcDocSetupWnd = { 0 };
        rcDocSetupWnd.right = 320;
        rcDocSetupWnd.bottom = 240;
        AdjustWindowRect(&rcDocSetupWnd, WS_OVERLAPPEDWINDOW, FALSE);

        int width = RECTWIDTH(&rcDocSetupWnd);
        int height = RECTHEIGHT(&rcDocSetupWnd);

        RECT rcWorkarea = { 0 };

#if 0   /* No monitor info */
        SystemParametersInfo(SPI_GETWORKAREA, sizeof(RECT), &rcWorkarea, 0);
#else
        /* With monitor info */


        do {
            BOOL bFromCursor = TRUE;
            DWORD dwFlags = MONITOR_DEFAULTTONEAREST;
            HMONITOR hMonitor = NULL;

            if (bFromCursor)
            {
                POINT pt;
                GetCursorPos(&pt);
                hMonitor = MonitorFromPoint(pt, dwFlags);
            }
            else {
                if (!IsWindow(hwndAppFrame))
                    break;
                hMonitor = MonitorFromWindow(hwndAppFrame, dwFlags);
            }

            if (!hMonitor) break;

            MONITORINFO mi = { 0 };
            mi.cbSize = sizeof(MONITORINFO);

            if (!GetMonitorInfo(hMonitor, &mi)) break;

            CopyRect(&rcWorkarea, &mi.rcWork);
            break;
        } while (FALSE);
#endif

        /* Center window in workarea */
        int x = rcWorkarea.left + (RECTWIDTH(&rcWorkarea) - width) / 2;
        int y = rcWorkarea.top + (RECTHEIGHT(&rcWorkarea) - height) / 2;

        HWND hwndDocSetup = NULL;
        hwndDocSetup = CreateWindowEx(0, L"{69097ff4-7bc3-4339-9164-2b4b5d88b601}", L"New", WS_OVERLAPPEDWINDOW,
            x, y, width, height, hwndAppFrame, NULL, hInstanceApp, NULL);
        if (hwndDocSetup == NULL || !IsWindow(hwndDocSetup))
        {
            LogMessage(LOGENTRY_TYPE_ERROR, L"MAIN", L"Failed to create DocumentSetupDialogV1 window.");
            return;
        }

        ShowWindow(hwndDocSetup, SW_NORMAL);
        UpdateWindow(hwndDocSetup);
    }
    else {
        NewFileDialog(pPanitentApp->pPanitentWindow);
    }    
}

void PanitentApp_CmdOpenFile(PanitentApp* pPanitentApp)
{
    Document_Open(NULL);
}

void PanitentApp_CmdSaveFile(PanitentApp* pPanitentApp)
{
    Document_Save(PanitentApp_GetActiveDocument(pPanitentApp));
}

void PanitentApp_CmdRunScript(PanitentApp* pPanitentApp)
{
    // Lua_RunScript(L"init.lua");
}

void PanitentApp_CmdBinView(PanitentApp* pPanitentApp)
{
    BinViewDialog* binView = (BinViewDialog*)malloc(sizeof(BinViewDialog));
    BinViewDialog_Init(binView);
    HWND hDlg = Dialog_CreateWindow(binView, IDD_BINVIEW, NULL, FALSE);
    ShowWindow(hDlg, SW_SHOW);
}

void PanitentApp_CmdClearCanvas(PanitentApp* pPanitentApp)
{
    if (pPanitentApp == NULL)
        return;

    Document* pDocument = PanitentApp_GetActiveDocument(pPanitentApp);
    if (pDocument == NULL)
        return;

    Canvas* pCanvas = Document_GetCanvas(pDocument);
    if (pCanvas == NULL)
        return;
    
    Canvas_Clear(pCanvas);
    Window_Invalidate(PanitentApp_GetActiveViewport(pPanitentApp));
}

void PanitentApp_CmdShowLog(PanitentApp* pPanitentApp)
{
    LogWindow* pLogWindow = NULL;
    pLogWindow = (LogWindow*)LogWindow_Create();

    if (pLogWindow) {
        Window_CreateWindow((Window*)pLogWindow, NULL);
    }
    Window_Show((Window*)pLogWindow, SW_SHOW);
}

void PanitentApp_CmdShowRbTreeViz(PanitentApp* pPanitentApp)
{
    RBHashMapVizDialog* pRBHashMapVizDialog = NULL;
    pRBHashMapVizDialog = RBHashMapVizDialog_Create();
    Dialog_CreateWindow(pRBHashMapVizDialog, IDD_RBTREEVIZ, NULL, TRUE);
}

void PanitentApp_CmdClipboardExport(PanitentApp* pPanitentApp) {
    HWND hWndPanitent = NULL;
    hWndPanitent = Window_GetHWND(pPanitentApp->pPanitentWindow);

    if (!OpenClipboard(hWndPanitent)) {
        return;
    }
    EmptyClipboard();

    Document* doc = PanitentApp_GetActiveDocument(pPanitentApp);
    if (!doc)
        return;

    Canvas* canvas = Document_GetCanvas(doc);
    if (!canvas)
        return;

    unsigned char* pData = malloc(canvas->buffer_size);
    memset(pData, 0, canvas->buffer_size);
    if (!pData)
    {
        return;
    }

    ZeroMemory(pData, canvas->buffer_size);
    memcpy(pData, canvas->buffer, canvas->buffer_size);

    for (size_t y = 0; y < (size_t)canvas->height; y++) {
        for (size_t x = 0; x < (size_t)canvas->width; x++) {
            uint8_t* pixel = pData + (x + y * canvas->width) * 4;

            float factor = (float)(pixel[3]) / 255.f;

            pixel[0] *= (uint8_t)factor;
            pixel[1] *= (uint8_t)factor;
            pixel[2] *= (uint8_t)factor;
        }
    }

    HBITMAP hBitmap = CreateBitmap(canvas->width,
        canvas->height, 1, sizeof(uint32_t) * 8, pData);

    SetClipboardData(CF_BITMAP, hBitmap);
    CloseClipboard();
}

void PanitentApp_CmdShowSettings(PanitentApp* pPanitentApp)
{
    /* TODO */
}

void PanitentApp_CmdShowPropertyGridDialog(PanitentApp* pPanitentApp)
{
    PropertyGridDialog* pPropertyGridDialog = PropertyGridDialog_Create(pPanitentApp);
    HWND hDlg = Dialog_CreateWindow(pPropertyGridDialog, IDD_PROPERTYGRIDDLG, NULL, FALSE);
    ShowWindow(hDlg, SW_SHOW);
}

void PanitentApp_CmdShowActivityDialog(PanitentApp* pPanitentApp)
{
    ActivityStubDialog* pActivityStubDialog = ActivityStubDialog_Create(pPanitentApp);
    ActivitySharingManager_AddClient(pPanitentApp->m_pActivitySharingManager, &pActivityStubDialog->m_activitySharingClient);
    HWND hWnd = Dialog_CreateWindow(pActivityStubDialog, IDD_ACTIVITYSTUB, NULL, FALSE);
    ShowWindow(hWnd, SW_SHOW);
}

void PanitentApp_SetActivityStatus(PanitentApp* pPanitentApp, PCWSTR pszStatusMessage)
{
    ActivitySharingManager_SetStatus(pPanitentApp->m_pActivitySharingManager, pszStatusMessage);
}

void PanitentApp_SetActivityStatusF(PanitentApp* pPanitentApp, PCWSTR pszFormat, ...)
{
    va_list argp;
    va_start(argp, pszFormat);

    WCHAR szMessage[256];
    vswprintf_s(szMessage, 256, pszFormat, argp);

    va_end(argp);

    PanitentApp_SetActivityStatus(pPanitentApp, szMessage);
}

void PanitentApp_CmdDisplayPixelBuffer(PanitentApp* pPanitentApp)
{
    PolygonPath* pPolygonPath = PolygonPath_Create();
    PolygonPath_AddPoint(pPolygonPath, 50, 50);
    PolygonPath_AddPoint(pPolygonPath, 100, 50);
    PolygonPath_AddPoint(pPolygonPath, 100, 100);
    PolygonPath_AddPoint(pPolygonPath, 50, 100);
    // PolygonPath_Enclose(pPolygonPath);


    PixelBuffer pixelBuffer;
    PixelBuffer_Init(&pixelBuffer, 128, 128);
    Color bgColor;
    bgColor.r = 0xFF;
    bgColor.g = 0xFF;
    bgColor.b = 0xFF;
    bgColor.a = 0xFF;
    PixelBuffer_Clear(&pixelBuffer, bgColor);

    Color fgColor;
    fgColor.b = 0xFF;
    fgColor.g = 0x00;
    fgColor.r = 0x00;
    fgColor.a = 0xFF;
    Polygon_DrawOnPixelBuffer(pPolygonPath, &pixelBuffer, fgColor);

    
    unsigned char* pData = PixelBuffer_GetData(&pixelBuffer);
    int width = PixelBuffer_GetWidth(&pixelBuffer);
    int height = PixelBuffer_GetHeight(&pixelBuffer);
    BITMAPINFO bmi = { 0 };
    Win32_InitGdiBitmapInfo32Bpp(&bmi, width, height);
    
    void* pBitmapData = NULL;
    HBITMAP hBitmap = CreateDIBSection(GetDC(HWND_DESKTOP), &bmi, DIB_RGB_COLORS, &pBitmapData, NULL, 0);
    ASSERT(hBitmap);

    int rowSize = width * 4;
    for (int y = 0; y < height; ++y)
    {
        unsigned char* srcRow = pData + (height - 1 - y) * rowSize;
        unsigned char* dstRow = (unsigned char*)pBitmapData + y * rowSize;
        memcpy(dstRow, srcRow, rowSize);
    }

    HDC hdcBm = CreateCompatibleDC(GetDC(HWND_DESKTOP));
    HBITMAP hOldBm = (HBITMAP)SelectObject(hdcBm, hBitmap);
    BitBlt(GetDC(HWND_DESKTOP), 0, 0, width, height, hdcBm, 0, 0, SRCCOPY);
    SelectObject(hdcBm, hOldBm);
    DeleteObject(hBitmap);
    DeleteDC(hdcBm);


    FILE* fp = fopen("pixbuftest.bin", "wb");
    if (fp)
    {
        unsigned char* pData = PixelBuffer_GetData(&pixelBuffer);
        int width = PixelBuffer_GetWidth(&pixelBuffer);
        int height = PixelBuffer_GetHeight(&pixelBuffer);
        fwrite(pData, width * height * 4, 1, fp);
        fclose(fp);
    }
}

void PanitentApp_CmdClose(PanitentApp* pPanitentApp)
{
    PostQuitMessage(0);
}

void PanitentApp_CmdUndo(PanitentApp* pPanitentApp)
{
    History_Undo(PanitentApp_GetActiveDocument(pPanitentApp));
}

void PanitentApp_CmdRedo(PanitentApp* pPanitentApp)
{
    History_Redo(PanitentApp_GetActiveDocument(pPanitentApp));
}

void PanitentApp_CmdAbout(PanitentApp* pPanitentApp)
{
    PanitentWindow* pPanitentWindow = PanitentApp_GetWindow(pPanitentApp);
    HWND hMainWnd = Window_GetHWND((Window*)pPanitentWindow);
    AboutBox_Run(hMainWnd);
}

void PanitentApp_RegisterCommands(PanitentApp* pPanitentApp)
{
    AppCmd* pAppCmd = &pPanitentApp->m_appCmd;

    AppCmd_AddCommand(pAppCmd, IDM_FILE_NEW, &PanitentApp_CmdNewFile);
    AppCmd_AddCommand(pAppCmd, IDM_FILE_OPEN, &PanitentApp_CmdOpenFile);
    AppCmd_AddCommand(pAppCmd, IDM_FILE_SAVE, &PanitentApp_CmdSaveFile);
    AppCmd_AddCommand(pAppCmd, IDM_FILE_CLIPBOARD_EXPORT, &PanitentApp_CmdClipboardExport);
    AppCmd_AddCommand(pAppCmd, IDM_FILE_BINVIEW, &PanitentApp_CmdBinView);
    AppCmd_AddCommand(pAppCmd, IDM_FILE_RUN_SCRIPT, &PanitentApp_CmdRunScript);
    AppCmd_AddCommand(pAppCmd, IDM_FILE_CLOSE, &PanitentApp_CmdClose);
    AppCmd_AddCommand(pAppCmd, IDM_EDIT_UNDO, &PanitentApp_CmdUndo);
    AppCmd_AddCommand(pAppCmd, IDM_EDIT_REDO, &PanitentApp_CmdRedo);
    AppCmd_AddCommand(pAppCmd, IDM_EDIT_CLRCANVAS, &PanitentApp_CmdClearCanvas);
    AppCmd_AddCommand(pAppCmd, IDM_WINDOW_ACTIVITY_DIALOG, &PanitentApp_CmdShowActivityDialog);
    AppCmd_AddCommand(pAppCmd, IDM_WINDOW_PROPERTY_GRID, &PanitentApp_CmdShowPropertyGridDialog);
    AppCmd_AddCommand(pAppCmd, IDM_OPTIONS_SETTINGS, &PanitentApp_CmdShowSettings);
    AppCmd_AddCommand(pAppCmd, IDM_HELP_LOG, &PanitentApp_CmdShowLog);
    AppCmd_AddCommand(pAppCmd, IDM_HELP_RBTREEVIZ, &PanitentApp_CmdShowRbTreeViz);
    AppCmd_AddCommand(pAppCmd, IDM_HELP_ABOUT, &PanitentApp_CmdAbout);
    AppCmd_AddCommand(pAppCmd, IDM_HELP_DISPLAYPIXELBUFFER, &PanitentApp_CmdDisplayPixelBuffer);
}
