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

#define __vsbar_cc 1

#include "vsbar.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

  activeVsBarClass *baro = (activeVsBarClass *) client;

  if ( !baro->init )
  {
    baro->needToDrawUnconnected = 1;
    baro->needRefresh = 1;
    baro->actWin->addDefExeNode( baro->aglPtr );
  }

  baro->unconnectedTimer = 0;

}

static void barc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  activeVsBarClass *baro = (activeVsBarClass *) client;
  char fmt[31 + 1], str[31 + 1];
  int l;

  baro->actWin->setChanged();

  baro->eraseSelectBoxCorners();
  baro->erase();

  baro->fgColourMode = baro->bufFgColourMode;
  if ( baro->fgColourMode == BARC_K_COLORMODE_ALARM )
    baro->fgColour.setAlarmSensitive();
  else
    baro->fgColour.setAlarmInsensitive();
  baro->fgColour.setColorIndex( baro->bufFgColour, baro->actWin->ci );

  baro->barColourMode = baro->bufBarColourMode;
  if ( baro->barColourMode == BARC_K_COLORMODE_ALARM )
    baro->barColour.setAlarmSensitive();
  else
    baro->barColour.setAlarmInsensitive();
  baro->barColour.setColorIndex( baro->bufBarColour, baro->actWin->ci );

  baro->bgColour.setColorIndex( baro->bufBgColour, baro->actWin->ci );

  baro->readPvExpStr.setRaw( baro->eBuf->bufReadPvName );
  baro->nullPvExpStr.setRaw( baro->eBuf->bufNullPvName );
  baro->maxPvExpStr.setRaw( baro->eBuf->bufMaxPvName );
  baro->minPvExpStr.setRaw( baro->eBuf->bufMinPvName );

  strncpy( baro->label, baro->bufLabel, 39 );

  baro->labelType = baro->bufLabelType;

  strncpy( baro->fontTag, baro->fm.currentFontTag(), 63 );
  baro->actWin->fi->loadFontTag( baro->fontTag );
  baro->fs = baro->actWin->fi->getXFontStruct( baro->fontTag );
  baro->actWin->drawGc.setFontTag( baro->fontTag, baro->actWin->fi );

  if ( baro->fs )
  {
    baro->barStrLen = XTextWidth( baro->fs, "10", 2 );
  }

  baro->border = baro->bufBorder;

  strncpy( baro->scaleFormat, baro->bufScaleFormat, 15 );
  baro->showScale = baro->bufShowScale;
  baro->labelTicks = baro->bufLabelTicks;
  baro->majorTicks = baro->bufMajorTicks;
  baro->minorTicks = baro->bufMinorTicks;

  baro->x = baro->bufX;
  baro->sboxX = baro->bufX;

  baro->y = baro->bufY;
  baro->sboxY = baro->bufY;

  baro->w = baro->bufW;
  baro->sboxW = baro->bufW;

  baro->h = baro->bufH;
  baro->sboxH = baro->bufH;

  baro->horizontal = baro->bufHorizontal;

  baro->limitsFromDb = baro->bufLimitsFromDb;
  baro->limitsFromPVs = baro->bufLimitsFromPVs;
  baro->efPrecision = baro->bufEfPrecision;
  baro->efReadMin = baro->bufEfReadMin;
  baro->efReadMax = baro->bufEfReadMax;
  baro->efBarOriginX = baro->bufEfBarOriginX;

  if ( baro->efPrecision.isNull() )
    baro->precision = 0;
  else
    baro->precision = baro->efPrecision.value();

  if ( strcmp( baro->scaleFormat, "GFloat" ) == 0 )
  {
    sprintf( fmt, "%%.%-dg", baro->precision );
  }
  else if ( strcmp( baro->scaleFormat, "Exponential" ) == 0 )
  {
    sprintf( fmt, "%%.%-de", baro->precision );
  }
  else
  {
    sprintf( fmt, "%%.%-df", baro->precision );
  }

  if ( ( baro->efReadMin.isNull() ) && ( baro->efReadMax.isNull() ) )
  {
    baro->readMin = 0;
    baro->readMax = 10;
  }
  else
  {
    baro->readMin = baro->efReadMin.value();
    baro->readMax = baro->efReadMax.value();
  }

  if ( baro->efBarOriginX.isNull() )
  {
    baro->barOriginX = baro->readMin;
  }
  else
  {
    baro->barOriginX = baro->efBarOriginX.value();
  }

  sprintf( str, fmt, baro->readMin );
  if ( baro->fs )
  {
    baro->barStrLen = XTextWidth( baro->fs, str, strlen(str) );
  }
  sprintf( str, fmt, baro->readMax );
  if ( baro->fs )
  {
    l = XTextWidth( baro->fs, str, strlen(str) );
    if ( l > baro->barStrLen ) baro->barStrLen = l;
  }

  baro->updateDimensions();

  if ( baro->horizontal )
  {
    if ( baro->h < baro->minH )
    {
      baro->h = baro->minH;
      baro->sboxH = baro->minH;
    }
  }
  else
  {
    if ( baro->h < baro->minVertH )
    {
      baro->h = baro->minVertH;
      baro->sboxH = baro->minVertH;
    }
  }
}

static void barc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  activeVsBarClass *baro = (activeVsBarClass *) client;

  barc_edit_update ( w, client, call );
  baro->refresh( baro );

}

static void barc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  activeVsBarClass *baro = (activeVsBarClass *) client;

  barc_edit_update ( w, client, call );
  baro->ef.popdown();
  baro->operationComplete();

}

static void barc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  activeVsBarClass *baro = (activeVsBarClass *) client;

  baro->ef.popdown();
  baro->operationCancel();

}

static void barc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  activeVsBarClass *baro = (activeVsBarClass *) client;

  baro->ef.popdown();
  baro->operationCancel();
  baro->erase();
  baro->deleteRequest = 1;
  baro->drawAll();

}

static void bar_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{
  activeVsBarClass *baro = (activeVsBarClass *) userarg;

  baro->actWin->appCtx->proc->lock();

  if ( baro->activeMode )
  {
    if ( pv->is_valid() )
    {
      baro->pvNotConnectedMask &= ~( (unsigned char) 1 );
      if ( !baro->pvNotConnectedMask )
      { // if all are connected
        baro->needConnectInit = 1;
        baro->actWin->addDefExeNode( baro->aglPtr );
      }

    }
    else
    {

      baro->pvNotConnectedMask |= 1; // read pv not connected
      baro->active = 0;
      baro->barColour.setDisconnected();
      baro->fgColour.setDisconnected();
      baro->bufInvalidate();
      baro->needFullDraw = 1;
      baro->actWin->addDefExeNode( baro->aglPtr );

    }

  }

  baro->actWin->appCtx->proc->unlock();

}

static void bar_monitor_null_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

  activeVsBarClass *baro = (activeVsBarClass *) userarg;

  baro->actWin->appCtx->proc->lock();

  if ( baro->activeMode )
  {

    if ( pv->is_valid() )
    {

      baro->pvNotConnectedMask &= ~( (unsigned char) 2 );
      if ( !baro->pvNotConnectedMask )
      { // if all are connected
        baro->needConnectInit = 1;
        baro->actWin->addDefExeNode( baro->aglPtr );
      }

    }
    else
    {

      baro->pvNotConnectedMask |= 2; // null pv not connected
      baro->active = 0;
      baro->barColour.setDisconnected();
      baro->fgColour.setDisconnected();
      baro->bufInvalidate();
      baro->needDraw = 1;
      baro->actWin->addDefExeNode( baro->aglPtr );

    }

  }

  baro->actWin->appCtx->proc->unlock();

}

