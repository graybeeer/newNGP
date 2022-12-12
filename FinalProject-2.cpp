// FinalProject-2.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "GameFrame.h"
#include "FinalProject-2.h"
#include "Server.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
HWND hWndAll;
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
WAITING_ROOM wr;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
#ifdef EDIT
INT_PTR CALLBACK    DialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DialogProc_Save(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DialogProc_BG(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DialogProc_Object(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DialogProc_Img(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DialogProc_SelImg(HWND, UINT, WPARAM, LPARAM);
#endif
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DialogProc_Server(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    SDF;
    GetWorkingDirPath();

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FINALPROJECT2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FINALPROJECT2));

    MSG msg{ };

    //다이얼로그(대기방)
    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_SERVER), msg.hwnd, DialogProc_Server);
    
    // 기본 메시지 루프입니다:
    while (true) {
        if (msg.message == WM_QUIT) {
            TextureManager::GetInst()->Save();
            break;
        }

        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
		else {
			gameFrame.FrameUpdate();
		}
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FINALPROJECT2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);

#ifdef EDIT
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDR_MENU1);
#else
    wcex.lpszMenuName = NULL;
#endif

    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   RECT rc = client;

#ifdef EDIT
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
#else
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
#endif
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      100, 0, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);

   hWndAll = hWnd;
   EnableWindow(hWnd, FALSE);
   clientMinMax = { rc.right - rc.left, rc.bottom - rc.top };
   if (!hWnd) {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   

   gameFrame.Init(hInst, hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
#ifdef EDIT
    case WM_COMMAND:
        switch (wParam) {
        case ID_SAVE:
            if (!gameFrame.IsStageNull()) {
                if (MessageBox(hWnd, L"현재 내용을 저장할까요?", L"경고", MB_YESNO) == IDYES) {
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_SAVE), hWnd, DialogProc_Save);
                }
            }
            break;

        case ID_CTSTAGE:
            if (!gameFrame.IsStageNull()) {
                if (MessageBox(hWnd, L"현재 내용을 저장할까요?", L"경고", MB_YESNO) == IDYES) {
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_SAVE), hWnd, DialogProc_Save);
                }
            }
            DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_BG), hWnd, DialogProc_BG);
            break;

        case ID_STTEST:
            break;

        case ID_ADDOBJ:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_OBJECT), hWnd, DialogProc_Object);
            break;

        case ID_ADDBG:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_BG), hWnd, DialogProc_BG);
            break;

        case ID_ADDIMG:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_IMG), hWnd, DialogProc_Img);
            break;

        case ID_ADDSPRITE:
            break;
        }
        break;
#endif
    case WM_GETMINMAXINFO:        // 윈도우 사이즈 변경시 발생하는 이벤트
        ((LPMINMAXINFO)lParam)->ptMinTrackSize.x = clientMinMax.cx;        // 윈도우 최소 사이즈 x
        ((LPMINMAXINFO)lParam)->ptMinTrackSize.y = clientMinMax.cy;        // 윈도우 최소 사이즈 y



        ((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = clientMinMax.cx;        // 윈도우 최대 사이즈 x
        ((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = clientMinMax.cy;        // 윈도우 최대 사이즈 y

        ((LPMINMAXINFO)lParam)->ptMaxSize.x = clientMinMax.cx;            // MAX 버튼 클릭시 최대 사이즈 x
        ((LPMINMAXINFO)lParam)->ptMaxSize.y = clientMinMax.cy;            // MAX 버튼 클릭시 최대 사이즈 y
        ((LPMINMAXINFO)lParam)->ptMaxPosition.x = 100;        // MAX 버튼 클릭시 x위치
        ((LPMINMAXINFO)lParam)->ptMaxPosition.y = 100;        // MAX 버튼 클릭시 y위치
		break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
        gameFrame.WndMessage(hWnd, message, wParam, lParam);
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

#ifdef EDIT
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            break;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            break;
        }
        break;

    }

    return INT_PTR{ };

}

INT_PTR CALLBACK DialogProc_Save(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            {
                wchar_t buffer[MAX_BUFFER];
                GetDlgItemText(hDlg, IDC_EDIT_SAVE, buffer, MAX_BUFFER);

                wstring saveFileName{ buffer };
                gameFrame.SaveStage(saveFileName);
            }
            EndDialog(hDlg, LOWORD(wParam));
            break;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            break;
        }
        break;
    }
    return INT_PTR();
}

INT_PTR CALLBACK DialogProc_BG(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
		case IDC_ADDBT:
		    {
			    wchar_t bgKey[MAX_BUFFER] = { 0, };
			    wchar_t bgFile[MAX_BUFFER] = { 0, };
			    GetDlgItemText(hDlg, IDC_ADDBGKEY, bgKey, MAX_BUFFER);
			    GetDlgItemText(hDlg, IDC_ADDBGFILENAME, bgFile, MAX_BUFFER);
                

			    wstring keyW{ bgKey };
			    wstring fileW{ bgFile };
			    string key{ };
                string file{ };
			    key.assign(keyW.begin(), keyW.end());
                file.assign(fileW.begin(), fileW.end());
                if (!FindFile(file.c_str())) {
                    if (MessageBox(NULL, L"경고", L"파일을 찾을 수 없습니다.", MB_OK)) {
                        EndDialog(hDlg, 0);
                    }
                }

                if (!gameFrame.CreateNewStage(key, fileW)) {
                    EndDialog(hDlg, 0);
                }
		    }
            EndDialog(hDlg, 0);
            break;
        }
        break;
    }
    return INT_PTR();
}

