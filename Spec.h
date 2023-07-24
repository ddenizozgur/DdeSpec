#ifndef SPEC_H
#define SPEC_H

// ******************************************************************** //

#define  WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

// ******************************************************************** //

struct _DdeBtn
{
  bool pressed,held,released;
};typedef struct _DdeBtn DdeBtn;

#define DDE_KBD_MAX 256

struct _DdeKbd
{
  DdeBtn key[DDE_KBD_MAX];
  bool key_new[DDE_KBD_MAX],
       key_old[DDE_KBD_MAX];
};typedef struct _DdeKbd DdeKbd;
DdeKbd kbd;

#define DDE_MS_MAX 5

struct _DdeMs
{
  int x,y,wheel;
  DdeBtn lb,rb,mb;
  bool lb_new,rb_new,mb_new,
       lb_old,rb_old,mb_old;
};typedef struct _DdeMs DdeMs;
DdeMs ms;

DdeKbd *Dde_KbdAlias()
{
  return &kbd;
}

DdeMs *Dde_MsAlias()
{
  return &ms;
}

void Dde_KbdInit()
{
  memset(&kbd,0,sizeof(kbd));
}

void Dde_MsInit()
{
  memset(&ms,0,sizeof(ms));
}

void Dde_BtnScan(DdeBtn *btn,bool *new,bool *old,size_t size)
{
  for (size_t i=0;i<size;i++) {
    btn[i].pressed=false;
    btn[i].released=false;
    if (new[i]!=old[i]) {
      if (new[i]) {
        btn[i].pressed=!btn[i].held;
        btn[i].held=true;
      } else {
        btn[i].released=true;
        btn[i].held=false;
      }
    }
    old[i]=new[i];
  }
}

// ******************************************************************** //

struct _DdeWin
{
  int width,height;
  wchar_t *title;
  bool should_close;
  HWND hwnd;
  HDC hdc;
  MSG msg;
};typedef struct _DdeWin DdeWin;
DdeWin win;

char   text4print[4096];
size_t text4print_cs=0;

DdeWin *Dde_WinAlias()
{
  return &win;
}

LRESULT CALLBACK Dde_WinProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
  if (msg==WM_CHAR) {
    WCHAR wchar=(WCHAR)wparam;
    char ch[2];
    int len=WideCharToMultiByte(1254,0,&wchar,1,ch,sizeof(ch),NULL,NULL);
    if (len==1) {
      ch[len]='\0';
      text4print[text4print_cs]=ch[0];
      text4print_cs++;
    }
  }
  if (msg==WM_MOUSEMOVE) {
    ms.x=LOWORD(lparam);
    ms.y=HIWORD(lparam);
  }
  if      (msg==WM_MOUSEWHEEL)  ms.wheel=GET_WHEEL_DELTA_WPARAM(wparam);
  if      (msg==WM_LBUTTONDOWN) ms.lb_new=true;
  else if (msg==WM_LBUTTONUP)   ms.lb_new=false;
  if      (msg==WM_RBUTTONDOWN) ms.rb_new=true;
  else if (msg==WM_RBUTTONUP)   ms.rb_new=false;
  if      (msg==WM_MBUTTONDOWN) ms.mb_new=true;
  else if (msg==WM_MBUTTONUP)   ms.mb_new=false;
  if      (msg==WM_KEYDOWN)     kbd.key_new[wparam]=true;
  else if (msg==WM_KEYUP)       kbd.key_new[wparam]=false;
  if      (msg==WM_SYSKEYDOWN)  kbd.key_new[wparam]=true;
  else if (msg==WM_SYSKEYUP)    kbd.key_new[wparam]=false;

  if (msg==WM_SIZE) {
    win.width=LOWORD(lparam);
    win.height=HIWORD(lparam);
    printf("win.width: %d, win.height: %d\n",
           win.width,win.height);
  }
  if (msg==WM_DESTROY) {
    win.should_close=TRUE;
    PostQuitMessage(0);
  }
  return DefWindowProcW(hwnd,msg,wparam,lparam);
}

#define DDE_WIN_OVERLAPPEDWINDOW WS_OVERLAPPEDWINDOW
#define DDE_WIN_POS_DEFAULT      CW_USEDEFAULT