static void bar_monitor_max_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

  activeVsBarClass *baro = (activeVsBarClass *) userarg;

  baro->actWin->appCtx->proc->lock();

  if ( baro->activeMode )
  {

    if ( pv->is_valid() )
    {

      baro->pvNotConnectedMask &= ~( (unsigned char) 4 );
      if ( !baro->pvNotConnectedMask )
      { // if all are connected
        baro->needConnectInit = 1;
        baro->actWin->addDefExeNode( baro->aglPtr );
      }

    }
    else
    {

      baro->pvNotConnectedMask |= 4; // max pv not connected
      baro->active = 0;
      baro->barColour.setDisconnected();
      baro->fgColour.setDisconnected();
      baro->bufInvalidate();
      baro->needDraw = 1;
      baro->actWin->addDefExeNode( baro->aglPtr );

    }

  }

  baro->actWin->appCtx->proc->unlock();

}

static void bar_monitor_min_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

  activeVsBarClass *baro = (activeVsBarClass *) userarg;

  baro->actWin->appCtx->proc->lock();

  if ( baro->activeMode )
  {

    if ( pv->is_valid() )
    {

      baro->pvNotConnectedMask &= ~( (unsigned char) 8 );
      if ( !baro->pvNotConnectedMask )
      { // if all are connected
        baro->needConnectInit = 1;
        baro->actWin->addDefExeNode( baro->aglPtr );
      }

    }
    else
    {

      baro->pvNotConnectedMask |= 8; // null pv not connected
      baro->active = 0;
      baro->barColour.setDisconnected();
      baro->fgColour.setDisconnected();
      baro->bufInvalidate();
      baro->needDraw = 1;
      baro->actWin->addDefExeNode( baro->aglPtr );

    }

  }

  baro->actWin->appCtx->proc->unlock();

}
static void bar_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

  activeVsBarClass *baro = (activeVsBarClass *) userarg;
  int st, sev;

  baro->actWin->appCtx->proc->lock();

  if ( baro->active )
  {

    st = pv->get_status();
    sev = pv->get_severity();
    if ( ( st != baro->oldStat ) || ( sev != baro->oldSev ) )
    {
      baro->oldStat = st;
      baro->oldSev = sev;
      baro->fgColour.setStatus( st, sev );
      baro->barColour.setStatus( st, sev );
      baro->needFullDraw = 1;
    }

    baro->curReadV = pv->get_double();
    baro->needDrawCheck = 1;
    baro->actWin->addDefExeNode( baro->aglPtr );

  }

  baro->actWin->appCtx->proc->unlock();

}

static void bar_nullUpdate (
  ProcessVariable *pv,
  void *userarg )
{

  activeVsBarClass *baro = (activeVsBarClass *) userarg;

  baro->actWin->appCtx->proc->lock();

  if ( baro->active )
  {

    baro->curNullV = pv->get_double();

    baro->needDrawCheck = 1;
    baro->actWin->addDefExeNode( baro->aglPtr );

  }

  baro->actWin->appCtx->proc->unlock();

}

static void bar_maxUpdate (
  ProcessVariable *pv,
  void *userarg )
{
  activeVsBarClass *baro = (activeVsBarClass *) userarg;

  baro->actWin->appCtx->proc->lock();

  if ( baro->active )
  {

    baro->curMaxV = pv->get_double();
    baro->readMax = baro->curMaxV;
    baro->updateScaleInfo ();

    baro->needFullDraw = 1;
//    baro->needDrawCheck = 1;
    baro->actWin->addDefExeNode( baro->aglPtr );

  }

  baro->actWin->appCtx->proc->unlock();

}

static void bar_minUpdate (
  ProcessVariable *pv,
  void *userarg )
{

  activeVsBarClass *baro = (activeVsBarClass *) userarg;

  baro->actWin->appCtx->proc->lock();

  if ( baro->active )
  {

    baro->curMinV = pv->get_double();
    baro->readMin = baro->curMinV;
    baro->updateScaleInfo ();

    baro->needFullDraw = 1;
//    baro->needDrawCheck = 1;
    baro->actWin->addDefExeNode( baro->aglPtr );

  }

  baro->actWin->appCtx->proc->unlock();

}

activeVsBarClass::activeVsBarClass ( void )
{
  name = new char[strlen("activeVsBarClass") + 1];
  strcpy( name, "activeVsBarClass" );
  minW = 50;
  minH = 5;
  minVertW = 5;
  minVertH = 10;
  barStrLen = 10;
  strcpy( fontTag, "" );
  fs = NULL;
  strcpy( label, "" );
  activeMode = 0;

  barColourMode = BARC_K_COLORMODE_STATIC;
  fgColourMode = BARC_K_COLORMODE_STATIC;
  labelType = BARC_K_LITERAL;
  border = 1;
  showScale = 1;
  labelTicks = 10;
  majorTicks = 20;
  minorTicks = 2;
  barOriginX = 0.0;
  readMin = 0;
  readMax = 10;

  limitsFromDb = 1;
  limitsFromPVs = 0;
  efReadMin.setNull(1);
  efReadMax.setNull(1);
  efPrecision.setNull(1);
  efBarOriginX.setNull(1);
  strcpy( scaleFormat, "FFloat" );
  precision = 0;
  unconnectedTimer = 0;
  eBuf = NULL;

}

// copy constructor
activeVsBarClass::activeVsBarClass
 ( const activeVsBarClass *source )
{

  activeGraphicClass *baro = (activeGraphicClass *) this;

  baro->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeVsBarClass") + 1];
  strcpy( name, "activeVsBarClass" );

  barCb = source->barCb;
  fgCb = source->fgCb;
  bgCb = source->bgCb;

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );

  barColour.copy( source->barColour );
  fgColour.copy( source->fgColour );
  bgColour.copy( source->bgColour );

  readPvExpStr.copy( source->readPvExpStr );
  nullPvExpStr.copy( source->nullPvExpStr );
  maxPvExpStr.copy( source->maxPvExpStr );
  minPvExpStr.copy( source->minPvExpStr );

  strncpy( label, source->label, 39 );

  barColourMode = source->barColourMode;
  fgColourMode = source->fgColourMode;
  labelType = source->labelType;
  border = source->border;
  showScale = source->showScale;
  labelTicks = source->labelTicks;
  majorTicks = source->majorTicks;
  minorTicks = source->minorTicks;
  barOriginX = source->barOriginX;
  barStrLen = source->barStrLen;

  minW = 50;
  minH = 5;
  minVertW = 5;
  minVertH = 10;
  activeMode = 0;

  limitsFromDb = source->limitsFromDb;
  limitsFromPVs = source->limitsFromPVs;
  readMin = source->readMin;
  readMax = source->readMax;
  precision = source->precision;

  efReadMin = source->efReadMin;
  efReadMax = source->efReadMax;
  efPrecision = source->efPrecision;
  efBarOriginX = source->efBarOriginX;
  strncpy( scaleFormat, source->scaleFormat, 15 );

  strcpy( label, source->label );

  horizontal = source->horizontal;

  unconnectedTimer = 0;

  eBuf = NULL;

  updateDimensions();

}

activeVsBarClass::~activeVsBarClass ( void )
{

/*   printf( "In activeVsBarClass::~activeVsBarClass\n" ); */

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer )
  {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

}

