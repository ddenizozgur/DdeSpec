#include "Spec.h"

int main()
{
  Dde_WinInit(DDE_WIN_POS_DEFAULT,DDE_WIN_POS_DEFAULT,
              1280,960,L"Title",DDE_WIN_OVERLAPPEDWINDOW);
  Dde_GLInit();
  Dde_TimerInit();

  DdeWin *win=Dde_WinAlias();
  DdeTimer *time=Dde_TimerAlias();
  DdeKbd *kbd=Dde_KbdAlias();
  DdeMs *ms=Dde_MsAlias();

  DdeCV *cv=malloc(sizeof(DdeCV));
  Dde_CVNew(cv,800,600,DDE_CV_LINEAR);

  Dde_WinShow(DDE_WIN_SHOW);
  while (!win->should_close) {
    Dde_TimerUpdate();
    Dde_WinPeekMsg();

    cv->color=(DdeRGBAU32){0xff,0xff,0xff,0xff};
    Dde_CVFill(cv);

    for (int j=0;j<cv->height/DDE_FONT_HEIGHT;j++)
      for (int i=0;i<cv->width/DDE_FONT_WIDTH;i++) {
        cv->color.r=rand()%256;
        cv->color.g=rand()%256;
        cv->color.b=rand()%256;
        Dde_CVPlotU64(cv,i*DDE_FONT_WIDTH,j*DDE_FONT_HEIGHT,
                      font_std[rand()%DDE_FONT_MAX]);
      }

    cv->color=(DdeRGBAU32){0,0,0,0xff};
    if (ms->lb.held) {
      Dde_CVPrint(cv,DDE_FONT_WIDTH*14,DDE_FONT_HEIGHT*7,
                  "ms->lb.pressed is true\n");
    }
    if (ms->rb.held) {
      Dde_CVPrint(cv,DDE_FONT_WIDTH*14,DDE_FONT_HEIGHT*7,
                  "ms->rb.pressed is true\n");
    }
    if (kbd->key[VK_RIGHT].held) {
      Dde_CVPrint(cv,DDE_FONT_WIDTH*14,DDE_FONT_HEIGHT*7,
                  "kbd->key[VK_RIGHT].held is true\n");
    }
    Dde_CVPrint(cv,DDE_FONT_WIDTH,DDE_FONT_HEIGHT*1,
                "delta_time: %.4f\nfps: %.3f",
                time->delta,time->fps);
    Dde_CVPrint(cv,DDE_FONT_WIDTH*14,DDE_FONT_HEIGHT*10,text4print);

    Dde_GLRender(cv);
    Dde_WinSwapBuffers();
  }

  Dde_CVDel(cv);
  Dde_GLClose();
  Dde_WinClose();
  return 0;
}