void Dde_WinInit(int x,int y,int width,int height,wchar_t *title,DWORD param)
{
  //SetThreadLocale(MAKELCID(MAKELANGID(LANG_TURKISH,SUBLANG_DEFAULT),SORT_DEFAULT));

  WNDCLASSW wc;
  memset(&wc,0,sizeof(wc));
  wc.hInstance=GetModuleHandle(NULL);
  wc.lpfnWndProc=Dde_WinProc;
  wc.lpszClassName=L"ClassName";
  wc.hCursor=LoadCursor(NULL,IDC_ARROW);
  wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
  wc.style=CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  RegisterClassW(&wc);

  RECT rect={0,0,width,height};
  AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
  win.width=rect.right-rect.left;
  win.height=rect.bottom-rect.top;

  win.title=title;
  win.hwnd=CreateWindowExW(0,wc.lpszClassName,win.title,param,
                             x,y,win.width,win.height,NULL,NULL
                             ,wc.hInstance,NULL);
  UnregisterClassW(wc.lpszClassName,wc.hInstance);
  win.hdc=GetDC(win.hwnd);
  win.should_close=FALSE;
  memset(&win.msg,0,sizeof(MSG));
  Dde_KbdInit();
  Dde_MsInit();
}

void Dde_WinClose()
{
  ReleaseDC(win.hwnd,win.hdc);
  DestroyWindow(win.hwnd);
}

#define DDE_WIN_SHOW SW_SHOW

void Dde_WinShow(int param)
{
  ShowWindow(win.hwnd,param);
}

void Dde_WinPeekMsg()
{
  while (PeekMessageW(&win.msg,win.hwnd,0,0,PM_REMOVE)>0) {
    TranslateMessage(&win.msg);
    DispatchMessageW(&win.msg);
  }
  Dde_BtnScan(kbd.key,kbd.key_new,kbd.key_old,DDE_KBD_MAX);
  Dde_BtnScan(&ms.lb,&ms.lb_new,&ms.lb_old,1);
  Dde_BtnScan(&ms.rb,&ms.rb_new,&ms.rb_old,1);
  Dde_BtnScan(&ms.mb,&ms.mb_new,&ms.mb_old,1);
}

void Dde_WinSwapBuffers()
{
  SwapBuffers(win.hdc);
}

// ******************************************************************** //

struct _DdeTimer
{
  LARGE_INTEGER freq,prev,last;
  float tick_per_sec,delta,fps;
};typedef struct _DdeTimer DdeTimer;
DdeTimer ft;

DdeTimer *Dde_TimerAlias()
{
  return &ft;
}

void Dde_TimerInit()
{
  QueryPerformanceFrequency(&ft.freq);
  ft.tick_per_sec=(float)ft.freq.QuadPart;
  QueryPerformanceCounter(&ft.prev);
}

void Dde_TimerUpdate()
{
  QueryPerformanceCounter(&ft.last);
  float elapsed_ticks=(float)(ft.last.QuadPart-ft.prev.QuadPart);
  ft.delta=elapsed_ticks/ft.tick_per_sec;
  ft.fps=1.0f/ft.delta;
  ft.prev=ft.last;
}

// ******************************************************************** //

#define DDE_FONT_WIDTH  8
#define DDE_FONT_HEIGHT 8
#define DDE_FONT_MAX    256

