#ifndef __onekeypad_h
#define __onekeypad_h 1

#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingA.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/ArrowBG.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/DialogS.h>
#include <Xm/ScrolledW.h>
#include <Xm/PanedW.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>

#include "utility.h"

typedef struct onekeypadWidgetListTag {
  struct widgetListTag *flink;
  Widget pb;
} onekeypadWidgetListType, *onekeypadWidgetListPtr;

#ifdef __onekeypad_cc

static void onekeypadPress (
  Widget w,
  XtPointer client,
  XtPointer call,
  int dataType );

static void doubleOnekeypadPress (
  Widget w,
  XtPointer client,
  XtPointer call );

static void intOnekeypadPress (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class onekeypadClass {

private:

friend void onekeypadPress (
  Widget w,
  XtPointer client,
  XtPointer call,
  int dataType );

friend void doubleOnekeypadPress (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void intOnekeypadPress (
  Widget w,
  XtPointer client,
  XtPointer call );

static const int MAXCHARS_TOP = 14;
static const int ISNULL = 0;
static const int ZERO = 1;
static const int NODECPOINT = 2;
static const int DECPOINT = 3;
static const int INT = 100;
static const int DOUBLE = 101;

int MAXCHARS;

int x, y, count, positive, state, poppedUp;
char *entryTag, *actionTag;
Display *display;
Widget shell, rowcol, kprowcol, topForm, mainForm, bottomForm, text,
 pb0, pb1, pb2, pb3, pb4, pb5, pb6, pb7, pb8, pb9,
 pbPoint, pbSign, pbOK, pbApply, pbCancel, pbBksp,
 pba, pbb, pbc, pbd, pbe, pbf;
onekeypadWidgetListPtr wlHead, wlTail;
char buf[MAXCHARS_TOP+1];

int *intDest;
double *doubleDest;

int hex;

void *userPtr;
void (*okFunc)(Widget,XtPointer,XtPointer);
void (*cancelFunc)(Widget,XtPointer,XtPointer);

public:

onekeypadClass ();

~onekeypadClass ( void );

void popup ( void );

void popdown ( void );

int isPoppedUp ( void );

int create (
  Widget top,
  //Colormap cmap,
  int _x,
  int _y,
  char *label,
  //fontInfoClass *fi,
  //const char *entryFontTag,
  //const char *actionFontTag,
  int dataType,
  void *destination,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc );

int create (
  Widget top,
  //Colormap cmap,
  int _x,
  int _y,
  char *label,
  //fontInfoClass *fi,
  //const char *entryFontTag,
  //const char *actionFontTag,
  int *destination,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc );

int create (
  Widget top,
  //Colormap cmap,
  int _x,
  int _y,
  char *label,
  //fontInfoClass *fi,
  //const char *entryFontTag,
  //const char *actionFontTag,
  double *destination,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc );

int createHex (
  Widget top,
  //Colormap cmap,
  int _x,
  int _y,
  char *label,
  //fontInfoClass *fi,
  //const char *entryFontTag,
  //const char *actionFontTag,
  int *destination,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc );

int createHex (
  Widget top,
  //Colormap cmap,
  int _x,
  int _y,
  char *label,
  //fontInfoClass *fi,
  //const char *entryFontTag,
  //const char *actionFontTag,
  double *destination,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc );

};

#endif




