int activeVsBarClass::createInteractive (
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

  if ( _w >= _h )
  {
    horizontal = 1;
  }
  else
  {
    horizontal = 0;
  }

  if ( horizontal )
  {
    if ( w < minW ) w = minW;
    if ( h < minH ) h = minH;
  }
  else
  {
    if ( w < minVertW ) w = minVertW;
    if ( h < minVertH ) h = minVertH;
  }

  barColour.setColorIndex( actWin->defaultFg1Color, actWin->ci );
  fgColour.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColour.setColorIndex( actWin->defaultBgColor, actWin->ci );

  strcpy( fontTag, actWin->defaultCtlFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  efPrecision.setValue( 0 );

  updateDimensions();

  this->draw();

  this->editCreate();

  return 1;

}

int activeVsBarClass::save (
  FILE *f )
{

  int stat, major, minor, release;

  tagClass tag;

  static int zero = 0;
  static char *emptyStr = "";

  int lit = 1;
  static char *labelTypeEnumStr[2] =
  {
    "pvName",
    "literal"
  };
  static int labelTypeEnum[2] =
  {
    0,
    1
  };

  int horz = 1;
  static char *orienTypeEnumStr[2] =
  {
    "vertical",
    "horizontal"
  };
  static int orienTypeEnum[2] =
  {
    0,
    1
  };

  major = BARC_MAJOR_VERSION;
  minor = BARC_MINOR_VERSION;
  release = BARC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "indicatorColour", actWin->ci, &barColour );
  tag.loadBoolW( "indicatorAlarm", &barColourMode, &zero );
  tag.loadW( "fgColour", actWin->ci, &fgColour );
  tag.loadBoolW( "fgAlarm", &fgColourMode, &zero );
  tag.loadW( "bgColour", actWin->ci, &bgColour );
  tag.loadW( "indicatorPv", &readPvExpStr, emptyStr );
  tag.loadW( "nullPv", &nullPvExpStr, emptyStr );
  tag.loadW( "maxPv", &maxPvExpStr, emptyStr );
  tag.loadW( "minPv", &minPvExpStr, emptyStr );
  tag.loadW( "label", label, emptyStr );
  tag.loadW( "labelType", 2, labelTypeEnumStr, labelTypeEnum,
   &labelType, &lit );
  tag.loadBoolW( "showScale", &showScale, &zero );
  tag.loadW( "origin", &efBarOriginX );
  tag.loadW( "font", fontTag );
  tag.loadW( "labelTicks", &labelTicks, &zero );
  tag.loadW( "majorTicks", &majorTicks, &zero );
  tag.loadW( "minorTicks", &minorTicks, &zero );
  tag.loadBoolW( "border", &border, &zero );
  tag.loadBoolW( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadBoolW( "limitsFromPVs", &limitsFromPVs, &zero );
  tag.loadW( "precision", &efPrecision );
  tag.loadW( "min", &efReadMin );
  tag.loadW( "max", &efReadMax );
  tag.loadW( "scaleFormat", scaleFormat );
  tag.loadW( "orientation", 2, orienTypeEnumStr, orienTypeEnum,
   &horizontal, &horz );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeVsBarClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

  int major, minor, release, stat;

  tagClass tag;

  static int zero = 0;
  static char *emptyStr = "";

  int lit = 1;
  static char *labelTypeEnumStr[2] =
  {
    "pvName",
    "literal"
  };
  static int labelTypeEnum[2] =
  {
    0,
    1
  };

  int horz = 1;
  static char *orienTypeEnumStr[2] =
  {
    "vertical",
    "horizontal"
  };
  static int orienTypeEnum[2] =
  {
    0,
    1
  };

  int l;
  char fmt[31 + 1], str[31 + 1];

  this->actWin = _actWin;

  tag.init();
  tag.loadR( "beginObjectProperties" );
  tag.loadR( "major", &major );
  tag.loadR( "minor", &minor );
  tag.loadR( "release", &release );
  tag.loadR( "x", &x );
  tag.loadR( "y", &y );
  tag.loadR( "w", &w );
  tag.loadR( "h", &h );
  tag.loadR( "indicatorColour", actWin->ci, &barColour );
  tag.loadR( "indicatorAlarm", &barColourMode, &zero );
  tag.loadR( "fgColour", actWin->ci, &fgColour );
  tag.loadR( "fgAlarm", &fgColourMode, &zero );
  tag.loadR( "bgColour", actWin->ci, &bgColour );
  tag.loadR( "indicatorPv", &readPvExpStr, emptyStr );
  tag.loadR( "nullPv", &nullPvExpStr, emptyStr );
  tag.loadR( "maxPv", &maxPvExpStr, emptyStr );
  tag.loadR( "minPv", &minPvExpStr, emptyStr );
  tag.loadR( "label", 39, label, emptyStr );
  tag.loadR( "labelType", 2, labelTypeEnumStr, labelTypeEnum,
   &labelType, &lit );
  tag.loadR( "showScale", &showScale, &zero );
  tag.loadR( "origin", &efBarOriginX );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "labelTicks", &labelTicks, &zero );
  tag.loadR( "majorTicks", &majorTicks, &zero );
  tag.loadR( "minorTicks", &minorTicks, &zero );
  tag.loadR( "border", &border, &zero );
  tag.loadR( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadR( "limitsFromPVs", &limitsFromPVs, &zero );
  tag.loadR( "precision", &efPrecision );
  tag.loadR( "min", &efReadMin );
  tag.loadR( "max", &efReadMax );
  tag.loadR( "scaleFormat", 15, scaleFormat );
  tag.loadR( "orientation", 2, orienTypeEnumStr, orienTypeEnum,
   &horizontal, &horz );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) )
  {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > BARC_MAJOR_VERSION )
  {
    postIncompatable();
    return 0;
  }

  if ( major < 4 )
  {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( barColourMode == BARC_K_COLORMODE_ALARM )
    barColour.setAlarmSensitive();
  else
    barColour.setAlarmInsensitive();

  if ( fgColourMode == BARC_K_COLORMODE_ALARM )
    fgColour.setAlarmSensitive();
  else
    fgColour.setAlarmInsensitive();

  bgColour.setAlarmInsensitive();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  if ( fs )
  {
    barStrLen = XTextWidth( fs, "10", 2 );
  }

  if ( limitsFromDb || efPrecision.isNull() )
    precision = 0;
  else
    precision = efPrecision.value();

  if ( ( limitsFromPVs || limitsFromDb || efReadMin.isNull() ) &&
       ( limitsFromPVs || limitsFromDb || efReadMax.isNull() ) )
  {
    readMin = 0;
    readMax = 10;
  }
  else
  {
    readMin = efReadMin.value();
    readMax = efReadMax.value();
  }

  if ( strcmp( scaleFormat, "GFloat" ) == 0 )
  {
    sprintf( fmt, "%%.%-dg", precision );
  }
  else if ( strcmp( scaleFormat, "Exponential" ) == 0 )
  {
    sprintf( fmt, "%%.%-de", precision );
  }
  else
  {
    sprintf( fmt, "%%.%-df", precision );
  }

  sprintf( str, fmt, readMin );
  if ( fs )
  {
    barStrLen = XTextWidth( fs, str, strlen(str) );
  }

  sprintf( str, fmt, readMax );
  if ( fs )
  {
    l = XTextWidth( fs, str, strlen(str) );
    if ( l > barStrLen ) barStrLen = l;
  }

  readV = barOriginX;
  curReadV = barOriginX;
  curNullV = 0.0;
  curMaxV = readMax;
  curMinV = readMin;
  updateDimensions();

  return stat;

}