uint64_t font_std[DDE_FONT_MAX]={
  0x0000000000000000,0x0000000000000000,
  0x000000FF00000000,0x000000FF00FF0000,//騵
  0x1818181818181818,0x6C6C6C6C6C6C6C6C,//竟
  0x181818F800000000,0x6C6C6CEC0CFC0000,//硭
  0x1818181F00000000,0x6C6C6C6F607F0000,//蕃
  0x000000F818181818,0x000000FC0CEC6C6C,//檀
  0x0000001F18181818,0x0000007F606F6C6C,//椇
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0008000000000000,//
  0x0000000000000000,0x00180018183C3C18,// !
  0x0000000000363636,0x006C6CFE6CFE6C6C,//"#
  0x00187ED07C16FC30,0x0060660C18306606,//$$%
  0x00DC66B61C36361C,0x0000000000181818,//&'
  0x0030180C0C0C1830,0x000C18303030180C,//()
  0x0000187E3C7E1800,0x000018187E181800,//*+
  0x0C18180000000000,0x000000007E000000,//,-
  0x0018180000000000,0x0000060C18306000,//./
  0x003C666E7E76663C,0x007E181818181C18,//01
  0x007E0C183060663C,0x003C66603860663C,//23
  0x0030307E363C3830,0x003C6660603E067E,//45
  0x003C66663E060C38,0x000C0C0C1830607E,//67
  0x003C66663C66663C,0x001C30607C66663C,//89
  0x0018180018180000,0x0C18180018180000,//:;
  0x0030180C060C1830,0x0000007E007E0000,//<=
  0x000C18306030180C,0x001800181830663C,//>?
  0x003C06765676663C,0x006666667E66663C,//@A
  0x003E66663E66663E,0x003C66060606663C,//BC
  0x001E36666666361E,0x007E06063E06067E,//DE
  0x000606063E06067E,0x003C66667606663C,//FG
  0x006666667E666666,0x007E18181818187E,//HI
  0x001C36303030307C,0x0066361E0E1E3666,//JK
  0x007E060606060606,0x00C6C6D6D6FEEEC6,//LM
  0x006666767E6E6666,0x003C66666666663C,//NO
  0x000606063E66663E,0x006C36566666663C,//PQ
  0x006666363E66663E,0x003C66603C06663C,//RS
  0x001818181818187E,0x003C666666666666,//TU
  0x00183C6666666666,0x00C6EEFED6D6C6C6,//VW
  0x0066663C183C6666,0x001818183C666666,//XY
  0x007E060C1830607E,0x003E06060606063E,//Z[
  0x00006030180C0600,0x007C60606060607C,//\]
  0x000000000000663C,0xFFFF000000000000,//^_
  0x000000000030180C,0x007C667C603C0000,//`a
  0x003E6666663E0606,0x003C6606663C0000,//bc
  0x007C6666667C6060,0x003C067E663C0000,//de
  0x000C0C0C3E0C0C38,0x3C607C66667C0000,//fg
  0x00666666663E0606,0x003C1818181C0018,//hi
  0x0E181818181C0018,0x0066361E36660606,//jk
  0x003C18181818181C,0x00C6D6D6FE6C0000,//lm
  0x00666666663E0000,0x003C6666663C0000,//no
  0x06063E66663E0000,0xE0607C66667C0000,//pq
  0x000606066E360000,0x003E603C067C0000,//rs
  0x00380C0C0C3E0C0C,0x007C666666660000,//tu
  0x00183C6666660000,0x006CFED6D6C60000,//vw
  0x00663C183C660000,0x3C607C6666660000,//xy
  0x007E0C18307E0000,0x003018180E181830,//z{
  0x0018181818181818,0x000C18187018180C,//|}
  0x000000000062D68C,0xFFFFFFFFFFFFFFFF,//~
  0x1E30181E3303331E,0x007E333333003300,//�
  0x001E033F331E0038,0x00FC667C603CC37E,//�
  0x007E333E301E0033,0x007E333E301E0007,//�
  0x007E333E301E0C0C,0x3C603E03033E0000,//�
  0x003C067E663CC37E,0x001E033F331E0033,//�
  0x001E033F331E0007,0x001E0C0C0C0E0033,//�
  0x003C1818181C633E,0x001E0C0C0C0E0007,//�
  0x00333F33331E0C33,0x00333F331E000C0C,//�
  0x003F061E063F0038,0x00FE33FE30FE0000,//�
  0x007333337F33367C,0x001E33331E00331E,//�
  0x001E33331E003300,0x001E33331E000700,//�
  0x007E33333300331E,0x007E333333000700,//�
  0x1F303F3333003300,0x001C3E63633E1C63,//�
  0x001E333333330033,0x18187E03037E1818,//�
  0x003F67060F26361C,0x000C3F0C3F1E3333,//�
  0x70337B332F1B1B0F,0x0E1B18187E18D870,//�
  0x007E333E301E0038,0x001E0C0C0C0E001C,//
  0x001E33331E003800,0x007E333333003800,//╯
  0x003333331F001F00,0x00333B3F3733003F,//丰
  0x00007E007C36363C,0x00007E003C66663C,//戍
  0x001E3303060C000C,0x000003033F000000,//貝
  0x000030303F000000,0xF81973C67C1B3363,//物
  0xC0F9F3E6CF1B3363,0x183C3C1818001800,//洎
  0x0000CC663366CC00,0x00003366CC663300,//悖
  0x1144114411441144,0x55AA55AA55AA55AA,//停
  0xEEBBEEBBEEBBEEBB,0x1818181818181818,//眾
  0x1818181F18181818,0x1818181F181F1818,//斯
  0x6C6C6C6F6C6C6C6C,0x6C6C6C7F00000000,//須
  0x1818181F181F0000,0x6C6C6C6F606F6C6C,//號
  0x6C6C6C6C6C6C6C6C,0x6C6C6C6F607F0000,//獄
  0x0000007F606F6C6C,0x0000007F6C6C6C6C,//播
  0x0000001F181F1818,0x1818181F00000000,//噶
  0x000000F818181818,0x000000FF18181818,//擱
  0x181818FF00000000,0x181818F818181818,//藏
  0x000000FF00000000,0x181818FF18181818,//霰
  0x181818F818F81818,0x6C6C6CEC6C6C6C6C,//
  0x000000FC0CEC6C6C,0x6C6C6CEC0CFC0000,//
  0x000000FF00EF6C6C,0x6C6C6CEF00FF0000,//帊
  0x6C6C6CEC0CEC6C6C,0x000000FF00FF0000,//昅
  0x6C6C6CEF00EF6C6C,0x000000FF00FF1818,//恘
  0x000000FF6C6C6C6C,0x181818FF00FF0000,//倳
  0x6C6C6CFF00000000,0x000000FC6C6C6C6C,//眑
  0x000000F818F81818,0x181818F818F80000,//婭
  0x6C6C6CFC00000000,0x6C6C6CEF6C6C6C6C,//笫
  0x181818FF00FF1818,0x0000001F18181818,//崷
  0x181818F800000000,0xFFFFFFFFFFFFFFFF,//窙
  0xFFFFFFFF00000000,0x0F0F0F0F0F0F0F0F,//嗲
  0xF0F0F0F0F0F0F0F0,0x00000000FFFFFFFF,//睧
  0x006E3B133B6E0000,0x03031F331F331E00,//颬
  0x0003030303637F00,0x0036363636367F00,//睼
  0x007F660C180C667F,0x001E3333337E0000,//麧
  0x03063E6666666600,0x00181818183B6E00,//緗
  0x3F0C1E33331E0C3F,0x001C36637F63361C,//鴇
  0x007736366363361C,0x001E33333E180C38,//膹
  0x00007EDBDB7E0000,0x03067EDBDB7E3060,//擨
  0x003C06033F03063C,0x003333333333331E,//闀
  0x00003F003F003F00,0x003F000C0C3F0C0C,//貘
  0x003F00060C180C06,0x003F00180C060C18,//覷
  0x1818181818D8D870,0x0E1B1B1818181818,//鏷
  0x000C0C003F000C0C,0x0000394E00394E00,//禴
  0x000000001C36361C,0x0000001818000000,//矙
  0x0000001800000000,0x383C3637303030F0,//
  0x000000363636361E,0x0000003E061C301E,//
  0x00003C3C3C3C0000,0xFFFFFFFFFFFFFFFF,//�
};

