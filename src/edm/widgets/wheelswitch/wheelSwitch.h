
#ifndef __wheelSwitch_h
#define __wheelSwitch_h 1

#include "ulBindings.h"
#include "act_grf.h"
#include "entry_form.h"
#include "utility.h"
#include "keypad.h"
#include "onekeypad.h"

#include "pv_factory.h"
#include "cvtFast.h"


#ifdef __wheelSwitch_cc

#include "wheelSwitch.str"

static int checkSelection (int selected, XtPointer client);

static void wheelSwitchEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void slc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sl_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void xtdoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetKpIntValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void setd ( int which, XtPointer client, int ud );
static void updown ( int which, XtPointer client, int ud );

#endif


class wheelSwitchClass : public activeGraphicClass {

private:

friend int checkSelection (int selected, XtPointer client);

friend void wheelSwitchEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void slc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sl_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void xtdoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetKpIntValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void setd ( int which, XtPointer client, int ud );
friend void updown ( int which, XtPointer client, int ud );

/*typedef struct editBufTag {
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  char bufTitle[64];
  int bufTitleDisplay;
  char bufTitlePosition[16];
  int bufOuterRect;
  char controlBufPvName[PV_Factory::MAX_PV_NAME + 1];
  efInt bufEfPrecision, bufEfLeadingDigits;
  int bufLimitsFromDb;
  int bufPrecisionFromDb;
  int bufLeadingDigits;
  int bufLeadingPlus;
  int bufBoundsVisible;
  int bufDisplayUnits;
  int bufUnitsFromDb;
  char bufUnits[16];
  efDouble bufEfScaleMin, bufEfScaleMax;
  char bufDisplayFormat[16];
  int bufFgColor, bufBgColor, bufControlColor, bufOfflineColor;
  int bufFgColorMode, bufBgColorMode, bufControlColorMode;
  char bufTitleFontTag[64], bufDisplayFontTag[64];
} editBufType, *editBufPtr;*/

  int bufX;
  int bufY;
  int bufW;
  int bufH;
  char bufTitle[64];
  int bufTitleDisplay;
  char bufTitlePosition[16];
  int bufOuterRect;
  char controlBufPvName[PV_Factory::MAX_PV_NAME + 1];
  efInt bufEfPrecision, bufEfLeadingDigits;
  int bufLimitsFromDb;
  int bufPrecisionFromDb;
  int bufLeadingDigits;
  int bufLeadingPlus;
  int bufBoundsVisible;
  int bufDisplayUnits;
  int bufUnitsFromDb;
  char bufUnits[16];
  efDouble bufEfScaleMin, bufEfScaleMax;
  char bufDisplayFormat[16];
  int bufFgColor, bufBgColor, bufControlColor;
  int bufFgColorMode, bufBgColorMode, bufControlColorMode;
  char bufTitleFontTag[64], bufDisplayFontTag[64];


int controlPvConnected;
int controlExists;
int activeMode;
ProcessVariable *controlPvId;

onekeypadClass kp;
int kpInt;
int editDialogIsActive;

int xx;
char title[64];
int titleDisplay;
char titlePosition[16];
int outerRect;
char valueS[16], oldValueS[16], value2[16], format[16], displayFormat[16], units[MAX_UNITS_SIZE + 1], oldUnits[MAX_UNITS_SIZE + 1];
double valueD, scaleMin, scaleMax;
//editBufPtr eBuf;
expStringClass controlPvName;
int precision;
efInt efPrecision, efLeadingDigits;
int limitsFromDb, precisionFromDb;
efDouble efScaleMin, efScaleMax;
Widget frameWidget, wheelSwitchWidget;
pvColorClass fgColor, bgColor, controlColor, shadeColor;
int fgColorMode, bgColorMode, controlColorMode;
fontMenuClass fmT, fmD;
XFontStruct *fsT, *fsD;
colorButtonClass fgCb, bgCb, controlCb, offlineCb;
char titleFontTag[64], displayFontTag[64];
int digitNumber, halfDigit, offset, selected, oldSelected, leadingDigits;
int leadingPlus, boundsVisible, displayUnits, unitsFromDb;
int titleFontAscent, titleFontDescent, titleFontHeight, titleStringWidth;
int displayFontAscent, displayFontDescent, displayFontHeight, displayStringWidth;
int oldStat, oldSev;
int unconnectedTimer, needToDrawUnconnected, init;

public:

wheelSwitchClass ( void );

wheelSwitchClass
 ( const wheelSwitchClass *source );

~wheelSwitchClass ( void ) {
  if ( name ) delete[] name;
  if ( unconnectedTimer ) {
    XtRemoveTimeOut ( unconnectedTimer );
    unconnectedTimer = 0;
  }
}

char *objName ( void ) {
  return name;
}

int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int save ( FILE *f );

int createFromFile ( FILE *fptr, char *name, activeWindowClass *actWin );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate ( int pass, void *ptr );

int deactivate ( int pass );

void executeDeferred ( void );

int expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int containsMacros ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_wheelSwitchClassPtr ( void );
void *clone_wheelSwitchClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