int activeVsBarClass::genericEdit ( void )
{

  char title[48], *ptr;

  if ( !eBuf )
  {
    eBuf = new editBufType;
  }

  if ( horizontal )
    strcpy( title, activeVsBarClass_str3 );
  else
    strcpy( title, activeVsBarClass_str4 );

  ptr = actWin->obj.getNameFromClass( "activeVsBarClass" );
  if ( ptr )
    Strncat( title, ptr, 47 );
  else
    Strncat( title, activeVsBarClass_str5, 47 );

  Strncat( title, activeVsBarClass_str6, 47 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufBarColour = barColour.pixelIndex();
  bufBarColourMode = barColourMode;

  bufFgColour = fgColour.pixelIndex();
  bufFgColourMode = fgColourMode;

  bufBgColour = bgColour.pixelIndex();

  strncpy( bufFontTag, fontTag, 63 );

  if ( readPvExpStr.getRaw() )
    strncpy( eBuf->bufReadPvName, readPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufReadPvName, "" );

  if ( nullPvExpStr.getRaw() )
    strncpy( eBuf->bufNullPvName, nullPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufNullPvName, "" );

  if ( maxPvExpStr.getRaw() )
    strncpy( eBuf->bufMaxPvName, maxPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufMaxPvName, "" );

  if ( minPvExpStr.getRaw() )
    strncpy( eBuf->bufMinPvName, minPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufMinPvName, "" );

  strncpy( bufLabel, label, 39 );

  bufLabelType = labelType;

  bufBorder = border;

  bufShowScale = showScale;
  bufLabelTicks = labelTicks;
  bufMajorTicks = majorTicks;
  bufMinorTicks = minorTicks;

  bufEfBarOriginX = efBarOriginX;

  bufLimitsFromDb = limitsFromDb;
  bufLimitsFromPVs = limitsFromPVs;
  bufEfPrecision = efPrecision;
  bufEfReadMin = efReadMin;
  bufEfReadMax = efReadMax;
  strncpy( bufScaleFormat, scaleFormat, 15 );
  bufHorizontal = horizontal;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeVsBarClass_str7, 35, &bufX );
  ef.addTextField( activeVsBarClass_str8, 35, &bufY );
  ef.addTextField( activeVsBarClass_str9, 35, &bufW );
  ef.addTextField( activeVsBarClass_str10, 35, &bufH );
  ef.addTextField( activeVsBarClass_str12, 35, eBuf->bufReadPvName, PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeVsBarClass_str13, 35, eBuf->bufNullPvName, PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeVsBarClass_str47, 35, eBuf->bufMinPvName, PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeVsBarClass_str46, 35, eBuf->bufMaxPvName, PV_Factory::MAX_PV_NAME );
  ef.addOption( activeVsBarClass_str14, activeVsBarClass_str15, &bufLabelType );
  ef.addTextField( activeVsBarClass_str16, 35, bufLabel, 39 );
  ef.addToggle( activeVsBarClass_str18, &bufBorder );
  ef.addToggle( activeVsBarClass_str19, &bufShowScale );

  ef.addTextField( activeVsBarClass_str20, 35, &bufLabelTicks );
  ef.addTextField( activeVsBarClass_str21, 35, &bufMajorTicks );
  ef.addTextField( activeVsBarClass_str22, 35, &bufMinorTicks );

  ef.addToggle( activeVsBarClass_str48, &bufLimitsFromPVs );
  ef.addToggle( activeVsBarClass_str23, &bufLimitsFromDb );
  ef.addOption( activeVsBarClass_str24, activeVsBarClass_str25, bufScaleFormat, 15 );
  ef.addTextField( activeVsBarClass_str26, 35, &bufEfPrecision );
  ef.addTextField( activeVsBarClass_str27, 35, &bufEfReadMin );
  ef.addTextField( activeVsBarClass_str28, 35, &bufEfReadMax );

  ef.addTextField( activeVsBarClass_str29, 35, &bufEfBarOriginX );

  ef.addOption( activeVsBarClass_str44, activeVsBarClass_str45,
   &bufHorizontal );

  ef.addColorButton( activeVsBarClass_str30, actWin->ci, &barCb, &bufBarColour );
  ef.addToggle( activeVsBarClass_str31, &bufBarColourMode );
  ef.addColorButton( activeVsBarClass_str32, actWin->ci, &fgCb, &bufFgColour );
  ef.addToggle( activeVsBarClass_str33, &bufFgColourMode );
  ef.addColorButton( activeVsBarClass_str34, actWin->ci, &bgCb, &bufBgColour );

  ef.addFontMenu( activeVsBarClass_str17, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeVsBarClass::editCreate ( void )
{

  this->genericEdit();
  ef.finished( barc_edit_ok, barc_edit_apply, barc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeVsBarClass::edit ( void )
{

  this->genericEdit();
  ef.finished( barc_edit_ok, barc_edit_apply, barc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeVsBarClass::erase ( void )
{

  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeVsBarClass::eraseActive ( void )
{

  if ( !enabled || !activeMode || !init ) return 1;

  actWin->executeGc.setFG( bgColour.getColor() );

  if ( bufInvalid )
  {

    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );

    //XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
    // actWin->executeGc.normGC(), x, y, w, h );

    //XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
    // actWin->executeGc.normGC(), x, y, w, h );

    XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

  }
  else
  {

//      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
//       actWin->executeGc.normGC(), oldBarX, barY, oldBarW, barH );

    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), oldBarX, barY, oldBarW, barH );

  }

  return 1;

}

void activeVsBarClass::drawHorzScale (
  Widget widget,
  gcClass *gc )
{
  drawXLinearScale ( actWin->d, XtWindow(widget), gc, 1, barAreaX,
   barY + barH + 3, barAreaW, readMin, readMax, labelTicks,
   majorTicks, minorTicks, fgColour.pixelColor(),
   bgColour.pixelColor(), 0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 0 );

  return;
}

void activeVsBarClass::drawVertScale (
  Widget widget,
  gcClass *gc )
{
  drawYLinearScale ( actWin->d, XtWindow(widget), gc, 1, barAreaX - 4,
   barAreaY, barAreaH, readMin, readMax, labelTicks,
   majorTicks, minorTicks, fgColour.pixelColor(),
   bgColour.pixelColor(), 0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 0 );

  return;
}

void activeVsBarClass::drawScale (
  Widget widget,
  gcClass *gc )
{

  if ( horizontal )
    drawHorzScale( widget, gc );
  else
    drawVertScale( widget, gc );

}

int activeVsBarClass::draw ( void )
{

  int tX, tY;

  if ( deleteRequest ) return 1;

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  actWin->drawGc.saveFg();

  if ( horizontal )
  {

    actWin->drawGc.setFG( bgColour.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( barColour.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), barAreaX, barY, barAreaW, barH );

    actWin->drawGc.setFG( fgColour.getColor() );

    if ( showScale ) drawScale( actWin->drawWidget, &actWin->drawGc );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    if ( strcmp( label, "" ) != 0 )
    {
      if ( fs )
      {
        actWin->drawGc.setFontTag( fontTag, actWin->fi );
        tX = barAreaX;
        tY = y + 2;
        if ( border ) tY += 2;
        drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
         XmALIGNMENT_BEGINNING, label );
      }
    }

  }
  else
  { // vertical

    actWin->drawGc.setFG( bgColour.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( barColour.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), barAreaX, barAreaY-barAreaH,
     barAreaW, barAreaH );

    actWin->drawGc.setFG( fgColour.getColor() );

    if ( showScale ) drawScale( actWin->drawWidget, &actWin->drawGc );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    if ( strcmp( label, "" ) != 0 )
    {
      if ( fs )
      {
        actWin->drawGc.setFontTag( fontTag, actWin->fi );
        tX = barAreaX + barAreaW;
        tY = y + (int) ( .25 * (double) fontHeight );
        if ( border ) tY += 2;
        drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
         XmALIGNMENT_END, label );
      }
    }

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeVsBarClass::drawActive ( void )
{

  int tX, tY, x0, y0, x1, y1;
  char str[39+1];

  if ( !init )
  {
    if ( needToDrawUnconnected )
    {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( bgColour.getDisconnected() );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
    }
  }
  else if ( needToEraseUnconnected )
  {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
  }

  if ( !enabled || !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  if ( horizontal )
  {

    if ( bufInvalid )
    {

      actWin->executeGc.setFG( bgColour.getColor() );

      XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      actWin->executeGc.setFG( barColour.getColor() );

      XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), barX, barY, barW, barH );

    }
    else
    {

      if ( zeroCrossover )
      {

        actWin->executeGc.setFG( bgColour.getColor() );

        XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
         actWin->executeGc.normGC(), oldBarX, barY, oldBarW, barH );

        actWin->executeGc.setFG( barColour.getColor() );

        XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
         actWin->executeGc.normGC(), barX, barY, barW, barH );

      }
      else
      {

        if ( aboveBarOrigin )
        {

          if ( barW > oldBarW )
          {

            actWin->executeGc.setFG( barColour.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), oldBarX+oldBarW, barY,
             barW-oldBarW, barH );

          }
          else
          {

            actWin->executeGc.setFG( bgColour.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX+barW, barY,
             oldBarW-barW, barH );

          }

        }
        else
        {

          if ( barX < oldBarX )
          {

            actWin->executeGc.setFG( barColour.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY,
             oldBarX-barX, barH );

          }
          else
          {

            actWin->executeGc.setFG( bgColour.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), oldBarX, barY,
             barX-oldBarX, barH );

          }

        }

      }

    }

    oldBarX = barX;
    oldBarW = barW;

  }
  else
  { // vertical

    if ( bufInvalid )
    {

      actWin->executeGc.setFG( bgColour.getColor() );

      XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      actWin->executeGc.setFG( barColour.getColor() );

      XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), barX, barY-barH, barW, barH );

      // draw line along origin
      if ( border || showScale )
        x0 = barAreaX - 4;
      else
        x0 = x;
      x1 = x + w;
      y1 = y0 = (int) rint( barAreaY -
       ( barOriginX - readMin ) * barAreaH / ( readMax - readMin ) );
      XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x0, y0, x1, y1 );

    }
    else
    {

      if ( zeroCrossover )
      {

        actWin->executeGc.setFG( bgColour.getColor() );

        XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
         actWin->executeGc.normGC(), barX, oldBarY-oldBarH, barW, oldBarH );

        actWin->executeGc.setFG( barColour.getColor() );

        XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
         actWin->executeGc.normGC(), barX, barY-barH, barW, barH );

        // draw line along origin
        if ( border || showScale )
          x0 = barAreaX - 4;
        else
          x0 = x;
        x1 = x + w;
        y1 = y0 = (int) rint( barAreaY -
         ( barOriginX - readMin ) * barAreaH / ( readMax - readMin ) );
        XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
         actWin->executeGc.normGC(), x0, y0, x1, y1 );

      }
      else
      {

        if ( aboveBarOrigin )
        {

          if ( barH > oldBarH )
          {

            actWin->executeGc.setFG( barColour.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY-barH,
             barW, barH-oldBarH );

          }
          else
          {

            actWin->executeGc.setFG( bgColour.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY-oldBarH,
             barW, oldBarH-barH );

          }

        }
        else
        {

          if ( barY > oldBarY )
          {

            actWin->executeGc.setFG( barColour.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, oldBarY,
             barW, barY-oldBarY );

          }
          else
          {

            actWin->executeGc.setFG( bgColour.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY,
             barW, oldBarY-barY );

          }

        }

      }

    }

    oldBarY = barY;
    oldBarH = barH;

  }

  if ( bufInvalid )
  { // draw scale, label, etc ...

    actWin->executeGc.setFG( fgColour.getColor() );

    if ( showScale )
    {
      drawScale( actWin->executeWidget, &actWin->executeGc );
    }

    if ( labelType == BARC_K_PV_NAME )
      strncpy( str, readPvId->get_name(), 39 );
    else
      strncpy( str, label, 39 );

    if ( horizontal )
    {

      if ( strcmp( str, "" ) != 0 )
      {
        if ( fs )
        {
          actWin->executeGc.setFontTag( fontTag, actWin->fi );
          tX = barAreaX;
          tY = y + 2;
          if ( border ) tY += 2;
          drawText( actWin->executeWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_BEGINNING, str );
        }
      }

    }
    else
    {

      if ( strcmp( str, "" ) != 0 )
      {
        if ( fs )
        {
          actWin->executeGc.setFontTag( fontTag, actWin->fi );
          tX = barAreaX + barAreaW;
          tY = y + (int) ( .25 * (double) fontHeight );
          if ( border ) tY += 2;
          drawText( actWin->executeWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_END, str );
        }
      }

    }

    if ( border )
    {
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
    }

    bufInvalid = 0;

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int activeVsBarClass::activate (
  int pass,
  void *ptr )
{

  int opStat;

  switch ( pass )
  {

  case 1:

    zeroCrossover = 0;
    oldAboveBarOrigin = 0;
    needConnectInit = needInfoInit = needRefresh = needErase = needDrawCheck =
     needDraw = needFullDraw = 0;
    needToEraseUnconnected = 0;
    needToDrawUnconnected = 0;
    unconnectedTimer = 0;

    readPvId = nullPvId = maxPvId = minPvId = NULL;
    initialReadConnection = initialNullConnection = initialMaxConnection = initialMinConnection = 1;
    oldStat = oldSev = -1;

    aglPtr = ptr;
    opComplete = 0;
    curNullV = 0.0;

    if ( horizontal )
    {
      barW = 0;
      oldBarW = 0;
      barX = 0;
      oldBarX = 0;
    }
    else
    {
      barH = 0;
      oldBarH = 0;
      barY = 0;
      oldBarY = 0;
    }

    pvNotConnectedMask = active = init = 0;
    activeMode = 1;

    if ( !readPvExpStr.getExpanded() ||
       blankOrComment( readPvExpStr.getExpanded() ) )
    {
      readExists = 0;
    }
    else
    {
      readExists = 1;
      pvNotConnectedMask |= 1;
      barColour.setConnectSensitive();
      fgColour.setConnectSensitive();
    }

    if ( !nullPvExpStr.getExpanded() ||
       blankOrComment( nullPvExpStr.getExpanded() ) )
    {
      nullExists = 0;
    }
    else
    {
      nullExists = 1;
      pvNotConnectedMask |= 2;
    }

    if ( !maxPvExpStr.getExpanded() ||
       blankOrComment( maxPvExpStr.getExpanded() ) )
    {
      maxExists = 0;
    }
    else
    {
      maxExists = 1;
      pvNotConnectedMask |= 4;
    }

    if ( !minPvExpStr.getExpanded() ||
       blankOrComment( minPvExpStr.getExpanded() ) )
    {
      minExists = 0;
    }
    else
    {
      minExists = 1;
      pvNotConnectedMask |= 8;
    }

    break;

  case 2:

    if ( !opComplete )
    {

      initEnable();

      if ( !unconnectedTimer )
      {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      opStat = 1;

      if ( readExists )
      {
        readPvId = the_PV_Factory->create( readPvExpStr.getExpanded() );
        if ( readPvId )
        {
          readPvId->add_conn_state_callback( bar_monitor_read_connect_state,
           this );
        }
        else
        {
          printf( activeVsBarClass_str36 );
          opStat = 0;
        }
      }

      if ( nullExists )
      {
        nullPvId = the_PV_Factory->create( nullPvExpStr.getExpanded() );
        if ( nullPvId )
        {
          nullPvId->add_conn_state_callback( bar_monitor_null_connect_state,
           this );
        }
        else
        {
          printf( activeVsBarClass_str36 );
          opStat = 0;
        }
      }

      if ( maxExists )
      {
        maxPvId = the_PV_Factory->create( maxPvExpStr.getExpanded() );
        if ( maxPvId )
        {
          maxPvId->add_conn_state_callback( bar_monitor_max_connect_state,
           this );
        }
        else
        {
          printf( activeVsBarClass_str36 );
          opStat = 0;
        }
      }

      if ( minExists )
      {
        minPvId = the_PV_Factory->create( minPvExpStr.getExpanded() );
        if ( minPvId )
        {
          minPvId->add_conn_state_callback( bar_monitor_min_connect_state,
           this );
        }
        else
        {
          printf( activeVsBarClass_str36 );
          opStat = 0;
        }
      }

      if ( opStat & 1 ) opComplete = 1;

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

int activeVsBarClass::deactivate (
  int pass
)
{

  active = 0;
  activeMode = 0;

  if ( pass == 1 )
  {

    if ( unconnectedTimer )
    {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    if ( readExists )
    {
      if ( readPvId )
      {
        readPvId->remove_conn_state_callback( bar_monitor_read_connect_state,
         this );
        readPvId->remove_value_callback( bar_readUpdate, this );
        readPvId->release();
        readPvId = NULL;
      }
    }

    if ( nullExists )
    {
      if ( nullPvId )
      {
        nullPvId->remove_conn_state_callback( bar_monitor_null_connect_state,
         this );
        nullPvId->remove_value_callback( bar_nullUpdate, this );
        nullPvId->release();
        nullPvId = NULL;
      }
    }

    if ( maxExists )
    {
      if ( maxPvId )
      {
        maxPvId->remove_conn_state_callback( bar_monitor_max_connect_state,
         this );
        maxPvId->remove_value_callback( bar_maxUpdate, this );
        maxPvId->release();
        maxPvId = NULL;
      }
    }

    if ( minExists )
    {
      if ( minPvId )
      {
        minPvId->remove_conn_state_callback( bar_monitor_min_connect_state,
         this );
        minPvId->remove_value_callback( bar_minUpdate, this );
        minPvId->release();
        minPvId = NULL;
      }
    }

  }

  return 1;

}

void activeVsBarClass::updateDimensions ( void )
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

  barAreaX = x;
  barAreaY = y;
  barAreaW = w;
  barAreaH = h;

  if ( horizontal )
  {

    minH = 5;
    barY = y;

    barAreaX = x;
    barAreaW = w;

    if ( ( strcmp( label, "" ) != 0 ) ||
         ( labelType == BARC_K_PV_NAME ) )
    {
      minH += fontHeight + 5;
      barY += fontHeight + 5;
      if ( border )
      {
        minH += 9;
        barY += 5;
        barAreaX = x + 5;
        barAreaW = w - 9;
      }
    }
    else
    {
      if ( border && showScale )
      {
        minH += 9;
        barY += 5;
      }
    }

    if ( showScale )
    {
      minH += fontHeight + fontHeight + 5;
      barAreaX = x + barStrLen/2 + 3;
      barAreaW = w - barStrLen - 6;
    }

    if ( border && !showScale && ( ( strcmp( label, "" ) == 0 ) ||
     ( labelType == BARC_K_PV_NAME ) ) )
    {
      minH += 9;
      barY += 5;
      barAreaX = x + 5;
      barAreaW = w - 9;
    }

    if ( h < minH )
    {

      h = minH;
      sboxH = minH;

    }

    barH = h;

    if ( ( strcmp( label, "" ) != 0 ) ||
         ( labelType == BARC_K_PV_NAME ) )
    {
      barH -= ( fontHeight + 5 );
      if ( border ) barH -= 9;
    }

    if ( showScale )
    {
      barH -= ( fontHeight + fontHeight + 5 );
    }

    if ( border && !showScale && ( ( strcmp( label, "" ) == 0 ) ||
     ( labelType == BARC_K_PV_NAME ) ) )
    {
      barH -= 9;
    }

  }
  else
  {  // vertical

    minVertW = 5;
    minVertH = 10;

    if ( ( strcmp( label, "" ) != 0 ) ||
         ( labelType == BARC_K_PV_NAME ) )
    {
      minVertH += fontHeight + 5;
    }

    if ( showScale )
    {
      minVertH += fontHeight;
      minVertW += 4 + barStrLen + 10 + (int) rint( 0.5 * fontHeight );
    }
    else if ( border )
    {
      minVertH += 8;
      minVertW += 4;
    }

    if ( w < minVertW )
    {
      w = minVertW;
      sboxW = minVertW;
    }

    if ( h < minVertH )
    {
      h = minVertH;
      sboxH = minVertH;
    }

    barH = barAreaH = h;
    barY = barAreaY = y + barAreaH;
    barX = barAreaX = x;
    barW = barAreaW = w;

    if ( ( strcmp( label, "" ) != 0 ) ||
         ( labelType == BARC_K_PV_NAME ) )
    {
      barAreaH -= (int) ( 1.5 * (double) fontHeight ) - 5;
      barH = barAreaH;
    }

    if ( showScale )
    {
      barH -= ( fontHeight );
      barAreaH -= ( fontHeight );
    }
    else if ( border )
    {
      barH -= 8;
      barAreaH -= 8;
    }

    if ( showScale )
    {
      barY -= (int) rint( 0.5 * fontHeight );
      barAreaY -= (int) rint( 0.5 * fontHeight );
      barAreaW -= ( 4 +  barStrLen + 8 + (int) rint( 0.5 * fontHeight ) );
      barW -= ( 4 + barStrLen + 8 + (int) rint( 0.5 * fontHeight ) );
      barAreaX += 2 + barStrLen + 8 + (int) rint( 0.5 * fontHeight );
      barX += 2 + barStrLen + 8 + (int) rint( 0.5 * fontHeight );
    }
    else if ( border )
    {
      barY -= 4;
      barAreaY -= 4;
      barAreaW -= 9;
      barW -= 9;
      barAreaX += 5;
      barX += 5;
    }

  }

  updateScaleInfo();

}

void activeVsBarClass::btnUp (
  int x,
  int y,
  int barState,
  int barNumber )
{

  if ( !enabled ) return;

}

void activeVsBarClass::btnDown (
  int x,
  int y,
  int barState,
  int barNumber )
{

  if ( !enabled ) return;

}

void activeVsBarClass::btnDrag (
  int x,
  int y,
  int barState,
  int barNumber )
{

  if ( !enabled ) return;

}

int activeVsBarClass::getBarActionRequest (
  int *up,
  int *down,
  int *drag )
{

  *up = 0;
  *down = 0;
  *drag = 0;
  return 1;

}

void activeVsBarClass::bufInvalidate ( void )
{

  bufInvalid = 1;

}

int activeVsBarClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

  int stat;

  stat = readPvExpStr.expand1st( numMacros, macros, expansions );
  stat = nullPvExpStr.expand1st( numMacros, macros, expansions );
  stat = maxPvExpStr.expand1st( numMacros, macros, expansions );
  stat = minPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeVsBarClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = nullPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = maxPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = minPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeVsBarClass::containsMacros ( void )
{

  int result;

  result = readPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = nullPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = maxPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = minPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  return 0;

}

int activeVsBarClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h )
{

  int tmpw, tmph, ret_stat;

  ret_stat = 1;

  tmpw = sboxW;
  tmph = sboxH;

  tmpw += _w;
  tmph += _h;

  if ( horizontal )
  {

    if ( tmpw < minW )
    {
      ret_stat = 0;
    }

    if ( tmph < minH )
    {
      ret_stat = 0;
    }

  }
  else
  {

    if ( tmpw < minVertW )
    {
      ret_stat = 0;
    }

    if ( tmph < minVertH )
    {
      ret_stat = 0;
    }

  }

  return ret_stat;

}

int activeVsBarClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h )
{

  int tmpw, tmph, ret_stat;

  ret_stat = 1;

  tmpw = _w;
  tmph = _h;

  if ( horizontal )
  {

    if ( tmpw != -1 )
    {
      if ( tmpw < minW )
      {
        ret_stat = 0;
      }
    }

    if ( tmph != -1 )
    {
      if ( tmph < minH )
      {
        ret_stat = 0;
      }
    }

  }
  else
  {

    if ( tmpw != -1 )
    {
      if ( tmpw < minVertW )
      {
        ret_stat = 0;
      }
    }

    if ( tmph != -1 )
    {
      if ( tmph < minVertH )
      {
        ret_stat = 0;
      }
    }

  }

  return ret_stat;

}

void activeVsBarClass::updateScaleInfo ( void )
{
  if ( horizontal )
    updateHorzScaleInfo();
  else
    updateVertScaleInfo();

}

void activeVsBarClass::updateHorzScaleInfo ( void )
{

  int locW;

  if ( readMax == readMin ) readMax = readMin + 1.0;

  if ( readMax >= readMin )
  {

    mode = BARC_K_MAX_GE_MIN;

    if ( barOriginX < readMin )
      barOriginX = readMin;
    else if ( barOriginX > readMax )
     barOriginX = readMax;

    originW = (int) rint( ( barOriginX - readMin ) *
     barAreaW / ( readMax - readMin ) );

    if ( readV > readMax ) readV = readMax;
    if ( readV < readMin ) readV = readMin;

  }
  else
  {

    mode = BARC_K_MAX_LT_MIN;

    if ( barOriginX > readMin )
      barOriginX = readMin;
    else if ( barOriginX < readMax )
      barOriginX = readMax;

    originW = (int) rint( ( barOriginX - readMin ) *
     barAreaW / ( readMax - readMin ) );

    if ( readV < readMax ) readV = readMax;
    if ( readV > readMin ) readV = readMin;

  }

  switch ( mode )
  {

  case BARC_K_MAX_GE_MIN:

    if ( readV >= barOriginX)
    {

      barX = originW;

      if ( barOriginX == readMax)
      {

        barW = 0;
        factorGe = 0;

      }
      else
      {

        barW = (int) ( ( barAreaW - originW ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barW > ( barAreaW - originW ) )
          barW = barAreaW - originW;

        factorGe = ( barAreaW - originW ) / ( readMax - barOriginX );
        barMaxW = barAreaW - originW;

      }

    }
    else
    {

      if ( barOriginX == readMin )
      {

        locW = 0;
        factorLt = 0;

      }
      else
      {

        locW = (int) ( ( originW ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );

        factorLt = originW / ( readMin - barOriginX );

      }

      barX = originW - locW;

      barW = abs( locW );

      if ( barX < 0 )
      {
        barX = 0;
        barW = originW;
      }

    }

    break;

  case BARC_K_MAX_LT_MIN:

    if ( readV < barOriginX )
    {

      barX = originW;

      if ( barOriginX == readMax )
      {

        barW = 0;
        factorLt = 0;

      }
      else
      {

        barW = (int) ( ( barAreaW - originW ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barW > ( barAreaW - originW ) )
          barW = barAreaW - originW;

        factorLt = ( barAreaW - originW ) / ( readMax - barOriginX );

      }

      barMaxW = barAreaW - originW;

    }
    else
    {

      if ( barOriginX == readMin )
      {

        locW = 0;
        factorGe = 0;

      }
      else
      {

        locW = (int) ( ( originW ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );

        factorGe = originW / ( readMin - barOriginX );

      }

      barX = originW - locW;

      barW = abs( locW );

      if ( barX < 0 )
      {
        barX = 0;
        barW = originW;
      }

    }

    break;

  }

  barX += barAreaX;

}

void activeVsBarClass::updateVertScaleInfo ( void )
{

  int locH;

  if ( readMax == readMin ) readMax = readMin + 1.0;

  if ( readMax >= readMin )
  {

    mode = BARC_K_MAX_GE_MIN;

    if ( barOriginX < readMin )
      barOriginX = readMin;
    else if ( barOriginX > readMax )
     barOriginX = readMax;

    originH = (int) rint( ( barOriginX - readMin ) *
     barAreaH / ( readMax - readMin ) );

    if ( readV > readMax ) readV = readMax;
    if ( readV < readMin ) readV = readMin;

  }
  else
  {

    mode = BARC_K_MAX_LT_MIN;

    if ( barOriginX > readMin )
      barOriginX = readMin;
    else if ( barOriginX < readMax )
     barOriginX = readMax;

    originH = (int) rint( ( barOriginX - readMin ) *
     barAreaH / ( readMax - readMin ) );

    if ( readV < readMax ) readV = readMax;
    if ( readV > readMin ) readV = readMin;

  }

  switch ( mode )
  {

  case BARC_K_MAX_GE_MIN:

    if ( readV >= barOriginX )
    {
      barY = barAreaY - originH;

      if ( barOriginX == readMax )
      {
        barH = 0;
        factorGe = 0;
      }
      else
      {
        barH = (int) ( ( barAreaH - originH ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barH > ( barAreaH - originH ) )
          barH = barAreaH - originH;

        factorGe = ( barAreaH - originH ) / ( readMax - barOriginX );
        barMaxH = barAreaH - originH;
      }
    }
    else
    {
      if ( barOriginX == readMin )
      {
        locH = 0;
        factorLt = 0;
      }
      else
      {
        locH = (int) ( ( originH ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );
        factorLt = originH / ( readMin - barOriginX );
      }

      barY = barAreaY - ( originH - locH );
      barH = abs( locH );

      if ( barY < 0 )
      {
        barY = 0;
        barH = originH;
      }
    }

    break;

  case BARC_K_MAX_LT_MIN:

    if ( readV < barOriginX )
    {
      barY = barAreaY - originH;

      if ( barOriginX == readMax )
      {
        barH = 0;
        factorLt = 0;
      }
      else
      {
        barH = (int) ( ( barAreaH - originH ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barH > ( barAreaH - originH ) )
          barH = barAreaH - originH;

        factorLt = ( barAreaH - originH ) / ( readMax - barOriginX );
      }
      barMaxH = barAreaH - originH;
    }
    else
    {
      if ( barOriginX == readMin )
      {
        locH = 0;
        factorGe = 0;
      }
      else
      {
        locH = (int) ( ( originH ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );

        factorGe = originH / ( readMin - barOriginX );
      }

      barY = barAreaY - ( originH - locH );
      barH = abs( locH );

      if ( barY < 0 )
      {
        barY = 0;
        barH = originH;
      }
    }
    break;
  }
}

void activeVsBarClass::updateBar ( void )
{
  int locW, locH;

  if ( horizontal )
  {
    switch ( mode )
    {

    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX )
      {
        aboveBarOrigin = 1;
      }
      else
      {
        aboveBarOrigin = 0;
      }
      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX )
      {
        aboveBarOrigin = 1;
      }
      else
      {
        aboveBarOrigin = 0;
      }
      break;
    }

    if ( aboveBarOrigin != oldAboveBarOrigin )
    {
      oldAboveBarOrigin = aboveBarOrigin;
      zeroCrossover = 1;
      updateScaleInfo();
    }
    else
    {
      zeroCrossover = 0;
    }

    switch ( mode )
    {
    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX )
      {
        barX = originW;
        barW = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );
        if ( barW > barMaxW ) barW = barMaxW;
      }
      else
      {
        locW = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );
        barX = originW - locW;
        barW = abs( locW );

        if ( barX < 0 )
        {
          barX = 0;
          barW = originW;
        }
      }
      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX )
      {
        barX = originW;
        barW = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );
        if ( barW > barMaxW ) barW = barMaxW;
      }
      else
      {
        locW = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );
        barX = originW - locW;
        barW = abs( locW );

        if ( barX < 0 )
        {
          barX = 0;
          barW = originW;
        }
      }
      break;
    }
    barX += barAreaX;
  }
  else
  { // vertical

    switch ( mode )
    {

    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX )
      {
        aboveBarOrigin = 1;
      }
      else
      {
        aboveBarOrigin = 0;
      }
      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX )
      {
        aboveBarOrigin = 1;
      }
      else
      {
        aboveBarOrigin = 0;
      }
      break;
    }

    if ( aboveBarOrigin != oldAboveBarOrigin )
    {
      oldAboveBarOrigin = aboveBarOrigin;
      zeroCrossover = 1;
      updateScaleInfo();
    }
    else
    {
      zeroCrossover = 0;
    }

    switch ( mode )
    {

    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX )
      {
        barY = barAreaY - originH;
        barH = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );
        if ( barH > barMaxH ) barH = barMaxH;
      }
      else
      {
        locH = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );
        barY = barAreaY - ( originH - locH );
        barH = abs( locH );

        if ( barY > barAreaY )
        {
          barY = barAreaY;
          barH = originH;
        }
      }
      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX )
      {
        barY = barAreaY - originH;
        barH = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );
        if ( barH > barMaxH ) barH = barMaxH;
      }
      else
      {
        locH = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );
        barY = barAreaY - ( originH - locH );
        barH = abs( locH );

        if ( barY > barAreaY )
        {
          barY = barAreaY;
          barH = originH;
        }
      }
      break;
    }
  }
}