uint64_t font_cyrillic[DDE_FONT_MAX]={
  0x0000000000000000,0x0000000000000000,
  0x000000FF00000000,0x000000FF00FF0000,//ÄÍ
  0x1818181818181818,0x6C6C6C6C6C6C6C6C,//³º
  0x181818F800000000,0x6C6C6CEC0CFC0000,//ÚÉ
  0x1818181F00000000,0x6C6C6C6F607F0000,//¿»
  0x000000F818181818,0x000000FC0CEC6C6C,//ÀÈ
  0x0000001F18181818,0x0000007F606F6C6C,//Ù¼
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0008000000000000,//
  0x0000000000000000,0x00180018183C3C18,// !
  0x0000000000363636,0x006C6CFE6CFE6C6C,//"#
  0x00187ED07C16FC30,0x0060660C18306606,//$$%
  0x00DC66B61C36361C,0x0000000000181818,//&'
  0x0030180C0C0C1830,0x000C18303030180C,//()
  0x0000187E3C7E1800,0x000018187E181800,//*+
  0x0C18180000000000,0x000000007E000000,//,-
  0x0018180000000000,0x0000060C18306000,//./
  0x003C666E7E76663C,0x007E181818181C18,//01
  0x007E0C183060663C,0x003C66603860663C,//23
  0x0030307E363C3830,0x003C6660603E067E,//45
  0x003C66663E060C38,0x000C0C0C1830607E,//67
  0x003C66663C66663C,0x001C30607C66663C,//89
  0x0018180018180000,0x0C18180018180000,//:;
  0x0030180C060C1830,0x0000007E007E0000,//<=
  0x000C18306030180C,0x001800181830663C,//>?
  0x003C06765676663C,0x006666667E66663C,//@A
  0x003E66663E66663E,0x003C66060606663C,//BC
  0x001E36666666361E,0x007E06063E06067E,//DE
  0x000606063E06067E,0x003C66667606663C,//FG
  0x006666667E666666,0x007E18181818187E,//HI
  0x001C36303030307C,0x0066361E0E1E3666,//JK
  0x007E060606060606,0x00C6C6D6D6FEEEC6,//LM
  0x006666767E6E6666,0x003C66666666663C,//NO
  0x000606063E66663E,0x006C36566666663C,//PQ
  0x006666363E66663E,0x003C66603C06663C,//RS
  0x001818181818187E,0x003C666666666666,//TU
  0x00183C6666666666,0x00C6EEFED6D6C6C6,//VW
  0x0066663C183C6666,0x001818183C666666,//XY
  0x007E060C1830607E,0x003E06060606063E,//Z[
  0x00006030180C0600,0x007C60606060607C,//\]
  0x000000000000663C,0xFFFF000000000000,//^_
  0x000000000030180C,0x007C667C603C0000,//`a
  0x003E6666663E0606,0x003C6606663C0000,//bc
  0x007C6666667C6060,0x003C067E663C0000,//de
  0x000C0C0C3E0C0C38,0x3C607C66667C0000,//fg
  0x00666666663E0606,0x003C1818181C0018,//hi
  0x0E181818181C0018,0x0066361E36660606,//jk
  0x003C18181818181C,0x00C6D6D6FE6C0000,//lm
  0x00666666663E0000,0x003C6666663C0000,//no
  0x06063E66663E0000,0xE0607C66667C0000,//pq
  0x000606066E360000,0x003E603C067C0000,//rs
  0x00380C0C0C3E0C0C,0x007C666666660000,//tu
  0x00183C6666660000,0x006CFED6D6C60000,//vw
  0x00663C183C660000,0x3C607C6666660000,//xy
  0x007E0C18307E0000,0x003018180E181830,//z{
  0x0018181818181818,0x000C18187018180C,//|}
  0x000000000062D68C,0xFFFFFFFFFFFFFFFF,//~
  0x1E30181E3303331E,0x007E333333003300,//€
  0x001E033F331E0038,0x00FC667C603CC37E,//‚ƒ
  0x007E333E301E0033,0x007E333E301E0007,//„…
  0x007E333E301E0C0C,0x3C603E03033E0000,//†‡
  0x003C067E663CC37E,0x001E033F331E0033,//ˆ‰
  0x001E033F331E0007,0x001E0C0C0C0E0033,//Š‹
  0x003C1818181C633E,0x001E0C0C0C0E0007,//Œ
  0x00333F33331E0C33,0x00333F331E000C0C,//Ž
  0x003F061E063F0038,0x00FE33FE30FE0000,//‘
  0x007333337F33367C,0x001E33331E00331E,//’“
  0x001E33331E003300,0x001E33331E000700,//”•
  0x007E33333300331E,0x007E333333000700,//–—
  0x1F303F3333003300,0x001C3E63633E1C63,//˜™
  0x001E333333330033,0x18187E03037E1818,//š›
  0x003F67060F26361C,0x000C3F0C3F1E3333,//œ
  0x70337B332F1B1B0F,0x0E1B18187E18D870,//žŸ
  0x000F11110F01111F,0x000101010101111F,// ¡
  0x000F0107010F000A,0x001515150E151515,//¢£
  0x000F10100E10100F,0x0011111315191111,//¤¥
  0x001113151911040A,0x001215141414141E,//¦§
  0x001111111111111F,0x000102040A111111,//¨©
  0x00040E1515150E04,0x001010101E111111,//ª«
  0x001F151515151515,0x000E12120E020203,//¬­
  0x0013151513111111,0x000E11101C10110E,//®¯
  0x0009151517151509,0x001112141E11111E,//°±
  0x000E11110F010618,0x0007090709070000,//²³
  0x00010101111F0000,0x000E011F110E000A,//´µ
  0x0015150E15150000,0x000F100C100F0000,//¶·
  0x0011131519110000,0x0013151911040A00,//¸¹
  0x0009050305090000,0x00121514141E0000,//º»
  0x001111151B110000,0x0011111F11110000,//¼½
  0x00111111111F0000,0x00040404041F0000,//¾¿
  0x0010101E11110000,0x001F151515150000,//ÀÁ
  0x000E120E02030000,0x0013151311110000,//ÂÃ
  0x0007090701010000,0x000E111C110E0000,//ÄÅ
  0x0009151715090000,0x0012141E111E0000,//ÆÇ
  0x0004120912040000,0x0004091209040000,//ÈÉ
  0x001B091200000000,0x000000000009121B,//ÊË
  0x001C141C090D0B09,0x000E110102040004,//ÌÍ
  0x000304040E040418,0x001E02020F020A04,//ÎÏ
  0x0004040000000000,0x0004040404000000,//ÐÑ
  0x000E000A0A0A0000,0x000A000A0A0A0000,//ÒÓ
  0x0001020408170005,0x00001B0E040E1B00,//ÔÕ
  0x0001020408150205,0x00000E040404040E,//Ö×
  0x00001F0A0A0A0A1F,0x0004040404150E04,//ØÙ
  0x00040E1504040404,0x0001090D1F0D0901,//ÚÛ
  0x001012161F161210,0x000111151F151101,//ÜÝ
  0x0001020408110701,0x000000000C0C0000,//Þß
  0x111F11121414141E,0x101F111111111111,//àá
  0x101F151515151515,0x111F1112141E0000,//âã
  0x040E15150E040400,0x101A111111110000,//äå
  0x101F151515150000,0x0000000000000408,//æç
  0x000000000000000A,0x0000000000000D12,//èé
  0x000E011F110E0408,0x06040E11010E0000,//êë
  0x0006080A0A0A000A,0x0000041F0E0E0E04,//ìí
  0x00000A0011000A00,0x00000E1111110E00,//îï
  0x0010101D1A150911,0x001C1019121D0911,//ðñ
  0x001C0419121D0911,0x10101C1B12030203,//òó
  0x00111F111F111F11,0x0000000600180003,//ôõ
  0x041E141015070502,0x07020E021E020E02,//ö÷
  0x0E040E041F040E04,0x1C08160017001600,//øù
  0x1C080E080F080E08,0x00040E041F040E04,//úû
  0x1C08090A0C0A0908,0x0609040A0A04120C,//üý
  0x001414141E15151E,0xFFFFFFFFFFFFFFFF,//þÿ
};

