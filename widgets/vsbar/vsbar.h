//  edm - extensible display manager

//  Copyright (C) 1999 John W. Sinclair

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef __vsbar_h
#define __vsbar_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define BARC_K_COLORMODE_STATIC 0
#define BARC_K_COLORMODE_ALARM 1

#define BARC_MAJOR_VERSION 4
#define BARC_MINOR_VERSION 0
#define BARC_RELEASE 0

#define BARC_K_PV_NAME 0
#define BARC_K_LITERAL 1

#define BARC_K_MAX_GE_MIN 1
#define BARC_K_MAX_LT_MIN 2

#define BARC_K_LIMITS_FROM_DB 1
#define BARC_K_LIMITS_FROM_FORM 0

#ifdef __vsbar_cc

#include "vsbar.str"

static char *dragName[] = {
  activeVsBarClass_str1,
  activeVsBarClass_str2,
};

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void barc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void bar_readUpdate (
  ProcessVariable *pv,
  void *userarg );

static void bar_nullUpdate (
  ProcessVariable *pv,
  void *userarg );

static void bar_maxUpdate (
  ProcessVariable *pv,
  void *userarg );

static void bar_minUpdate (
  ProcessVariable *pv,
  void *userarg );

static void bar_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void bar_monitor_null_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void bar_monitor_max_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void bar_monitor_min_connect_state (
  ProcessVariable *pv,
  void *userarg );

#endif

class activeVsBarClass : public activeGraphicClass {

private:

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void barc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void bar_readUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void bar_nullUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void bar_maxUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void bar_minUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void bar_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void bar_monitor_null_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void bar_monitor_max_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void bar_monitor_min_connect_state (
  ProcessVariable *pv,
  void *userarg );

int horizontal, bufHorizontal;

typedef struct editBufTag {
// edit buffer
  char bufReadPvName[PV_Factory::MAX_PV_NAME + 1];
  char bufNullPvName[PV_Factory::MAX_PV_NAME + 1];
  char bufMaxPvName[PV_Factory::MAX_PV_NAME + 1];
  char bufMinPvName[PV_Factory::MAX_PV_NAME + 1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

int bufX, bufY, bufW, bufH;

int opComplete;

int minW, minVertW;
int minH, minVertH;

double readV, curReadV, curNullV, curMaxV, curMinV;
int barY, oldBarY, barH, oldBarH, barW, oldBarW, bufInvalid, barX, oldBarX,
 originW, originH, mode, barAreaX, barAreaW, barAreaY, barAreaH, barStrLen, barMaxW, barMaxH,
 aboveBarOrigin, oldAboveBarOrigin, zeroCrossover;
double barOriginX, barOriginY, factorLt, factorGe;
efDouble efBarOriginX;

fontMenuClass fm;
char fontTag[63 + 1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

ProcessVariable *readPvId, *nullPvId, *maxPvId, *minPvId;
int initialReadConnection, initialNullConnection, initialMaxConnection, initialMinConnection;
int oldStat, oldSev;

expStringClass readPvExpStr, nullPvExpStr, maxPvExpStr, minPvExpStr;

unsigned char pvNotConnectedMask;

int readExists, nullExists, maxExists, minExists;

int init, active, activeMode;

int barColourMode, fgColourMode;
pvColorClass barColour, fgColour, bgColour;
colorButtonClass barCb, fgCb, bgCb;
char label[39 + 1];
int labelType;
int border;
int showScale;
int labelTicks, majorTicks, minorTicks;
char scaleFormat[15 + 1];
int limitsFromDb, limitsFromPVs;
double readMin, readMax;
efDouble efReadMin, efReadMax;
int precision;
efInt efPrecision;

int bufBarColourMode, bufFgColourMode;
int bufBarColour, bufFgColour, bufBgColour;
char bufLabel[39 + 1];
int bufLabelType;
int bufBorder;
int bufShowScale;
int bufLabelTicks, bufMajorTicks, bufMinorTicks;
char bufFontTag[63 + 1];
char bufScaleFormat[15 + 1];
int bufLimitsFromDb, bufLimitsFromPVs;
efDouble bufEfReadMin, bufEfReadMax;
efInt bufEfPrecision;
efDouble bufEfBarOriginX;

int needErase, needDraw, needFullDraw, needDrawCheck, needConnectInit,
 needRefresh, needInfoInit;
int needToDrawUnconnected, needToEraseUnconnected;
int unconnectedTimer;

public:

activeVsBarClass ( void );

activeVsBarClass
 ( const activeVsBarClass *source );

~activeVsBarClass ( void );

char *objName ( void ) {

  return name;

}

int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int save (
  FILE *f );

int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate ( int pass, void *ptr );

int deactivate ( int pass );

void updateDimensions ( void );

void bufInvalidate ( void );

int expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int containsMacros ( void );

void btnUp (
  int x,
  int y,
  int barState,
  int barNumber );

void btnDown (
  int x,
  int y,
  int barState,
  int barNumber );

void btnDrag (
  int x,
  int y,
  int barState,
  int barNumber );

int getBarActionRequest (
  int *up,
  int *down,
  int *drag );

int checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void drawScale (
  Widget widget,
  gcClass *gc );

void drawHorzScale (
  Widget widget,
  gcClass *gc );

void drawVertScale (
  Widget widget,
  gcClass *gc );

void updateScaleInfo ( void );

void updateHorzScaleInfo ( void );

void updateVertScaleInfo ( void );

void updateBar ( void );

void executeDeferred ( void );

char *firstDragName ( void );

char *nextDragName ( void );

char *dragValue (
  int i );

void changeDisplayParams (
  unsigned int flag,
  char *fontTag,
  int alignment,
  char *ctlFontTag,
  int ctlAlignment,
  char *btnFontTag,
  int btnAlignment,
  int textFgColour,
  int fg1Colour,
  int fg2Colour,
  int offsetColour,
  int bgColour,
  int topShadowColour,
  int botShadowColour );

void changePvNames (
  int flag,
  int numCtlPvs,
  char *ctlPvs[],
  int numReadbackPvs,
  char *readbackPvs[],
  int numNullPvs,
  char *nullPvs[],
  int numVisPvs,
  char *visPvs[],
  int numAlarmPvs,
  char *alarmPvs[] );

void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeVsBarClassPtr ( void );
void *clone_activeVsBarClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
