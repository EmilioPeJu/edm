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

#define __menu_mux_cc 1

#include "menu_muxPV.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void doBlink (
  void *ptr
) 
{

menuMuxPVClass *mmo = (menuMuxPVClass *) ptr;

  if ( !mmo->activeMode )
  {
    if ( mmo->isSelected () ) mmo->drawSelectBoxCorners (); // erase via xor
    mmo->smartDrawAll ();
    if ( mmo->isSelected () ) mmo->drawSelectBoxCorners ();
  }
  else
  {
    mmo->bufInvalidate ();
    mmo->needDraw = 1;
    mmo->actWin->addDefExeNode ( mmo->aglPtr );
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

menuMuxPVClass *mmo = (menuMuxPVClass *) client;

  if ( !mmo->controlPvConnected )
  {
    if ( mmo->controlExists )
    {
      mmo->needToDrawUnconnected = 1;
      mmo->needDraw = 1;
      mmo->actWin->addDefExeNode ( mmo->aglPtr );
    }
  }

  mmo->unconnectedTimer = 0;

}

static void mmuxSetItem (
  Widget w,
  XtPointer client,
  XtPointer call )
{

efSetItemCallbackDscPtr dsc = (efSetItemCallbackDscPtr) client;
entryFormClass *ef = (entryFormClass *) dsc->ef;
menuMuxPVClass *mmo = (menuMuxPVClass *) dsc->obj;
int i;

  mmo->elbt->setValue ( mmo->eBuf->bufTag[ef->index] );

  for ( i = 0; i < MMUX_MAX_ENTRIES; i++ )
  {
    mmo->elbm[i]->setValue ( mmo->eBuf->bufM[ef->index][i] );
    mmo->elbe[i]->setValue ( mmo->eBuf->bufE[ef->index][i] );
  }

}

static void mmux_putValueNoPv (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxPVClass *mmuxo = (menuMuxPVClass *) client;
int i;

  if ( !mmuxo->active ) return;

  for ( i = 0; i < mmuxo->numStates; i++ )
  {

    if ( w == mmuxo->pb[i] ) 
    {

      mmuxo->actWin->appCtx->proc->lock ();

      mmuxo->curControlV = i;

      if ( mmuxo->curControlV < 0 )
        mmuxo->curControlV = 0;
      else if ( mmuxo->curControlV >= mmuxo->numStates )
        mmuxo->curControlV = mmuxo->numStates - 1;

      mmuxo->needUpdate = 1;

      mmuxo->actWin->addDefExeNode ( mmuxo->aglPtr );

      mmuxo->actWin->appCtx->proc->unlock ();

    }

  }

}

static void mmux_putValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxPVClass *mmuxo = (menuMuxPVClass *) client;
int i, value;

  for ( i = 0; i < mmuxo->numStates; i++ )
  {

    if ( w == mmuxo->pb[i] )
    {
      value = i;
      mmuxo->controlPvId->put ( value );
      return;
    }

  }

}

static void mmux_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

  menuMuxPVClass *mmuxo = (menuMuxPVClass *) userarg;

  if ( pv->is_valid () )
  {
    mmuxo->controlPvNotConnectedMask = 0;
    bool allExpansionPvsConnected = true;
    for (int i = 0; i < MMUX_MAX_ENTRIES; i++)
      if (mmuxo->expansionPvNotConnectedMask[i])
        allExpansionPvsConnected = false;
    if (allExpansionPvsConnected) 
    {
      mmuxo->needConnectInit = 1;
      mmuxo->actWin->appCtx->proc->lock ();
      mmuxo->actWin->addDefExeNode ( mmuxo->aglPtr );
      mmuxo->actWin->appCtx->proc->unlock ();
    }
  }
  else
  {
    mmuxo->controlPvNotConnectedMask = 1;
    bool allExpansionPvsConnected = true;
    for (int i = 0; i < MMUX_MAX_ENTRIES; i++)
      if (mmuxo->expansionPvNotConnectedMask[i])
        allExpansionPvsConnected = false;
    if (allExpansionPvsConnected) 
    {
      mmuxo->needDisconnect = 1;
      mmuxo->actWin->appCtx->proc->lock ();
      mmuxo->actWin->addDefExeNode ( mmuxo->aglPtr );
      mmuxo->actWin->appCtx->proc->unlock ();
    }
  }
}

static void mmux_monitor_expansion_connect_state (
  ProcessVariable *pv,
  void *userarg )
{
#ifdef DEBUG
  printf ("Start of mmux_monitor_expansion_connect_state\n");
#endif
  menuMuxPVClass *mmuxo = (menuMuxPVClass *) userarg;

  if ( pv->is_valid () )
  {
    for (int n = 0; n < mmuxo->numItems; n++)
    {
      for (int i = 0; i <MMUX_MAX_ENTRIES; i++)
      {
        if (pv == mmuxo->expansionPVs[n][i])
        {
          mmuxo->expansionPvNotConnectedMask[i] &= ~((unsigned int)(1 << n));
          bool allExpansionPvsConnected = true;
          for (int j = 0; j < MMUX_MAX_ENTRIES; j++)
            if (mmuxo->expansionPvNotConnectedMask[j])
              allExpansionPvsConnected = false;
          if (allExpansionPvsConnected && (!mmuxo->controlPvNotConnectedMask)) 
          {
            mmuxo->needConnectInit = 1;
            mmuxo->actWin->appCtx->proc->lock ();
            mmuxo->actWin->addDefExeNode ( mmuxo->aglPtr );
            mmuxo->actWin->appCtx->proc->unlock ();
          }
        }
      }
    }
  }
  else
  {
    for (int n = 0; n < mmuxo->numItems; n++)
    {
      for (int i = 0; i <MMUX_MAX_ENTRIES; i++)
      {
        if (pv == mmuxo->expansionPVs[n][i])
        {
          bool allPvsWereConnected = true;
          for (int j = 0; j < MMUX_MAX_ENTRIES; j++)
            if (mmuxo->expansionPvNotConnectedMask[j])
              allPvsWereConnected = false;
          if (mmuxo->controlPvNotConnectedMask)
            allPvsWereConnected = false;
          mmuxo->expansionPvNotConnectedMask[i] |= (1 << n);
          if (allPvsWereConnected) 
          {
            mmuxo->needDisconnect = 1;
            mmuxo->actWin->appCtx->proc->lock ();
            mmuxo->actWin->addDefExeNode ( mmuxo->aglPtr );
            mmuxo->actWin->appCtx->proc->unlock ();
          }
        }
      }
    }
  }
#ifdef DEBUG
  printf ("End of mmux_monitor_expansion_connect_state\n");
#endif
}

static void mmux_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

menuMuxPVClass *mmuxo = (menuMuxPVClass *) userarg;
int st, sev;

  if ( !mmuxo->active ) return;

  mmuxo->actWin->appCtx->proc->lock ();

  mmuxo->curControlV = pv->get_int ();
  if ( mmuxo->curControlV < 0 )
    mmuxo->curControlV = 0;
  else if ( mmuxo->curControlV >= mmuxo->numStates )
    mmuxo->curControlV = mmuxo->numStates - 1;

  st = pv->get_status ();
  sev = pv->get_severity ();
  if ( ( st != mmuxo->oldStat ) || ( sev != mmuxo->oldSev ) )
  {
    mmuxo->oldStat = st;
    mmuxo->oldSev = sev;
    mmuxo->fgColour.setStatus ( st, sev );
    mmuxo->bgColour.setStatus ( st, sev );
    mmuxo->bufInvalidate ();
    mmuxo->needDraw = 1;
  }

  mmuxo->needUpdate = 1;
  mmuxo->actWin->addDefExeNode ( mmuxo->aglPtr );

  mmuxo->actWin->appCtx->proc->unlock ();

}