union _DdeRGBAU32
{
  struct {uint8_t r,g,b,a;};
  uint32_t n;
};typedef union _DdeRGBAU32 DdeRGBAU32;

struct _DdeCV
{
  int width,height;
  DdeRGBAU32 color;
  uint32_t *buff,id;
};typedef struct _DdeCV DdeCV;

#define DDE_CV_NEAREST GL_NEAREST
#define DDE_CV_LINEAR  GL_LINEAR

void Dde_CVNew(DdeCV *cv,int width,int height,int param)
{
  cv->width=width;
  cv->height=height;
  cv->color=(DdeRGBAU32){0,0,0,0};
  cv->buff=(uint32_t*)malloc(width*height*sizeof(uint32_t));
  memset(cv->buff,0,width*height*sizeof(uint32_t));

  glGenTextures(1,&cv->id);
  glBindTexture(GL_TEXTURE_2D,cv->id);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,param);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,param);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,
               GL_RGBA,GL_UNSIGNED_BYTE,cv->buff);
}

void Dde_CVDel(DdeCV *cv)
{
  glDeleteTextures(1,&cv->id);
  free(cv->buff);
}

void Dde_CVPlot(DdeCV *cv,int x,int y)
{
  if (0<=x<cv->width && 0<=y<cv->height)
    cv->buff[(size_t)y*cv->width+x]=cv->color.n;
}