void activeVsBarClass::executeDeferred ( void )
{
  int l, nc, ni, nr, ne, nd, nfd, ndc;
  char fmt[31+1], str[31+1];
  double v;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  nfd = needFullDraw; needFullDraw = 0;
  ndc = needDrawCheck; needDrawCheck = 0;
  v = curReadV - curNullV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc )
  {
    v = curReadV = readPvId->get_double();

    if (limitsFromPVs)
    {
      readMin = minPvId->get_double ();
      readMax = maxPvId->get_double ();
    }
    else
    {
      if ( limitsFromDb || efReadMin.isNull() )
      {
        readMin = readPvId->get_lower_disp_limit();
      }

      if ( limitsFromDb || efReadMax.isNull() )
      {
        readMax = readPvId->get_upper_disp_limit();
      }
    }

    if ( limitsFromPVs || limitsFromDb || efPrecision.isNull() )
    {
      precision = readPvId->get_precision();
    }
    ni = 1;
  }

//----------------------------------------------------------------------------

  if ( ni )
  {

    if ( efBarOriginX.isNull() )
    {
      barOriginX = readMin;
    }

    if ( strcmp( scaleFormat, "GFloat" ) == 0 )
    {
      sprintf( fmt, "%%.%-dg", precision );
    }
    else if ( strcmp( scaleFormat, "Exponential" ) == 0 )
    {
      sprintf( fmt, "%%.%-de", precision );
    }
    else
    {
      sprintf( fmt, "%%.%-df", precision );
    }

    sprintf( str, fmt, readMin );
    if ( fs )
    {
      barStrLen = XTextWidth( fs, str, strlen(str) );
    }

    sprintf( str, fmt, readMax );
    if ( fs )
    {
      l = XTextWidth( fs, str, strlen(str) );
      if ( l > barStrLen ) barStrLen = l;
    }

    updateDimensions();

    active = 1;
    init = 1;
    barColour.setConnected();
    fgColour.setConnected();
    bufInvalidate();
    eraseActive();
    readV = v;
    updateDimensions();
    drawActive();

    if ( initialReadConnection )
    {
      initialReadConnection = 0;
      readPvId->add_value_callback( bar_readUpdate, this );
    }

    if ( nullExists )
    {
      if ( initialNullConnection )
      {
        initialNullConnection = 0;
        nullPvId->add_value_callback( bar_nullUpdate, this );
      }
    }

    if ( maxExists )
    {
      if ( initialMaxConnection )
      {
        initialMaxConnection = 0;
        maxPvId->add_value_callback( bar_maxUpdate, this );
      }
    }

    if ( minExists )
    {
      if ( initialMinConnection )
      {
        initialMinConnection = 0;
        minPvId->add_value_callback( bar_minUpdate, this );
      }
    }
  }