static void mmux_expansionUpdate (
  ProcessVariable *pv,
  void *userarg )
{
#ifdef DEBUG
  printf ("Start of mmux_expansionUpdate - pv %s\n", pv->get_name());
#endif
  menuMuxPVClass *mmuxo = (menuMuxPVClass *) userarg;

  if ( !mmuxo->active )
  {
#ifdef DEBUG
    printf ("mmux_expansionUpdate - !active\n");
#endif
    return;
  }

  for (int n = 0; n < mmuxo->numItems; n++)
  {
    for (int i = 0; i <MMUX_MAX_ENTRIES; i++)
    {
      if (pv == mmuxo->expansionPVs[n][i])
      {
#ifdef DEBUG
        printf ("mmux_expansionUpdate - PV match found for n = %d i = %d\n",
                n, i);
#endif
        pv->get_string (mmuxo->expansionStrings[n][i], MMUX_MAX_STRING_SIZE); 
      }
    }
  }
  mmuxo->actWin->appCtx->proc->lock ();
  mmuxo->needUpdate = 1;
  mmuxo->actWin->addDefExeNode ( mmuxo->aglPtr );

  mmuxo->actWin->appCtx->proc->unlock ();

#ifdef DEBUG
  printf ("End of mmux_expansionUpdate\n");
#endif
}

static void mmuxc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxPVClass *mmuxo = (menuMuxPVClass *) client;
int i, ii;

  mmuxo->actWin->setChanged ();

  mmuxo->eraseSelectBoxCorners ();
  mmuxo->erase ();

  strncpy ( mmuxo->fontTag, mmuxo->fm.currentFontTag (), 63 + 1 );
  mmuxo->actWin->fi->loadFontTag ( mmuxo->fontTag );
  mmuxo->actWin->drawGc.setFontTag ( mmuxo->fontTag, mmuxo->actWin->fi );
  mmuxo->actWin->fi->getTextFontList ( mmuxo->fontTag, &mmuxo->fontList );
  mmuxo->fs = mmuxo->actWin->fi->getXFontStruct ( mmuxo->fontTag );

  mmuxo->topShadowColour = mmuxo->eBuf->bufTopShadowColour;
  mmuxo->botShadowColour = mmuxo->eBuf->bufBotShadowColour;

  mmuxo->fgColourMode = mmuxo->eBuf->bufFgColourMode;
  if ( mmuxo->fgColourMode == MMUXC_K_COLOURMODE_ALARM )
    mmuxo->fgColour.setAlarmSensitive ();
  else
    mmuxo->fgColour.setAlarmInsensitive ();
  mmuxo->fgColour.setColorIndex ( mmuxo->eBuf->bufFgColour, mmuxo->actWin->ci );

  mmuxo->bgColourMode = mmuxo->eBuf->bufBgColourMode;
  if ( mmuxo->bgColourMode == MMUXC_K_COLOURMODE_ALARM )
    mmuxo->bgColour.setAlarmSensitive ();
  else
    mmuxo->bgColour.setAlarmInsensitive ();
  mmuxo->bgColour.setColorIndex ( mmuxo->eBuf->bufBgColour, mmuxo->actWin->ci );

  mmuxo->x = mmuxo->eBuf->bufX;
  mmuxo->sboxX = mmuxo->eBuf->bufX;

  mmuxo->y = mmuxo->eBuf->bufY;
  mmuxo->sboxY = mmuxo->eBuf->bufY;

  mmuxo->w = mmuxo->eBuf->bufW;
  mmuxo->sboxW = mmuxo->eBuf->bufW;

  mmuxo->h = mmuxo->eBuf->bufH;
  mmuxo->sboxH = mmuxo->eBuf->bufH;

  mmuxo->controlPvExpStr.setRaw ( mmuxo->eBuf->bufControlPvName );

  mmuxo->initialStateExpStr.setRaw ( mmuxo->eBuf->bufInitialState );

  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    strncpy ( mmuxo->tag[i], mmuxo->eBuf->bufTag[i],
              MMUX_MAX_STRING_SIZE + 1 );
    if ( strlen (mmuxo->tag[i]) == 0 )
    {
      strcpy ( mmuxo->tag[i], "?" );
    }
  }

  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    for ( ii = 0; ii < MMUX_MAX_ENTRIES; ii++ )
    {
      strncpy ( mmuxo->macroStrings[i][ii], mmuxo->eBuf->bufM[i][ii],
                MMUX_MAX_STRING_SIZE + 1 );
      strncpy ( mmuxo->expansionPVNames[i][ii], mmuxo->eBuf->bufE[i][ii],
                MMUX_MAX_STRING_SIZE + 1 );
      strcpy ( mmuxo->expansionStrings[i][ii], "");
    }
  }

  mmuxo->numItems = mmuxo->ef.numItems;

  mmuxo->updateDimensions ();

}

static void mmuxc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxPVClass *mmuxo = (menuMuxPVClass *) client;

  mmuxc_edit_update ( w, client, call );
  mmuxo->refresh ( mmuxo );

}

static void mmuxc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxPVClass *mmuxo = (menuMuxPVClass *) client;

  mmuxc_edit_update ( w, client, call );
  mmuxo->ef.popdown ();
  mmuxo->operationComplete ();

}

static void mmuxc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxPVClass *mmuxo = (menuMuxPVClass *) client;

  mmuxo->ef.popdown ();
  mmuxo->operationCancel ();

}

static void mmuxc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxPVClass *mmuxo = (menuMuxPVClass *) client;

  mmuxo->ef.popdown ();
  mmuxo->operationCancel ();
  mmuxo->erase ();
  mmuxo->deleteRequest = 1;
  mmuxo->drawAll ();

}

menuMuxPVClass::menuMuxPVClass ( void )
{

int i, ii;

  name = new char[strlen ("menuMuxPVClass") + 1];
  strcpy ( name, "menuMuxPVClass" );

  numStates = 0;

  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    stateString[i] = NULL;
    pb[i] = NULL;
  }

  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    strcpy ( tag[i], "" );
  }
  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    for ( ii = 0; ii < MMUX_MAX_ENTRIES; ii++ )
    {
      strcpy ( macroStrings[i][ii], "" );
      strcpy ( expansionPVNames[i][ii], "" );
      strcpy ( expansionStrings[i][ii], "" );
    }
  }

  numItems = 2;
  numMac = 0;
  mac = NULL;
  exp = NULL;

  fgColourMode = MMUXC_K_COLOURMODE_STATIC;
  bgColourMode = MMUXC_K_COLOURMODE_STATIC;

  active = 0;
  activeMode = 0;
  widgetsCreated = 0;
  fontList = NULL;
  unconnectedTimer = 0;

  eBuf = NULL;

  setBlinkFunction ( (void *) doBlink );

}

// copy constructor
menuMuxPVClass::menuMuxPVClass
 ( const menuMuxPVClass *source )
{

int i, ii;
activeGraphicClass *mmuxo = (activeGraphicClass *) this;

  mmuxo->clone ( (activeGraphicClass *) source );

  name = new char[strlen ("menuMuxPVClass") + 1];
  strcpy ( name, "menuMuxPVClass" );

  numItems = source->numItems;

  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    stateString[i] = NULL;
    pb[i] = NULL;
  }

  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    strncpy ( tag[i], source->tag[i], MMUX_MAX_STRING_SIZE + 1 );
  }
  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    for ( ii = 0; ii < MMUX_MAX_ENTRIES; ii++ )
    {
      strncpy ( macroStrings[i][ii], source->macroStrings[i][ii],
                MMUX_MAX_STRING_SIZE + 1 );
      strncpy ( expansionPVNames[i][ii], source->expansionPVNames[i][ii],
                MMUX_MAX_STRING_SIZE + 1 );
      strcpy ( expansionStrings[i][ii], "");
    }
  }

  numMac = 0;
  mac = NULL;
  exp = NULL;

  strncpy ( fontTag, source->fontTag, 63 + 1 );
  fs = actWin->fi->getXFontStruct ( fontTag );
  actWin->fi->getTextFontList ( fontTag, &fontList );

  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;

  topShadowColour = source->topShadowColour;
  botShadowColour = source->botShadowColour;

  fgColour.copy (source->fgColour);
  bgColour.copy (source->bgColour);

  fgColourMode = source->fgColourMode;
  bgColourMode = source->bgColourMode;

  controlPvExpStr.copy ( source->controlPvExpStr );

  initialStateExpStr.copy ( source->initialStateExpStr );

  widgetsCreated = 0;
  active = 0;
  activeMode = 0;
  unconnectedTimer = 0;

  eBuf = NULL;

  setBlinkFunction ( (void *) doBlink );

}