void Dde_CVFillRect(DdeCV *cv,int x,int y,int w,int h)
{
  for (int j=0;j<h;j++)
    for (int i=0;i<w;i++)
      Dde_CVPlot(cv,x+i,y+j);
}

void Dde_CVFill(DdeCV *cv)
{
  for (int j=0;j<cv->height;j++)
    for (int i=0;i<cv->width;i++)
      Dde_CVPlot(cv,i,j);
}

void Dde_CVPlotU64(DdeCV *cv,int x,int y,uint64_t val)
{
  for (int j=0;j<DDE_FONT_HEIGHT;j++)
    for (int i=0;i<DDE_FONT_WIDTH;i++)
      if (val>>(j*DDE_FONT_WIDTH+i)&1)
        Dde_CVPlot(cv,x+i,y+j);
}

void Dde_CVPlotInvU64(DdeCV *cv,int x,int y,uint64_t val)
{
  for (int j=0;j<DDE_FONT_HEIGHT;j++)
    for (int i=0;i<DDE_FONT_WIDTH;i++)
      if (!(val>>(j*DDE_FONT_WIDTH+i)&1))
        Dde_CVPlot(cv,x+i,y+j);
}

void Dde_CVPutStdChar(DdeCV *cv,int x,int y,char ch)
{
  Dde_CVPlotU64(cv,x,y,font_std[ch]);
}

