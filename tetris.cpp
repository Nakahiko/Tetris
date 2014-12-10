#pragma comment (lib, "User32.lib")
#pragma comment (lib, "Gdi32.lib")
#include <windows.h>


HINSTANCE hinstance;
HWND hMainWindow;

HDC hMemDC, hBlockDC;
HBITMAP hMemPrev,hBlockPrev;

int board[12][25];

typedef struct _TAG_POSITION{
	int x;
	int y;
}POSITION;

typedef struct _TAG_BLOCK{
	int rotate;
	POSITION p[3];
}BLOCK;

BLOCK block[8]={
    {1, {{0,  0},{0, 0}, {0 ,0}}},  // null
    {2, {{0, -1},{0, 1}, {0 ,2}}},  // tetris
    {4, {{0, -1},{0, 1}, {1 ,1}}},  // L1
    {4, {{0, -1},{0, 1}, {-1,1}}},  // L2
    {2, {{0, -1},{1, 0}, {1 ,1}}},  // key1
    {2, {{0, -1},{-1,0}, {-1,1}}},  // key2
    {1, {{0,  1},{1, 0}, {1 ,1}}},  // square
    {4, {{0, -1},{1, 0}, {-1 ,0}}}  // T
};

typedef struct _TAG_STATUS{
	int x;
	int y;
	int type;
	int rotate;
}STATUS;

STATUS current;

int random(int max){
	return (int)(rand() / (RAND_MAX + 1.0) * max );
} 

bool putBlock(STATUS s,bool action = false){
	if(board[s.x][s.y] != 0){
		return false;
	}
	if(action){
		board[s.x][s.y] = s.type;
	}
	for(int i = 0; i < 3; i++){
		int dx = block[s.type].p[i].x;
		int dy = block[s.type].p[i].y;
		int r = s.rotate % block[s.type].rotate;
		for(int j = 0; j < r; j++){
			int nx = dx,ny = dy;
			dx = ny,dy = -nx;
		}
		if(board[s.x + dx][s.y + dy]!= 0){
			return false;
		}
		if(action){
			board[s.x + dx][s.y + dy]= s.type;
		}
	}
	if(!action){
		putBlock(s, true);
	}
	return true;
}

bool deleteBlock(STATUS s){
	board[s.x][s.y] = 0;
	for(int i = 0; i < 3; i++){
		int dx = block[s.type].p[i].x;
		int dy = block[s.type].p[i].y;
		int r = s.rotate % block[s.type].rotate;
		for(int j = 0; j < r; j++){
			int nx = dx,ny = dy;
			dx = ny,dy = -nx;
		}
		board[s.x + dx][s.y + dy]= 0;
	}
	return true;
}


void showBoard(){
	for(int x = 1; x <= 10; x++){
		for(int  y= 1; y <= 20; y++){
			BitBlt(hMemDC, (x -1 ) * 24, (20 - y) * 24, 24, 24, hBlockDC, 0, board[x][y] * 24, SRCCOPY);
		}
	}
}

bool processIuput(){
	bool ret = false;
	STATUS n = current;
	if(GetAsyncKeyState(VK_LEFT)){
		n.x--;
	}else if(GetAsyncKeyState(VK_RIGHT)){
		n.x++;
	}else if(GetAsyncKeyState(VK_UP)){
		n.rotate++;
	}else if(GetAsyncKeyState(VK_DOWN)){
		ret = true ;
	}
	if(n.x != current.x || n.y != current.y || n.rotate != current.rotate){
		deleteBlock(current);
		if(putBlock(n)){
			current = n;
		}else{
			putBlock(current);
		}
	}
	return ret;
}

void gameOver(){
	KillTimer(hMainWindow,100);
	for(int x = 0; x < 12; x++){
		for(int  y= 0; y < 25; y++){
			if(board[x][y]!=0){
				board[x][y] = 1;
			}
		}
	}
	InvalidateRect(hMainWindow, NULL, false);
}

void deleteLine(){
	for(int y = 1; y < 23; y++){
		bool flag = true;
		for(int x = 1;x <= 10; x++){
			if(board[x][y]==0){
				flag = false;
			}
		}
		if(flag){
			for(int j = y; j < 23; j++){
				for(int i = 1; i <= 10; i++){
					board[i][j] = board[i][j+1];
				}
			}
			y--;
		}
	}
}