menuMuxPVClass::~menuMuxPVClass ( void )
{

int i;

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer )
  {
    XtRemoveTimeOut ( unconnectedTimer );
    unconnectedTimer = 0;
  }

  if ( mac && exp )
  {
    for ( i = 0; i < numMac; i++ )
    {
      if ( mac[i] )
      {
        delete[] mac[i];
      }
      if ( exp[i] )
      {
        delete[] exp[i];
      }
    }
  }
  if ( mac ) delete[] mac;
  if ( exp ) delete[] exp;

  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    if ( stateString[i] ) delete[] stateString[i];
  }

  if ( fontList ) XmFontListFree ( fontList );

  updateBlink ( 0 );

}

int menuMuxPVClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h )
{

  actWin = (activeWindowClass *) aw_obj;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  strncpy ( fontTag, actWin->defaultBtnFontTag, 63 + 1 );
  actWin->fi->loadFontTag ( fontTag );
  fs = actWin->fi->getXFontStruct ( fontTag );
  actWin->fi->getTextFontList ( fontTag, &fontList );

  updateDimensions ();

  topShadowColour = actWin->defaultTopShadowColor;
  botShadowColour = actWin->defaultBotShadowColor;

  fgColour.setColorIndex ( actWin->defaultTextFgColor, actWin->ci );
  bgColour.setColorIndex ( actWin->defaultBgColor, actWin->ci );

  this->draw ();

  this->editCreate ();

  return 1;

}

int menuMuxPVClass::save (
  FILE *f )
{

int i, ii, stat, major, minor, release;
char tmpS[MMUX_MAX_ENTRIES][15 + 1], tmpV[MMUX_MAX_ENTRIES][15 + 1];
char tmpBufS[MMUX_MAX_ENTRIES][MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE + 1];
char tmpBufV[MMUX_MAX_ENTRIES][MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE + 1];

tagClass itemTag;

int zero = 0;
char *emptyStr = "";

  major = MMUXC_MAJOR_VERSION;
  minor = MMUXC_MINOR_VERSION;
  release = MMUXC_RELEASE;

  itemTag.init ();
  itemTag.loadW ( "beginObjectProperties" );
  itemTag.loadW ( "major", &major );
  itemTag.loadW ( "minor", &minor );
  itemTag.loadW ( "release", &release );
  itemTag.loadW ( "x", &x );
  itemTag.loadW ( "y", &y );
  itemTag.loadW ( "w", &w );
  itemTag.loadW ( "h", &h );
  itemTag.loadW ( "fgColour", actWin->ci, &fgColour );
  itemTag.loadBoolW ( "fgAlarm", &fgColourMode, &zero );
  itemTag.loadW ( "bgColour", actWin->ci, &bgColour );
  itemTag.loadBoolW ( "bgAlarm", &bgColourMode, &zero );
  itemTag.loadW ( "topShadowColour", actWin->ci, &topShadowColour );
  itemTag.loadW ( "botShadowColour", actWin->ci, &botShadowColour );
  itemTag.loadW ( "controlPv", &controlPvExpStr, emptyStr );
  itemTag.loadW ( "font", fontTag );
  itemTag.loadW ( "initialState", &initialStateExpStr, emptyStr );
  itemTag.loadW ( "numItems", &numItems );
  itemTag.loadW ( "symbolTag", MMUX_MAX_STRING_SIZE + 1, tag[0], numItems,
   emptyStr );
  for ( i = 0; i < MMUX_MAX_ENTRIES; i++ )
  {
    for ( ii = 0; ii < MMUX_MAX_STATES; ii++ )
    {
      strcpy ( tmpBufS[i][ii], macroStrings[ii][i] );
      strcpy ( tmpBufV[i][ii], expansionPVNames[ii][i] );
    }
  }
  for ( i = 0; i < MMUX_MAX_ENTRIES; i++ )
  {
    snprintf ( tmpS[i], 15, "symbol%-d", i );
    itemTag.loadW ( tmpS[i], MMUX_MAX_STRING_SIZE + 1, tmpBufS[i][0], numItems,
     emptyStr );
    snprintf ( tmpV[i], 15, "PV%-d", i );
    itemTag.loadW ( tmpV[i], MMUX_MAX_STRING_SIZE + 1, tmpBufV[i][0], numItems,
     emptyStr );
  }
  itemTag.loadW ( "endObjectProperties" );
  itemTag.loadW ( "" );

  stat = itemTag.writeTags ( f );

  return stat;

}

int menuMuxPVClass::old_save (
  FILE *f )
{

int index, i, ii;

  fprintf ( f, "%-d %-d %-d\n", MMUXC_MAJOR_VERSION, MMUXC_MINOR_VERSION,
   MMUXC_RELEASE );

  fprintf ( f, "%-d\n", x );
  fprintf ( f, "%-d\n", y );
  fprintf ( f, "%-d\n", w );
  fprintf ( f, "%-d\n", h );

  index = fgColour.pixelIndex ();
  actWin->ci->writeColorIndex ( f, index );

  fprintf ( f, "%-d\n", fgColourMode );

  index = bgColour.pixelIndex ();
  actWin->ci->writeColorIndex ( f, index );

  fprintf ( f, "%-d\n", bgColourMode );

  index = topShadowColour;
  actWin->ci->writeColorIndex ( f, index );

  index = botShadowColour;
  actWin->ci->writeColorIndex ( f, index );

  if ( controlPvExpStr.getRaw () )
    writeStringToFile ( f, controlPvExpStr.getRaw () );
  else
    writeStringToFile ( f, "" );

  writeStringToFile ( f, fontTag );

  fprintf ( f, "%-d\n", numItems );

  for ( i = 0; i < numItems; i++ )
  {
    writeStringToFile ( f, tag[i] );
  }

  for ( i = 0; i < numItems; i++ )
  {
    for ( ii = 0; ii < MMUX_MAX_ENTRIES; ii++ )
    {
      writeStringToFile ( f, macroStrings[i][ii] );
      writeStringToFile ( f, expansionPVNames[i][ii] );
    }
  }

  // version 1.2.0
  if ( initialStateExpStr.getRaw () )
    writeStringToFile ( f, initialStateExpStr.getRaw () );
  else
    writeStringToFile ( f, "" );

  return 1;

}

int menuMuxPVClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, ii, n, stat, major, minor, release;
char tmpS[MMUX_MAX_ENTRIES][15 + 1], tmpV[MMUX_MAX_ENTRIES][15 + 1];
char tmpBufS[MMUX_MAX_ENTRIES][MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE + 1];
char tmpBufV[MMUX_MAX_ENTRIES][MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE + 1];

tagClass itemTag;