void Dde_CVPutInvStdChar(DdeCV *cv,int x,int y,char ch)
{
  Dde_CVPlotInvU64(cv,x,y,font_std[ch]);
}

void Dde_CVPutCyrillicChar(DdeCV *cv,int x,int y,char ch)
{
  Dde_CVPlotU64(cv,x,y,font_cyrillic[ch]);
}

void Dde_CVPutInvCyrillicChar(DdeCV *cv,int x,int y,char ch)
{
  Dde_CVPlotInvU64(cv,x,y,font_cyrillic[ch]);
}

void Dde_CVPutChar(DdeCV *cv,int x,int y,char ch)
{
  Dde_CVPlotU64(cv,x,y,font_std[ch]);
}

void Dde_CVPutInvChar(DdeCV *cv,int x,int y,char ch)
{
  Dde_CVPlotInvU64(cv,x,y,font_std[ch]);
}

#define DDE_TAB_WIDTH 2

void Dde_CVPutStdStr(DdeCV *cv,int x,int y,char *str)
{
  int x0=x;
  while (*str) {
    if (*str=='\n') {
      x=x0;
      y+=DDE_FONT_HEIGHT;
      str++;
    } else if (*str=='\t') {
      x+=DDE_TAB_WIDTH*DDE_FONT_WIDTH;
      str++;
    } else {
      Dde_CVPutStdChar(cv,x,y,*str);
      x+=DDE_FONT_WIDTH;
      str++;
    }
  }
}

void Dde_CVPutInvStdStr(DdeCV *cv,int x,int y,char *str)
{
  int x0=x;
  while (*str) {
    if (*str=='\n') {
      x=x0;
      y+=DDE_FONT_HEIGHT;
      str++;
    } else if (*str=='\t') {
      x+=DDE_TAB_WIDTH*DDE_FONT_WIDTH;
      str++;
    } else {
      Dde_CVPutInvStdChar(cv,x,y,*str);
      x+=DDE_FONT_WIDTH;
      str++;
    }
  }
}

void Dde_CVPutCyrillicStr(DdeCV *cv,int x,int y,char *str)
{
  int x0=x;
  while (*str) {
    if (*str=='\n') {
      x=x0;
      y+=DDE_FONT_HEIGHT;
      str++;
    } else if (*str=='\t') {
      x+=DDE_TAB_WIDTH*DDE_FONT_WIDTH;
      str++;
    } else {
      Dde_CVPutCyrillicChar(cv,x,y,*str);
      x+=DDE_FONT_WIDTH;
      str++;
    }
  }
}

void Dde_CVPutInvCyrillicStr(DdeCV *cv,int x,int y,char *str)
{
  int x0=x;
  while (*str) {
    if (*str=='\n') {
      x=x0;
      y+=DDE_FONT_HEIGHT;
      str++;
    } else if (*str=='\t') {
      x+=DDE_TAB_WIDTH*DDE_FONT_WIDTH;
      str++;
    } else {
      Dde_CVPutInvCyrillicChar(cv,x,y,*str);
      x+=DDE_FONT_WIDTH;
      str++;
    }
  }
}

void Dde_CVPutStr(DdeCV *cv,int x,int y,char *str)
{
  Dde_CVPutStdStr(cv,x,y,str);
}