//----------------------------------------------------------------------------

  if ( nr )
  {
    bufInvalidate();
    eraseActive();
    readV = v;
    updateDimensions();
    drawActive();
  }

//----------------------------------------------------------------------------

  if ( ne )
  {
    eraseActive();
  }

//----------------------------------------------------------------------------

  if ( nd )
  {
    readV = v;
    drawActive();
  }

//----------------------------------------------------------------------------

  if ( nfd )
  {
    readV = v;
    bufInvalidate();
    drawActive();
  }

//----------------------------------------------------------------------------

  if ( ndc )
  {
      readV = v;
      updateBar();
      drawActive();
  }

//----------------------------------------------------------------------------

}

char *activeVsBarClass::firstDragName ( void )
{
  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];
}

char *activeVsBarClass::nextDragName ( void )
{
  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 )
  {
    dragIndex++;
    return dragName[dragIndex];
  }
  else
  {
    return NULL;
  }
}

char *activeVsBarClass::dragValue (
  int i )
{
  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE )
  {
    if ( !i )
    {
      return readPvExpStr.getExpanded();
    }
    else
    if (i == 1)
    {
      return nullPvExpStr.getExpanded();
    }
    else
    if (i == 2)
    {
      return maxPvExpStr.getExpanded();
    }
    else
    {
      return minPvExpStr.getExpanded();
    }
  }
  else
  {
    if ( !i )
    {
      return readPvExpStr.getRaw();
    }
    else
    if (i == 1)
    {
      return nullPvExpStr.getRaw();
    }
    else
    if (i == 2)
    {
      return maxPvExpStr.getRaw();
    }
    else
    {
      return minPvExpStr.getRaw();
    }
  }
}