int zero = 0;
char *emptyStr = "";

  for ( i = 0; i < MMUX_MAX_ENTRIES; i++ )
  {
    for ( ii = 0; ii < MMUX_MAX_STATES; ii++ )
    {
      strcpy ( tmpBufS[i][ii], "" );
      strcpy ( tmpBufV[i][ii], "" );
    }
  }

  this->actWin = _actWin;

  itemTag.init ();
  itemTag.loadR ( "beginObjectProperties" );
  itemTag.loadR ( "major", &major );
  itemTag.loadR ( "minor", &minor );
  itemTag.loadR ( "release", &release );
  itemTag.loadR ( "x", &x );
  itemTag.loadR ( "y", &y );
  itemTag.loadR ( "w", &w );
  itemTag.loadR ( "h", &h );
  itemTag.loadR ( "fgColour", actWin->ci, &fgColour );
  itemTag.loadR ( "fgAlarm", &fgColourMode, &zero );
  itemTag.loadR ( "bgColour", actWin->ci, &bgColour );
  itemTag.loadR ( "bgAlarm", &bgColourMode, &zero );
  itemTag.loadR ( "topShadowColour", actWin->ci, &topShadowColour );
  itemTag.loadR ( "botShadowColour", actWin->ci, &botShadowColour );
  itemTag.loadR ( "controlPv", &controlPvExpStr, emptyStr );
  itemTag.loadR ( "font", 63, fontTag );
  itemTag.loadR ( "initialState", &initialStateExpStr, emptyStr );
  itemTag.loadR ( "numItems", &numItems );
  itemTag.loadR ( "symbolTag", MMUX_MAX_STATES, MMUX_MAX_STRING_SIZE + 1,
                  tag[0], &numItems, emptyStr );
  for ( i = 0; i < MMUX_MAX_ENTRIES; i++ )
  {
    snprintf ( tmpS[i], 15, "symbol%-d", i );
    itemTag.loadR ( tmpS[i], MMUX_MAX_STATES, MMUX_MAX_STRING_SIZE + 1,
     tmpBufS[i][0], &n, emptyStr );
    snprintf ( tmpV[i], 15, "PV%-d", i );
    itemTag.loadR ( tmpV[i], MMUX_MAX_STATES, MMUX_MAX_STRING_SIZE + 1,
     tmpBufV[i][0], &n, emptyStr );
  }
  itemTag.loadR ( "endObjectProperties" );

  stat = itemTag.readTags ( f, "endObjectProperties" );

  if ( !( stat & 1 ) )
  {
    actWin->appCtx->postMessage ( itemTag.errMsg () );
  }

  if ( major > MMUXC_MAJOR_VERSION )
  {
    postIncompatable ();
    return 0;
  }

  if ( major < 4 )
  {
    postIncompatable ();
    return 0;
  }

  this->initSelectBox (); // call after getting x, y, w, h

  for ( i = 0; i < MMUX_MAX_ENTRIES; i++ )
  {
    for ( ii = 0; ii < MMUX_MAX_STATES; ii++ )
    {
      strcpy ( macroStrings[ii][i], tmpBufS[i][ii] );
      strcpy ( expansionPVNames[ii][i], tmpBufV[i][ii] );
      strcpy ( expansionStrings[ii][i], "");
    }
  }

  if ( fgColourMode == MMUXC_K_COLOURMODE_ALARM )
    fgColour.setAlarmSensitive ();
  else
    fgColour.setAlarmInsensitive ();

  if ( bgColourMode == MMUXC_K_COLOURMODE_ALARM )
    bgColour.setAlarmSensitive ();
  else
    bgColour.setAlarmInsensitive ();

  actWin->fi->loadFontTag ( fontTag );
  actWin->drawGc.setFontTag ( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct ( fontTag );
  actWin->fi->getTextFontList ( fontTag, &fontList );

  updateDimensions ();

  numMac = 0;
  mac = NULL;
  exp = NULL;

  return stat;

}

int menuMuxPVClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, i, ii, index;
int major, minor, release;
unsigned int pixel;
char oneName[PV_Factory::MAX_PV_NAME + 1];

  this->actWin = _actWin;

  fscanf ( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine ();

  if ( major > MMUXC_MAJOR_VERSION )
  {
    postIncompatable ();
    return 0;
  }

  fscanf ( f, "%d\n", &x ); actWin->incLine ();
  fscanf ( f, "%d\n", &y ); actWin->incLine ();
  fscanf ( f, "%d\n", &w ); actWin->incLine ();
  fscanf ( f, "%d\n", &h ); actWin->incLine ();

  this->initSelectBox (); // call after getting x, y, w, h

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) )
  {

    actWin->ci->readColorIndex ( f, &index );
    actWin->incLine (); actWin->incLine ();
    fgColour.setColorIndex ( index, actWin->ci );

    fscanf ( f, "%d\n", &fgColourMode ); actWin->incLine ();

    if ( fgColourMode == MMUXC_K_COLOURMODE_ALARM )
      fgColour.setAlarmSensitive ();
    else
      fgColour.setAlarmInsensitive ();

    actWin->ci->readColorIndex ( f, &index );
    actWin->incLine (); actWin->incLine ();
    bgColour.setColorIndex ( index, actWin->ci );

    fscanf ( f, "%d\n", &bgColourMode ); actWin->incLine ();

    if ( bgColourMode == MMUXC_K_COLOURMODE_ALARM )
      bgColour.setAlarmSensitive ();
    else
      bgColour.setAlarmInsensitive ();

    actWin->ci->readColorIndex ( f, &index );
    actWin->incLine (); actWin->incLine ();
    topShadowColour = index;

    actWin->ci->readColorIndex ( f, &index );
    actWin->incLine (); actWin->incLine ();
    botShadowColour = index;

  }
  else if ( major > 1 )
  {

    fscanf ( f, "%d\n", &index ); actWin->incLine ();
    fgColour.setColorIndex ( index, actWin->ci );

    fscanf ( f, "%d\n", &fgColourMode ); actWin->incLine ();

    if ( fgColourMode == MMUXC_K_COLOURMODE_ALARM )
      fgColour.setAlarmSensitive ();
    else
      fgColour.setAlarmInsensitive ();

    fscanf ( f, "%d\n", &index ); actWin->incLine ();
    bgColour.setColorIndex ( index, actWin->ci );

    fscanf ( f, "%d\n", &bgColourMode ); actWin->incLine ();

    if ( bgColourMode == MMUXC_K_COLOURMODE_ALARM )
      bgColour.setAlarmSensitive ();
    else
      bgColour.setAlarmInsensitive ();

    fscanf ( f, "%d\n", &index ); actWin->incLine ();
    topShadowColour = index;

    fscanf ( f, "%d\n", &index ); actWin->incLine ();
    botShadowColour = index;

  }
  else
  {

    fscanf ( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine ();
    if ( ( major < 2 ) && ( minor < 1 ) )
    {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB ( r, g, b, &pixel );
    index = actWin->ci->pixIndex ( pixel );
    fgColour.setColorIndex ( index, actWin->ci );

    fscanf ( f, "%d\n", &fgColourMode ); actWin->incLine ();

    if ( fgColourMode == MMUXC_K_COLOURMODE_ALARM )
      fgColour.setAlarmSensitive ();
    else
      fgColour.setAlarmInsensitive ();

    fscanf ( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine ();
    if ( ( major < 2 ) && ( minor < 1 ) )
    {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB ( r, g, b, &pixel );
    index = actWin->ci->pixIndex ( pixel );
    bgColour.setColorIndex ( index, actWin->ci );

    fscanf ( f, "%d\n", &bgColourMode ); actWin->incLine ();

    if ( bgColourMode == MMUXC_K_COLOURMODE_ALARM )
      bgColour.setAlarmSensitive ();
    else
      bgColour.setAlarmInsensitive ();

    fscanf ( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine ();
    if ( ( major < 2 ) && ( minor < 1 ) )
    {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB ( r, g, b, &pixel );
    topShadowColour = actWin->ci->pixIndex ( pixel );

    fscanf ( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine ();
    if ( ( major < 2 ) && ( minor < 1 ) )
    {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB ( r, g, b, &pixel );
    botShadowColour = actWin->ci->pixIndex ( pixel );

  }

  readStringFromFile ( oneName, PV_Factory::MAX_PV_NAME + 1, f );
   actWin->incLine ();
  controlPvExpStr.setRaw ( oneName );

  readStringFromFile ( fontTag, 63 + 1, f ); actWin->incLine ();

  actWin->fi->loadFontTag ( fontTag );
  actWin->drawGc.setFontTag ( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct ( fontTag );
  actWin->fi->getTextFontList ( fontTag, &fontList );

  updateDimensions ();

  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    strcpy ( tag[i], "" );
  }
  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    for ( ii = 0; ii < MMUX_MAX_ENTRIES; ii++ )
    {
      strcpy ( macroStrings[i][ii], "" );
      strcpy ( expansionPVNames[i][ii], "" );
      strcpy ( expansionStrings[i][ii], "" );
    }
  }

  fscanf ( f, "%d\n", &numItems ); actWin->incLine ();

  for ( i = 0; i < numItems; i++ )
  {
    readStringFromFile ( tag[i], MMUX_MAX_STRING_SIZE + 1, f );
    actWin->incLine ();
  }

  for ( i = 0; i < numItems; i++ )
  {
    for ( ii = 0; ii < MMUX_MAX_ENTRIES; ii++ )
    {
      readStringFromFile ( macroStrings[i][ii], MMUX_MAX_STRING_SIZE + 1, f );
      actWin->incLine ();
      readStringFromFile ( expansionPVNames[i][ii], MMUX_MAX_STRING_SIZE + 1, f );
      actWin->incLine ();
      strcpy (expansionStrings[i][ii], "");
    }
  }

  if ( ( major > 1 ) || ( minor > 1 ) )
  {
    readStringFromFile ( oneName, 39 + 1, f ); actWin->incLine ();
    initialStateExpStr.setRaw ( oneName );
  }
  else 
  {
    initialStateExpStr.setRaw ( "0" );
  }

  numMac = 0;
  mac = NULL;
  exp = NULL;

  return 1;

}

int menuMuxPVClass::genericEdit ( void )
{

int i, ii;
char title[32], *ptr;

  if ( !eBuf )
  {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass ( "menuMuxPVClass" );
  if ( ptr )
    strncpy ( title, ptr, 31 + 1 );
  else
    strncpy ( title, menuMuxPVClass_str2, 31 + 1 );

  Strncat ( title, menuMuxPVClass_str3, 31 + 1 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufTopShadowColour = topShadowColour;
  eBuf->bufBotShadowColour = botShadowColour;

  eBuf->bufFgColour = fgColour.pixelIndex ();
  eBuf->bufFgColourMode = fgColourMode;

  eBuf->bufBgColour = bgColour.pixelIndex ();
  eBuf->bufBgColourMode = bgColourMode;

  if ( controlPvExpStr.getRaw () )
    strncpy ( eBuf->bufControlPvName, controlPvExpStr.getRaw (),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy ( eBuf->bufControlPvName, "" );

  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    strncpy ( eBuf->bufTag[i], tag[i], MMUX_MAX_STRING_SIZE + 1 );
  }
  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    for ( ii = 0; ii < MMUX_MAX_ENTRIES; ii++ )
    {
      strncpy ( eBuf->bufM[i][ii], macroStrings[i][ii],
                MMUX_MAX_STRING_SIZE + 1 );
      strncpy ( eBuf->bufE[i][ii], expansionPVNames[i][ii],
                MMUX_MAX_STRING_SIZE + 1 );
    }
  }

  if ( initialStateExpStr.getRaw () )
    strncpy ( eBuf->bufInitialState, initialStateExpStr.getRaw (), 15 + 1 );
  else
    strncpy ( eBuf->bufInitialState, "0", 15 + 1 );

  ef.create ( actWin->top, actWin->appCtx->ci.getColorMap (),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, MMUX_MAX_STATES, numItems,
   mmuxSetItem, (void *) this, NULL, NULL, NULL );

  ef.addTextField ( menuMuxPVClass_str4, 35, &eBuf->bufX );
  ef.addTextField ( menuMuxPVClass_str5, 35, &eBuf->bufY );
  ef.addTextField ( menuMuxPVClass_str6, 35, &eBuf->bufW );
  ef.addTextField ( menuMuxPVClass_str7, 35, &eBuf->bufH );
  ef.addTextField ( menuMuxPVClass_str17, 35, eBuf->bufControlPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField ( menuMuxPVClass_str18, 35, eBuf->bufInitialState, 30 );

  ef.addColorButton ( menuMuxPVClass_str8, actWin->ci, &eBuf->fgCb,
                      &eBuf->bufFgColour );
  ef.addToggle ( menuMuxPVClass_str10, &eBuf->bufFgColourMode );
  ef.addColorButton ( menuMuxPVClass_str11, actWin->ci, &eBuf->bgCb,
                      &eBuf->bufBgColour );
  ef.addColorButton ( menuMuxPVClass_str14, actWin->ci, &eBuf->topShadowCb,
                      &eBuf->bufTopShadowColour );
  ef.addColorButton ( menuMuxPVClass_str15, actWin->ci, &eBuf->botShadowCb,
                      &eBuf->bufBotShadowColour );

  ef.addFontMenu ( menuMuxPVClass_str16, actWin->fi, &fm, fontTag );

  XtUnmanageChild ( fm.alignWidget () ); // no alignment info

  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    tagPtr[i] = (char *) &eBuf->bufTag[i];
  }

  for ( i = 0; i < MMUX_MAX_STATES; i++ )
  {
    for ( ii = 0; ii < MMUX_MAX_ENTRIES; ii++ )
    {
      mPtr[ii][i] = (char *) &eBuf->bufM[i][ii];
      ePtr[ii][i] = (char *) &eBuf->bufE[i][ii];
    }
  }

  ef.addTextFieldArray ( menuMuxPVClass_str19, 35, tagPtr,
                         MMUX_MAX_STRING_SIZE, &elbt );

  for ( i = 0; i < MMUX_MAX_ENTRIES; i++ )
  {
    ef.addTextFieldArray ( menuMuxPVClass_str20, 35, mPtr[i],
                           MMUX_MAX_STRING_SIZE, &elbm[i] );
    ef.addTextFieldArray ( menuMuxPVClass_str21, 35, ePtr[i],
                           MMUX_MAX_STRING_SIZE, &elbe[i] );
  }

  return 1;

}

int menuMuxPVClass::editCreate ( void )
{

  this->genericEdit ();
  ef.finished ( mmuxc_edit_ok, mmuxc_edit_apply, mmuxc_edit_cancel_delete,
   this );
  actWin->currentEf = NULL;
  ef.popup ();

  return 1;

}

int menuMuxPVClass::edit ( void )
{

  this->genericEdit ();
  ef.finished ( mmuxc_edit_ok, mmuxc_edit_apply, mmuxc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup ();

  return 1;

}

int menuMuxPVClass::erase ( void )
{

  if ( deleteRequest || activeMode ) return 1;

  XDrawRectangle ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.eraseGC (), x, y, w, h );

  XFillRectangle ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.eraseGC (), x, y, w, h );

  return 1;

}

int menuMuxPVClass::eraseActive ( void )
{

  if ( !activeMode ) return 1;

  XDrawRectangle ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.eraseGC (), x, y, w, h );

  XFillRectangle ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.eraseGC (), x, y, w, h );

  return 1;

}

int menuMuxPVClass::draw ( void )
{

int tX, tY, bumpX, bumpY;
XRectangle xR = { x + 3, y, w - 23, h };
int blink = 0;

  if ( deleteRequest || activeMode ) return 1;

  actWin->drawGc.saveFg ();

  actWin->drawGc.setFG ( bgColour.pixelIndex (), &blink );

  actWin->drawGc.setLineStyle ( LineSolid );
  actWin->drawGc.setLineWidth ( 1 );

  XFillRectangle ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x, y, w, h );

  XDrawRectangle ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x, y, w, h );

  actWin->drawGc.setFG ( actWin->ci->pix (botShadowColour) );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x, y, x + w, y );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x, y, x, y + h );

   actWin->drawGc.setFG ( actWin->ci->pix (topShadowColour) );

   XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
    actWin->drawGc.normGC (), x, y + h, x + w, y + h );

   XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
    actWin->drawGc.normGC (), x + w, y, x + w, y + h );

  actWin->drawGc.setFG ( actWin->ci->pix (topShadowColour) );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x + 1, y + 1, x + w - 1, y + 1 );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x + 2, y + 2, x + w - 2, y + 2 );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x + 1, y + 1, x + 1, y + h - 1 );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x + 2, y + 2, x + 2, y + h - 2 );

  actWin->drawGc.setFG ( actWin->ci->pix (botShadowColour) );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x + 1, y + h - 1, x + w - 1, y + h - 1 );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x + 2, y + h - 2, x + w - 2, y + h - 2 );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x + w - 1, y + 1, x + w - 1, y + h - 1 );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x + w - 2, y + 2, x + w - 2, y + h - 2 );

  // draw bump

  bumpX = x + w - 10 - 10;
  bumpY = y + h / 2 - 5;

  actWin->drawGc.setFG ( actWin->ci->pix (topShadowColour) );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), bumpX, bumpY + 10, bumpX, bumpY );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), bumpX, bumpY, bumpX + 10, bumpY );

  actWin->drawGc.setFG ( actWin->ci->pix (botShadowColour) );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), bumpX + 10, bumpY, bumpX + 10, bumpY + 10 );

  XDrawLine ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), bumpX + 10, bumpY + 10, bumpX, bumpY + 10 );

  if ( fs ) 
  {

    actWin->drawGc.addNormXClipRectangle ( xR );

    actWin->drawGc.setFG ( fgColour.pixelIndex (), &blink );
    actWin->drawGc.setFontTag ( fontTag, actWin->fi );

    tX = x + w / 2 - 10;
    tY = y + h / 2 - fontAscent / 2;

    drawText ( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
     XmALIGNMENT_CENTER, "Mux" );

    actWin->drawGc.removeNormXClipRectangle ();

  }

  actWin->drawGc.restoreFg ();

  updateBlink ( blink );

  return 1;

}