void Dde_CVPutInvStr(DdeCV *cv,int x,int y,char *str)
{
  Dde_CVPutInvStdStr(cv,x,y,str);
}

#define DDE_CV_PRINT_MAX 1024*8

void Dde_CVPrintStd(DdeCV *cv,int x,int y,char *fmt,...)
{
  va_list args;
  va_start(args,fmt);
  static char buff[DDE_CV_PRINT_MAX]={0};
  vsnprintf(buff,DDE_CV_PRINT_MAX,fmt,args);
  Dde_CVPutStdStr(cv,x,y,buff);
  va_end(args);
}

void Dde_CVPrintInvStd(DdeCV *cv,int x,int y,char *fmt,...)
{
  va_list args;
  va_start(args,fmt);
  static char buff[DDE_CV_PRINT_MAX]={0};
  vsnprintf(buff,DDE_CV_PRINT_MAX,fmt,args);
  Dde_CVPutInvStdStr(cv,x,y,buff);
  va_end(args);
}

void Dde_CVPrintCyrillic(DdeCV *cv,int x,int y,char *fmt,...)
{
  va_list args;
  va_start(args,fmt);
  static char buff[DDE_CV_PRINT_MAX]={0};
  vsnprintf(buff,DDE_CV_PRINT_MAX,fmt,args);
  Dde_CVPutCyrillicStr(cv,x,y,buff);
  va_end(args);
}

void Dde_CVPrintInvCyrillic(DdeCV *cv,int x,int y,char *fmt,...)
{
  va_list args;
  va_start(args,fmt);
  static char buff[DDE_CV_PRINT_MAX]={0};
  vsnprintf(buff,DDE_CV_PRINT_MAX,fmt,args);
  Dde_CVPutInvCyrillicStr(cv,x,y,buff);
  va_end(args);
}

void Dde_CVPrint(DdeCV *cv,int x,int y,char *fmt,...)
{
  va_list args;
  va_start(args,fmt);
  static char buff[DDE_CV_PRINT_MAX]={0};
  vsnprintf(buff,DDE_CV_PRINT_MAX,fmt,args);
  Dde_CVPutStr(cv,x,y,buff);
  va_end(args);
}

void Dde_CVPrintInv(DdeCV *cv,int x,int y,char *fmt,...)
{
  va_list args;
  va_start(args,fmt);
  static char buff[DDE_CV_PRINT_MAX]={0};
  vsnprintf(buff,DDE_CV_PRINT_MAX,fmt,args);
  Dde_CVPutInvStr(cv,x,y,buff);
  va_end(args);
}

// ******************************************************************** //

struct _DdeGL
{
  HGLRC hrc;
};typedef struct _DdeGL DdeGL;
DdeGL gl;

DdeGL *Dde_GLAlias()
{
  return &gl;
}

void Dde_GLInit()
{
  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd,0,sizeof(pfd));
  pfd.nSize=sizeof(pfd);
  pfd.nVersion=1;
  pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
  pfd.iPixelType=PFD_TYPE_RGBA;
  pfd.cColorBits=32;

  const int pf=ChoosePixelFormat(win.hdc,&pfd);
  SetPixelFormat(win.hdc,pf,&pfd);
  gl.hrc=wglCreateContext(win.hdc);
  wglMakeCurrent(win.hdc,gl.hrc);
  glEnable(GL_TEXTURE_2D);
}

void Dde_GLClose()
{
  glDisable(GL_TEXTURE_2D);
  wglMakeCurrent(NULL,NULL);
  wglDeleteContext(gl.hrc);
}

void Dde_GLRender(DdeCV *cv)
{

  glViewport(0,0,win.width,win.height);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindTexture(GL_TEXTURE_2D,cv->id);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,cv->width,cv->height,0,
               GL_RGBA,GL_UNSIGNED_BYTE,cv->buff);
  glBegin(GL_QUADS);
  {
    glTexCoord2f(0.0f,0.0f);glVertex2f(-1.0f, 1.0f);
    glTexCoord2f(1.0f,0.0f);glVertex2f( 1.0f, 1.0f);
    glTexCoord2f(1.0f,1.0f);glVertex2f( 1.0f,-1.0f);
    glTexCoord2f(0.0f,1.0f);glVertex2f(-1.0f,-1.0f);
  } glEnd();
  glBindTexture(GL_TEXTURE_2D,cv->id);
}

// ******************************************************************** //

#endif // SPEC_H