void blockDown(){
	deleteBlock(current);
	current.y--;
	if(!putBlock(current)){
		current.y++;
		putBlock(current);
		
		deleteLine();
		
		current.x = 5;
		current.y = 21;
		current.type = random(7)+1;
		current.rotate = random(4);
		if(!putBlock(current)){
			gameOver();
		}
	}
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch(uMsg){
		case WM_CREATE :{
			
			for(int x = 0; x < 12; x++){
				for(int  y= 0; y < 25; y++){
					if( x == 0 || x == 11 || y == 0){
						board[x][y] = 1;
					}else{
						board[x][y] = 0;
					}
				}
			}
			
			//debug/*
			
			current.x = 5;
			current.y = 15;
			current.type = random(7)+1;
			current.rotate = 3;
			putBlock(current);
			
			
			HDC hdc = GetDC(hwnd);
			
			hMemDC = CreateCompatibleDC(hdc);
			HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 24 * 10, 24 * 20);
			hMemPrev = (HBITMAP)SelectObject(hMemDC, hBitmap);
			
			hBlockDC = CreateCompatibleDC(hdc);
			hBitmap = (HBITMAP)LoadImage(NULL, TEXT("block.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

			hBlockPrev = (HBITMAP)SelectObject(hBlockDC, hBitmap);
			
			
			// debug
			//BitBlt(hMemDC, 0, 0, 24, 24, hBlockDC, 0, 0, SRCCOPY);
			
			ReleaseDC(hwnd,hdc);
			break;
		}
		case WM_TIMER: {
			static int w = 0;
			if(w % 2 == 0){
				if(processIuput()){
					w = 0;
				}
			}
			if(w % 5 == 0){
				blockDown();
			}
			w++;
			InvalidateRect(hwnd, NULL, false);
			break;
		}
		case WM_PAINT:{
			showBoard();
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			BitBlt(hdc, 0, 0, 24 * 10, 24 * 20, hMemDC, 0, 0, SRCCOPY);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_DESTROY :{
			HBITMAP hBitmap = (HBITMAP)SelectObject(hMemDC, hMemPrev);
			DeleteObject(hBitmap);
			DeleteObject(hMemDC);
			
			hBitmap = (HBITMAP)SelectObject(hBlockDC, hBlockPrev);
			DeleteObject(hBitmap);
			DeleteObject(hBlockDC);
			
			PostQuitMessage(0);
			break;
		}
		default:
			break;
	}
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

int WinMain(HINSTANCE hInst,HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow){
	hinstance = hInst;
	WNDCLASSEX wc;                                    //登録用シンボルを宣言(シンボル名は何でもいい)
    static LPSTR pClassName = "tetrisPrograming";             //クラス登録に使う名前.同じクラスで同じ名前は使えない

    wc.cbSize        = sizeof(WNDCLASSEX);            //クラスのサイズ.
    wc.style         = CS_HREDRAW | CS_VREDRAW;       //ウィンドウスタイル
    wc.lpfnWndProc   = (WNDPROC)WndProc;          //プロシージャ関数呼び出し(メイン処理)
    wc.cbClsExtra    = 0;                             //.複数用途で使用するときクラスで微妙な設定をできるように確保するメモリ
    wc.cbWndExtra    = 0;                             //cbClsExtraのウィンドウバージョン
    wc.hInstance     = hInst;                     //プログラムのインスタンス
    wc.hIcon         = NULL;  //リソースファイルで登録したアイコンの呼び出し
    wc.hCursor       = LoadCursor(NULL,IDC_ARROW);    //ウィンドウで使うカーソルの呼び出し
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);      //背景色(この場合は白)
    wc.lpszMenuName  = NULL;                      //リソースファイルで登録したウィンドウメニューの名前
    wc.lpszClassName = pClassName;                    //クラスに登録する名前.重複不可
    wc.hIconSm       = NULL;
    if (!RegisterClassEx(&wc)) return FALSE;          //ここで登録.万一重複していればここでエラー
    //クラス登録終わり
    
    RECT r;
    r.left = r.top = 0;
    r.right = 24 * 10;
    r.bottom = 24 * 20;
    AdjustWindowRectEx(&r,WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION,false,0);
    
    hMainWindow = CreateWindow(pClassName,"tetorisPrograming",WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
    		CW_USEDEFAULT,CW_USEDEFAULT, r.right - r.left , r.bottom - r.top ,NULL,NULL,hInst,NULL);
    ShowWindow(hMainWindow,SW_SHOW);
    
	SetTimer(hMainWindow, 100, 1000 / 30, NULL);
    MSG msg;
    while(GetMessage(&msg,NULL,0,0)){
    	TranslateMessage(&msg);
    	DispatchMessage(&msg);
    }
	KillTimer(hMainWindow, 100);
    
	return 0;
}