int menuMuxPVClass::drawActive ( void )
{

int tX, tY, bumpX, bumpY;
XRectangle xR = { x + 3, y, w - 23, h };
int blink = 0;
char string[MMUX_MAX_STRING_SIZE + 1];

  if ( !controlPvConnected )
  {
    if ( controlExists )
    {
      if ( needToDrawUnconnected )
      {
        actWin->executeGc.saveFg ();
        actWin->executeGc.setFG ( bgColour.getDisconnectedIndex (), &blink );
        actWin->executeGc.setLineWidth ( 1 );
        actWin->executeGc.setLineStyle ( LineSolid );
        XDrawRectangle ( actWin->d, XtWindow (actWin->executeWidget),
         actWin->executeGc.normGC (), x, y, w, h );
        actWin->executeGc.restoreFg ();
        needToEraseUnconnected = 1;
        updateBlink ( blink );
      }
    }
    else if ( needToEraseUnconnected )
    {
      actWin->executeGc.setLineWidth ( 1 );
      actWin->executeGc.setLineStyle ( LineSolid );
      XDrawRectangle ( actWin->d, XtWindow (actWin->executeWidget),
       actWin->executeGc.eraseGC (), x, y, w, h );
      needToEraseUnconnected = 0;
      eraseActive ();
      smartDrawAllActive ();
    }
  }

  if ( !activeMode || !widgetsCreated ) return 1;

  actWin->executeGc.saveFg ();
  actWin->executeGc.setLineWidth ( 1 );
  actWin->executeGc.setLineStyle ( LineSolid );
  actWin->executeGc.setFG ( bgColour.getIndex (), &blink );

  XFillRectangle ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x, y, w, h );

  XDrawRectangle ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x, y, w, h );

  actWin->executeGc.setFG ( actWin->ci->pix (botShadowColour) );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x, y, x + w, y );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x, y, x, y + h );

  actWin->executeGc.setFG ( actWin->ci->pix (topShadowColour) );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x, y + h, x + w, y + h );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x + w, y, x + w, y + h );

  // top
  actWin->executeGc.setFG ( actWin->ci->pix (topShadowColour) );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x + 1, y + 1, x + w - 1, y + 1 );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x + 2, y + 2, x + w - 2, y + 2 );

  // left
  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x + 1, y + 1, x + 1, y + h - 1 );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x + 2, y + 2, x + 2, y + h - 2 );

  // bottom
  actWin->executeGc.setFG ( actWin->ci->pix (botShadowColour) );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x + 1, y + h - 1, x + w - 1, y + h - 1 );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x + 2, y + h - 2, x + w - 2, y + h - 2 );

  // right
  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x + w - 1, y + 1, x + w - 1, y + h - 1 );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), x + w - 2, y + 2, x + w - 2, y + h - 2 );

  // draw bump

  bumpX = x + w - 10 - 10;
  bumpY = y + h / 2 - 5;

  actWin->executeGc.setFG ( actWin->ci->pix (topShadowColour) );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), bumpX, bumpY + 10, bumpX, bumpY );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), bumpX, bumpY, bumpX + 10, bumpY );

  actWin->executeGc.setFG ( actWin->ci->pix (botShadowColour) );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), bumpX + 10, bumpY, bumpX + 10, bumpY + 10 );

  XDrawLine ( actWin->d, XtWindow (actWin->executeWidget),
   actWin->executeGc.normGC (), bumpX + 10, bumpY + 10, bumpX, bumpY + 10 );

  if ( fs )
  {

    actWin->executeGc.addNormXClipRectangle ( xR );

    actWin->executeGc.setFG ( fgColour.getIndex (), &blink );
    actWin->executeGc.setFontTag ( fontTag, actWin->fi );

    tX = x + w / 2 - 10;
    tY = y + h / 2 - fontAscent / 2;

    if ( ( controlV >= 0 ) && ( controlV < numStates ) )
    {
      strncpy ( string, tag[controlV], MMUX_MAX_STRING_SIZE );
    }
    else
    {
      strcpy ( string, "?" );
    }

    drawText ( actWin->executeWidget, &actWin->executeGc, fs, tX, tY,
     XmALIGNMENT_CENTER, string );

    actWin->executeGc.removeNormXClipRectangle ();

  }

  actWin->executeGc.restoreFg ();

  updateBlink ( blink );

  return 1;

}

int menuMuxPVClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;
  
  stat = controlPvExpStr.expand1st ( numMacros, macros, expansions );
  stat = initialStateExpStr.expand1st ( numMacros, macros, expansions );

  for (int n = 0; n < numItems; n++)
  {
    for (int i = 0; i <MMUX_MAX_ENTRIES; i++)
    {
      if ( ( strcmp ( macroStrings[n][i], "" ) != 0 ) &&
           ( strcmp ( expansionPVNames[n][i], "" ) != 0 ) ) 
      {
          expansionPvExpStr[n][i].setRaw (expansionPVNames[n][i]);
          expansionPvExpStr[n][i].expand1st (numMacros, macros, expansions);
      }
    }
  }
  return stat;

}