INT_PTR CALLBACK DialogProc_Object(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            {
                wchar_t buffer[MAX_BUFFER];
                GetDlgItemText(hDlg, IDC_OBJTAG, buffer, MAX_BUFFER);
                wstring tagW{ buffer };
                if (tagW == L"Barigate") {
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_SELIMG), hDlg, DialogProc_SelImg);
                }
                else {
                    gameFrame.SetCreateObjTag(tagW);
                }
            }
            EndDialog(hDlg, 0);
            break;

        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;

        default:
            break;
        }
        break;
    }

    return INT_PTR();
}

INT_PTR CALLBACK DialogProc_Img(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_ADDBT:
            {
                wchar_t imgKey[MAX_BUFFER] = { 0, };
                wchar_t imgFile[MAX_BUFFER] = { 0, };
                GetDlgItemText(hDlg, IDC_ADDIMGKEY, imgKey, MAX_BUFFER);
                GetDlgItemText(hDlg, IDC_ADDIMGFILENAME, imgFile, MAX_BUFFER);
                if (lstrlen(imgKey) == 0) {
                
                }
                else if (lstrlen(imgFile) == 0) {

                }

                wstring keyW{ imgKey };
                wstring file{ imgFile };
                string key{ };
                key.assign(keyW.begin(), keyW.end());
                if (!TextureManager::GetInst()->AddNewTextrue(key, file.c_str())) {
                    MessageBox(hDlg, L"", L"", MB_OK);
                }
            }
            EndDialog(hDlg, 0);
            break;

        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;
        }

        break;
    }
    return INT_PTR();
}

INT_PTR CALLBACK    DialogProc_SelImg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            {
                wchar_t buffer[MAX_BUFFER] = { 0, };
                GetDlgItemText(hDlg, IDC_SELIMG, buffer, MAX_BUFFER);

                wstring keyW = buffer;
                string key{ };
                key.assign(keyW.begin(), keyW.end());

                if (key != "" and !TextureManager::GetInst()->FindTexture(key)) {
                    MessageBox(hDlg, L"경고", L"입력하신 이미지는 등록되지 않았습니다.", MB_OK);
                }
                else {
                    gameFrame.m_barigateTag = key;
                    gameFrame.SetCreateObjTag("Barigate");
                }
            }
            EndDialog(hDlg, 0);
            break;

        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;
        }
        break;
    }
    return INT_PTR();
}
#endif
// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        
        }

        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


// 대화상자 프로시저
INT_PTR CALLBACK DialogProc_Server(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char nickbuf[NICKBUFSIZE];
    char ipbuf[60] = "\0";
    char portbuf[21];

    switch (uMsg) {
    case WM_INITDIALOG:
        wr.SetDlgHandle(hDlg);
        SetDlgItemTextA(hDlg, IDC_IPADDRESS, "127.0.0.1");
        SetDlgItemTextA(hDlg, IDC_EDIT_SERVERPORT, "4444");
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_MAKEROOM:
            GetDlgItemTextA(hDlg, IDC_EDITNICKNAME, nickbuf, NICKBUFSIZE); //닉네임 설정
            nickbuf[NICKBUFSIZE - 1] = '\0';
            GetDlgItemTextA(hDlg, IDC_EDIT_SERVERPORT, portbuf, NICKBUFSIZE); //포트번호 설정
            if (nickbuf[0] != '\0' && nickbuf[0] != ' ') {
                if (wr.MAKE_ROOM(portbuf) == 0) {
                    wr.enableConnectGui(FALSE);
                    SetDlgItemTextA(hDlg, IDC_EDITNICKNAME, nickbuf);
                    SetDlgItemTextA(hDlg, IDC_HOSTNAME, nickbuf);
                    SetDlgItemTextA(hDlg, IDC_IPADDRESS, ipbuf);
                    SetDlgItemTextA(hDlg, IDC_EDIT_SERVERPORT, portbuf);
                }
            }
            else if (nickbuf[0] == ' ')
                wr.printErrorEditbox((char*)"사용 불가능한 닉네임입니다. (공백닉네임)");

            return TRUE;
        case IDC_CONNECTROOM:
            GetDlgItemTextA(hDlg, IDC_EDITNICKNAME, nickbuf, NICKBUFSIZE); //닉네임 설정
            nickbuf[NICKBUFSIZE - 1] = '\0';
            GetDlgItemTextA(hDlg, IDC_EDIT_SERVERPORT, portbuf, NICKBUFSIZE); //포트번호 설정
            if (nickbuf[0] != '\0' && nickbuf[0] != ' ') {
                GetDlgItemTextA(hDlg, IDC_IPADDRESS, ipbuf, NICKBUFSIZE);
                int retval = wr.CONNECT_ROOM(ipbuf, nickbuf, portbuf);
                if (retval == 0) {
                    wr.enableConnectGui(FALSE);
                    SetDlgItemTextA(hDlg, IDC_EDITNICKNAME, nickbuf);
                }
                else if (retval == -1)
                    wr.printErrorEditbox((char*)"사용 불가능한 닉네임입니다. (중복닉네임)");
                else if (retval == -2)
                    wr.printErrorEditbox((char*)"서버에 연결할 수 없습니다.");
            }
            else if (nickbuf[0] == ' ') {
                wr.printErrorEditbox((char*)"사용 불가능한 닉네임입니다. (공백닉네임)");
            }
            return TRUE;
        case IDC_READY:
            wr.pressReady();
            return TRUE;
        case IDC_START:
            if (wr.GetIsHost())
                wr.pressStart();
            return TRUE;
        }
        return FALSE;
    case WM_CLOSE:
        EnableWindow(hWndAll, TRUE);
        EndDialog(hDlg, LOWORD(wParam));
        return TRUE;
    case WM_DESTROY:
        EnableWindow(hWndAll, TRUE);
        return TRUE;
    }
    return FALSE;
}