void activeVsBarClass::changeDisplayParams (
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
  if ( _flag & ACTGRF_FG1COLOR_MASK )
    barColour.setColorIndex( _fg1Colour, actWin->ci );

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColour.setColorIndex( _textFgColour, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColour.setColorIndex( _bgColour, actWin->ci );

  if ( _flag & ACTGRF_CTLFONTTAG_MASK )
  {
    strcpy( fontTag, _ctlFontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
    updateDimensions();
  }
}

void activeVsBarClass::changePvNames (
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
  if ( flag & ACTGRF_READBACKPVS_MASK )
  {
    if ( numReadbackPvs )
    {
      readPvExpStr.setRaw( readbackPvs[0] );
    }
  }

  if ( flag & ACTGRF_NULLPVS_MASK )
  {
    if ( numNullPvs )
    {
      nullPvExpStr.setRaw( nullPvs[0] );
    }
  }
}

void activeVsBarClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n )
{

  if ( max < 4 )
  {
    *n = 0;
    return;
  }
  *n = 2;
  pvs[0] = readPvId;
  pvs[1] = nullPvId;
  pvs[2] = maxPvId;
  pvs[3] = minPvId;
}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeVsBarClassPtr ( void )
{
  activeVsBarClass *ptr;

  ptr = new activeVsBarClass;
  return (void *) ptr;
}

void *clone_activeVsBarClassPtr (
  void *_srcPtr )
{
  activeVsBarClass *ptr, *srcPtr;

  srcPtr = (activeVsBarClass *) _srcPtr;
  ptr = new activeVsBarClass( srcPtr );
  return (void *) ptr;
}

#ifdef __cplusplus
}
#endif