int menuMuxPVClass::getMacros (
  int *numMacros,
  char ***macro,
  char ***expansion )
{

int i, ii, n, count;

#ifdef DEBUG
  printf ("Start of menuMuxPVClass::getMacros\n");
#endif

  if ( controlV < 0 )
    n = 0;
  else if ( controlV >= numItems )
    n = numItems - 1;
  else
    n = controlV;

// count number of non-null entries
  count = 0;
  for ( i = 0; i < MMUX_MAX_ENTRIES; i++ )
  {
    if ( ( strcmp ( macroStrings[n][i], "" ) != 0 ) &&
         ( strcmp ( expansionPVNames[n][i], "" ) != 0 ) )
    {
      count++;
    }
  }

  if ( numMac < count )
  {

    for ( i = 0; i < numMac; i++ )
    {
      if ( mac[i] )
      {
        delete[] mac[i];
        mac[i] = NULL;
      }
      if ( exp[i] )
      {
        delete[] exp[i];
        exp[i] = NULL;
      }
    }
    if ( mac )
    {
      delete[] mac;
      mac = NULL;
    }
    if ( exp )
    {
      delete[] exp;
      exp = NULL;
    }

    numMac = count;

    mac = new char*[numMac];
    exp = new char*[numMac];

    for ( i = 0; i < numMac; i++ )
    {
      mac[i] = new char[MMUX_MAX_STRING_SIZE + 1];
      exp[i] = new char[MMUX_MAX_STRING_SIZE + 1];
    }

  }

  // populate ptr arrays
  ii = 0;
  for ( i = 0; i < MMUX_MAX_ENTRIES; i++ )
  {
    if ( ( strcmp ( macroStrings[n][i], "" ) != 0 ) &&
         ( strcmp ( expansionPVNames[n][i], "" ) != 0 ) ) 
    {
      strncpy ( mac[ii], macroStrings[n][i], MMUX_MAX_STRING_SIZE + 1 );
      strncpy ( exp[ii], expansionStrings[n][i], MMUX_MAX_STRING_SIZE + 1 );
#ifdef DEBUG
      printf ("menuMuxPVClass::getMacros - macro %s = %s\n", mac[ii], exp[ii]);
#endif
      ii++;
    }
  }

  *numMacros = count;
  *macro = mac;
  *expansion = exp;

  return 1;

}

int menuMuxPVClass::activate (
  int pass,
  void *ptr )
{

int opStat;

  switch ( pass ) 
  {

  case 1:

    opComplete = 0;
    controlPvNotConnectedMask = 0;
    for (int i = 0; i < MMUX_MAX_ENTRIES; i++)
      expansionPvNotConnectedMask[i] = 0;

    break;

  case 2:

    if ( !opComplete )
    {

      aglPtr = ptr;
      needConnectInit = needDisconnect = needInfoInit = needUpdate =
       needDraw = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;
      widgetsCreated = 0;
      firstEvent = 1;
      controlV = 0;
      buttonPressed = 0;
      initialConnection = 1;
      oldStat = oldSev = -1;

      controlPvConnected = active = 0;
      activeMode = 1;
      controlPvId = NULL;

      popUpMenu = (Widget) NULL;

      // if ( strcmp ( controlPvExpStr.getExpanded (), "" ) != 0 )
      if ( !blankOrComment ( controlPvExpStr.getExpanded () ) )
        controlExists = 1;
      else
        controlExists = 0;

      if ( !unconnectedTimer )
      {
        unconnectedTimer = appAddTimeOut ( actWin->appCtx->appContext (),
         2000, unconnectedTimeout, this );
      }

      opStat = 1;

      if ( controlExists )
      {

        controlPvId = the_PV_Factory->create ( controlPvExpStr.getExpanded () );
        if ( controlPvId )
        {
          controlPvNotConnectedMask |= 1;
          controlPvId->add_conn_state_callback (
           mmux_monitor_control_connect_state, this );
        }
        else
        {
          printf ( menuMuxPVClass_str23 );
          opStat = 0;
        }

      }
      else
      {

        actWin->appCtx->proc->lock ();
        if ( initialStateExpStr.getExpanded () )
        {
          curControlV = atol ( initialStateExpStr.getExpanded () );
        }
        else
        {
          curControlV = 0;
        }
        needInfoInit = 1;
        actWin->addDefExeNode ( aglPtr );
        actWin->appCtx->proc->unlock ();

      }

      for (int n = 0; n < numItems; n++)
      {
        for (int i = 0; i <MMUX_MAX_ENTRIES; i++)
        {
          if ( ( strcmp ( macroStrings[n][i], "" ) != 0 ) &&
               ( strcmp ( expansionPVNames[n][i], "" ) != 0 ) ) 
          {
              expansionPVs[n][i] = the_PV_Factory->create (
                                     expansionPvExpStr[n][i].getExpanded () );
              expansionPvNotConnectedMask[i] |= 1 << n;
              expansionPVs[n][i]->add_conn_state_callback (
                                     mmux_monitor_expansion_connect_state, this );
          }
        }
      }
      if ( !( opStat & 1 ) ) opComplete = 1;

      return opStat;

    }

    break;

  case 3:
  case 4:
  case 5:
  case 6:

    break;

  }

  return 1;

}

int menuMuxPVClass::deactivate (
  int pass
) 
{

int i;

  active = 0;
  activeMode = 0;

  if ( pass == 1 )
  {

    if ( unconnectedTimer )
    {
      XtRemoveTimeOut ( unconnectedTimer );
      unconnectedTimer = 0;
    }

    //updateBlink ( 0 );

    if ( controlExists )
    {
      if ( controlPvId )
      {
        controlPvId->remove_conn_state_callback (
         mmux_monitor_control_connect_state, this );
        controlPvId->remove_value_callback (
         mmux_controlUpdate, this );
        controlPvId->release ();
        controlPvId = NULL;
      }
    }
    for (int n = 0; n < numItems; n++)
    {
      for (int i = 0; i <MMUX_MAX_ENTRIES; i++)
      {
        if ( ( strcmp ( macroStrings[n][i], "" ) != 0 ) &&
             ( strcmp ( expansionPVNames[n][i], "" ) != 0 ) ) 
        {
          expansionPVs[n][i]->remove_conn_state_callback (
                                   mmux_monitor_expansion_connect_state,
                                   this );
          expansionPVs[n][i]->remove_value_callback (mmux_expansionUpdate,
                                                     this);
          expansionPVs[n][i]->release ();
          expansionPVs[n][i] = NULL;
        }
      }
    }
  }
  else if ( pass == 2 )
  {

    if ( widgetsCreated )
    {
      for ( i = 0; i < numStates; i++ )
      {
        XtDestroyWidget ( pb[i] );
      }
      XtDestroyWidget ( pullDownMenu );
      XtDestroyWidget ( popUpMenu );
      widgetsCreated = 0;
    }

  }

  return 1;

}

void menuMuxPVClass::updateDimensions ( void )
{

  if ( fs )
  {
    fontAscent = fs->ascent;
    fontDescent = fs->descent;
    fontHeight = fontAscent + fontDescent;
  }
  else
  {
    fontAscent = 10;
    fontDescent = 5;
    fontHeight = fontAscent + fontDescent;
  }

}

void menuMuxPVClass::btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled ) return;

  if ( !buttonPressed ) return;

  buttonPressed = 0;

  if ( buttonNumber == 1 )
  {

    XmMenuPosition ( popUpMenu, be );
    XtManageChild ( popUpMenu );

  }

}

void menuMuxPVClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled ) return;

  if ( controlExists )
  {
    if ( !controlPvId->have_write_access () ) return;
  }

  if ( buttonNumber == 1 )
  {
    buttonPressed = 1;
  }

}

void menuMuxPVClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled ) return;

  if ( controlExists )
  {
    if ( !controlPvId->have_write_access () )
    {
      actWin->cursor.set ( XtWindow (actWin->executeWidget), CURSOR_K_NO );
    }
    else
    {
      actWin->cursor.set ( XtWindow (actWin->executeWidget), CURSOR_K_DEFAULT );
    }
  }

  activeGraphicClass::pointerIn ( _x, _y, buttonState );

}

int menuMuxPVClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;
  *focus = 1;
  *down = 1;
  *up = 1;

  return 1;

}

#if 0
static void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

class menuMuxPVClass *mmo;
int stat;

  XtVaGetValues ( w, XmNuserData, &mmo, NULL );

  stat = mmo->startDrag ( w, e );

}

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

class menuMuxPVClass *mmo;
int stat;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues ( w, XmNuserData, &mmo, NULL );

  stat = mmo->selectDragValue ( be );

}
#endif

void menuMuxPVClass::executeDeferred ( void )
{

int v;
int stat, i, nc, ndis, ni, nu, nd;
XmString str;
Arg args[15];
int n;
#ifdef DEBUG
  printf ("Start of menuMuxPVClass::executeDeferred\n");
#endif
//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock ();
  nc = needConnectInit; needConnectInit = 0;
  ndis = needDisconnect; needDisconnect = 0;
  ni = needInfoInit; needInfoInit = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  v = curControlV;
  actWin->remDefExeNode ( aglPtr );
  actWin->appCtx->proc->unlock ();

  if ( !activeMode )
  {
#ifdef DEBUG
    printf ("menuMuxPVClass::executeDeferred - !activeMode\n");
#endif
    return;
  }

//----------------------------------------------------------------------------

  if ( nc )
  {
#ifdef DEBUG
    printf ("menuMuxPVClass::executeDeferred - start of nc block\n");
#endif
    if (controlExists)
    {
      v = curControlV = controlPvId->get_int ();
#ifdef DEBUG
      printf ("menuMuxPVClass::executeDeferred - after controlPvId->get_int\n");
#endif
      controlPvConnected = 1;
    }
    fgColour.setConnected ();

    ni = 1;
#ifdef DEBUG
    printf ("menuMuxPVClass::executeDeferred - end of nc block\n");
#endif
  }

//----------------------------------------------------------------------------

  if ( ndis )
  {
#ifdef DEBUG
    printf ("menuMuxPVClass::executeDeferred - start of ndis block\n");
#endif
    controlPvConnected = 0;
    fgColour.setDisconnected ();
    active = 0;

    if ( widgetsCreated )
    {

      for ( i = 0; i < numStates; i++ )
      {
        XtDestroyWidget ( pb[i] );
      }
      XtDestroyWidget ( pullDownMenu );
      XtDestroyWidget ( popUpMenu );

      widgetsCreated = 0;

    }

  }

//----------------------------------------------------------------------------

  if ( ni )
  {
#ifdef DEBUG
    printf ("menuMuxPVClass::executeDeferred - start of ni block\n");
#endif
    controlV = v;

    if ( widgetsCreated )
    {

      for ( i = 0; i < numStates; i++ )
      {
        XtDestroyWidget ( pb[i] );
      }
      XtDestroyWidget ( pullDownMenu );
      XtDestroyWidget ( popUpMenu );

      widgetsCreated = 0;

    }

    n = 0;
    XtSetArg ( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
    popUpMenu = XmCreatePopupMenu ( actWin->topWidgetId (), "", args, n );

    pullDownMenu = XmCreatePulldownMenu ( popUpMenu, "", NULL, 0 );

    numStates = numItems;

    for ( i = 0; i < numStates; i++ )
    {

      stateString[i] = new char[strlen (tag[i]) + 1];
      strncpy ( stateString[i], tag[i], strlen (tag[i]) + 1 );

      str = XmStringCreate ( stateString[i], fontTag );

      pb[i] = XtVaCreateManagedWidget ( "", xmPushButtonWidgetClass,
       popUpMenu,
       XmNlabelString, str,
       NULL );

      XmStringFree ( str );

      if ( controlExists )
      {
        XtAddCallback ( pb[i], XmNactivateCallback, mmux_putValue,
         (XtPointer) this );
      }
      else
      {
        XtAddCallback ( pb[i], XmNactivateCallback, mmux_putValueNoPv,
         (XtPointer) this );
      }

    }

    widgetsCreated = 1;

    active = 1;

    if ( controlExists )
    {

      if ( initialConnection )
      {

        initialConnection = 0;

        controlPvId->add_value_callback ( mmux_controlUpdate, this );

      }

    }
    else
    {

      firstEvent = 0;

    }

    for (int n = 0; n < numItems; n++)
    {
      for (int i = 0; i <MMUX_MAX_ENTRIES; i++)
      {
        if ( ( strcmp ( macroStrings[n][i], "" ) != 0 ) &&
             ( strcmp ( expansionPVNames[n][i], "" ) != 0 ) ) 
        {
#ifdef DEBUG
          printf ("menuMuxPVClass::execDef - Call add_val_callbk for PV %s\n",
                  expansionPVNames[n][i]);
#endif
          expansionPVs[n][i]->add_value_callback (
                                     mmux_expansionUpdate, this );
        }
      }
    }

    nu = 1;

  }

//----------------------------------------------------------------------------

  if ( nu )
  {
#ifdef DEBUG
    printf ("menuMuxPVClass::executeDeferred - start of nu block\n");
#endif
    controlV = v;
    stat = drawActive ();

    if ( !firstEvent )
    {
      actWin->preReexecute ();
      actWin->setNoRefresh ();
      actWin->appCtx->reactivateActiveWindow ( actWin );
    }
    firstEvent = 0;

  }

//----------------------------------------------------------------------------

  if ( nd )
  {
#ifdef DEBUG
    printf ("menuMuxPVClass::executeDeferred - start of nd block\n");
#endif
    controlV = v;
    drawActive ();
  }

//----------------------------------------------------------------------------
#ifdef DEBUG
  printf ("End of menuMuxPVClass::executeDeferred\n");
#endif
}

char *menuMuxPVClass::firstDragName ( void )
{

  dragIndex = 0;
  return dragName[dragIndex];

}

char *menuMuxPVClass::nextDragName ( void )
{

  return NULL;

}

char *menuMuxPVClass::dragValue (
  int i )
{

  if ( actWin->mode != AWC_EXECUTE )
  {

    return controlPvExpStr.getExpanded ();

  }
  else
  {

    return controlPvExpStr.getRaw ();

  }

}

void menuMuxPVClass::changeDisplayParams (
  unsigned int _flag,
  char *_fontTag,
  int _alignment,
  char *_ctlFontTag,
  int _ctlAlignment,
  char *_btnFontTag,
  int _btnAlignment,
  int _textFgColour,
  int _fg1Colour,
  int _fg2Colour,
  int _offsetColour,
  int _bgColour,
  int _topShadowColour,
  int _botShadowColour )
{

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColour.setColorIndex ( _textFgColour, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColour.setColorIndex ( _bgColour, actWin->ci );

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topShadowColour = _topShadowColour;

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botShadowColour = _botShadowColour;

  if ( _flag & ACTGRF_BTNFONTTAG_MASK )
  {

    strcpy ( fontTag, _btnFontTag );
    actWin->fi->loadFontTag ( fontTag );
    fs = actWin->fi->getXFontStruct ( fontTag );
    actWin->fi->getTextFontList ( fontTag, &fontList );

    updateDimensions ();

  }

}

void menuMuxPVClass::changePvNames (
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
  char *alarmPvs[] )
{

  if ( flag & ACTGRF_CTLPVS_MASK )
  {
    if ( numCtlPvs )
    {
      controlPvExpStr.setRaw ( ctlPvs[0] );
    }
  }

}

void menuMuxPVClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n )
{

  if ( max < 1 )
  {
    *n = 0;
    return;
  }

  *n = 1;
  pvs[0] = controlPvId;

}

#ifdef __cplusplus
extern "C"
{
#endif

void *create_menuMuxPVClassPtr ( void )
{

menuMuxPVClass *ptr;

  ptr = new menuMuxPVClass;
  return (void *) ptr;

}

void *clone_menuMuxPVClassPtr (
  void *_srcPtr )
{

menuMuxPVClass *ptr, *srcPtr;

  srcPtr = (menuMuxPVClass *) _srcPtr;

  ptr = new menuMuxPVClass ( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
