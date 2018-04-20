
#define __wheelSwitch_cc 1
#define CLASS_NAME "wheelSwitchClass"

#include "wheelSwitch.h"
#include "app_pkg.h"
#include "act_win.h"
#include "color_pkg.h"

#include "thread.h"

// constructor
wheelSwitchClass::wheelSwitchClass ( void ) {

  name = new char[strlen (CLASS_NAME) + 1];
  strcpy ( name, CLASS_NAME );

  valueD = 0;
  strcpy (valueS, "0.0");

  activeMode = 0;
  deleteRequest = 0;
  selected = -1;
  efScaleMin.setNull (1);
  efScaleMax.setNull (1);
  efPrecision.setNull (1);
  limitsFromDb = 1;
  precisionFromDb = 0;
  strcpy (title, "");
  strcpy (titlePosition, "Top");
  titleDisplay = 0;
  leadingPlus = 1;
  efLeadingDigits.setNull (1);
  boundsVisible = 0;
  displayUnits = 0;
  strcpy (units, "");
  outerRect = 1;
  unitsFromDb = 1;
  unconnectedTimer = 0;
  leadingDigits = 0;

  frameWidget = NULL;
//  eBuf = NULL;

}

//copy constructor
wheelSwitchClass::wheelSwitchClass ( const wheelSwitchClass *source ) {

  activeGraphicClass *slo = (activeGraphicClass *) this;

  slo->clone ( (activeGraphicClass *) source );

  name = new char[strlen (CLASS_NAME) + 1];
  strcpy ( name, CLASS_NAME );

  controlPvName.copy ( source->controlPvName );

  valueD = 0;
  strcpy (valueS, source->valueS);

  activeMode = 0;
  deleteRequest = 0;
  fgColor = source->fgColor;
  bgColor = source->bgColor;
  controlColor = source->controlColor;
  fgColorMode = source->fgColorMode;
  bgColorMode = source->bgColorMode;
  controlColorMode = source->controlColorMode;
  shadeColor = source->shadeColor;
//  offlineColor = source->offlineColor;
  strcpy (titleFontTag, source->titleFontTag);
  strcpy (displayFontTag, source->displayFontTag);
  selected = -1;
  efPrecision = source->efPrecision;
  efScaleMin = source->efScaleMin;
  efScaleMax = source->efScaleMax;
  precision = source->precision;
  scaleMin = source->scaleMin;
  scaleMax = source->scaleMax;
  limitsFromDb = source->limitsFromDb;
  precisionFromDb = source->precisionFromDb;
  fgCb = source->fgCb;
  bgCb = source->bgCb;
  controlCb = source->controlCb;
  strcpy (title, source->title);
  strcpy (titlePosition, source->titlePosition);
  titleDisplay = source->titleDisplay;
  leadingPlus = source->leadingPlus;
  efLeadingDigits = source->efLeadingDigits;
  boundsVisible = source->boundsVisible;
  displayUnits = source->displayUnits;
  strcpy (units, source->units);
  unitsFromDb = source->unitsFromDb;
  strcpy (displayFormat, source->displayFormat);

  scaleMin = efScaleMin.value ();
  scaleMax = efScaleMax.value ();
  if (efPrecision.isNull ()) precision = 1;
  else precision = efPrecision.value ();
  if (efScaleMin.isNull ()) scaleMin = -1e+100;
  if (efScaleMax.isNull ()) scaleMax = 1e+100;
  if (efLeadingDigits.isNull ()) leadingDigits = 0;
  else leadingDigits = efLeadingDigits.value ();
  outerRect = source->outerRect;

  this->initSelectBox (); // call after getting x, y, w, h

  valueD = 0;
  strcpy (valueS, "0.0");

  actWin->fi->loadFontTag ( displayFontTag );
  actWin->drawGc.setFontTag ( displayFontTag, actWin->fi );
  fsD = actWin->fi->getXFontStruct ( displayFontTag );
  updateFont ( valueS, displayFontTag, &fsD, &displayFontAscent, &displayFontDescent, &displayFontHeight,
   &displayStringWidth );
  actWin->fi->loadFontTag ( titleFontTag );
  actWin->drawGc.setFontTag ( titleFontTag, actWin->fi );
  fsT = actWin->fi->getXFontStruct ( titleFontTag );
  updateFont ( title, titleFontTag, &fsT, &titleFontAscent, &titleFontDescent, &titleFontHeight,
   &titleStringWidth );

  frameWidget = NULL;
//  eBuf = NULL;

}

// called when widget is created
int wheelSwitchClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  // used to hold a copy of the active window pointer
  actWin = aw_obj;
  // size and location
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  // set colors
  fgColor.setColorIndex (actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex (actWin->defaultBgColor, actWin->ci);
  controlColor.setColorIndex (actWin->defaultTextFgColor, actWin->ci);
  shadeColor.setColorIndex (actWin->defaultOffsetColor, actWin->ci);
//  offlineColor.setColorIndex (bgColor.getDisconnected (), actWin->ci);
  fgColorMode = 0;
  bgColorMode = 0;
  controlColorMode = 0;
  fgColor.setAlarmInsensitive ();
  bgColor.setAlarmInsensitive ();
  controlColor.setAlarmInsensitive ();
  // set font
  strcpy ( titleFontTag, actWin->defaultCtlFontTag );
  strcpy ( displayFontTag, actWin->defaultCtlFontTag );
  actWin->fi->loadFontTag ( displayFontTag );

  strcpy (displayFormat, "Float");

  // SJS Modification 25-05-05 -initialise precision and precisionFromDb
  precision = 0;
  precisionFromDb = 0;
  // End of SJS Modification

  // draw widget
  this->draw ();
  // pop up the properties dialog box
  this->editCreate ();

  return 1;

}

//saving widget to a file
int wheelSwitchClass::save ( FILE *f ) {
int zero = 0;
int stat;
tagClass tag;
char *emptyStr = "";
  tag.init ();
  tag.loadW ( "beginObjectProperties" );
  tag.loadW ( "x", &x );
  tag.loadW ( "y", &y );
  tag.loadW ( "w", &w );
  tag.loadW ( "h", &h );
  tag.loadW ( "title", title);
  tag.loadBoolW ( "titleDisplay", &titleDisplay, &zero );
  tag.loadW ( "titlePosition", titlePosition);
  tag.loadW ( "fgColor", actWin->ci, &fgColor );
  tag.loadBoolW ( "fgColorMode", &fgColorMode, &zero );
  tag.loadW ( "bgColor", actWin->ci, &bgColor );
  tag.loadBoolW ( "bgColorMode", &bgColorMode, &zero );
  tag.loadW ( "controlColor", actWin->ci, &controlColor );
  tag.loadBoolW ( "controlColorMode", &controlColorMode, &zero );
  //tag.loadW ( "offlineColor", actWin->ci, &offlineColor );
  tag.loadW ( "shadeColor", actWin->ci, &shadeColor );
  tag.loadW ( "titleFont", titleFontTag );
  tag.loadW ( "displayFont", displayFontTag );
  tag.loadBoolW ( "outerRect", &outerRect, &zero );
  tag.loadW ( "controlPv", &controlPvName, emptyStr );
  tag.loadBoolW ( "precisionFromDb", &precisionFromDb, &zero );
  tag.loadW ( "leadingDigits", &efLeadingDigits );
  tag.loadW ( "precision", &efPrecision );
  tag.loadBoolW ( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadBoolW ( "leadingPlus", &leadingPlus, &zero );
  tag.loadBoolW ( "boundsVisible", &boundsVisible, &zero );
  tag.loadW ( "units", units );
  tag.loadBoolW ( "displayUnits", &displayUnits, &zero );
  tag.loadBoolW ( "unitsFromDb", &unitsFromDb, &zero );
  tag.loadW ( "scaleMin", &efScaleMin );
  tag.loadW ( "scaleMax", &efScaleMax );
  tag.loadW ( "displayFormat", displayFormat );
  tag.loadW ( "endObjectProperties" );
  tag.loadW ( "" );
  stat = tag.writeTags ( f );
  return stat;
}

//loads widget from a file
int wheelSwitchClass::createFromFile ( FILE *f, char *name, activeWindowClass *_actWin ) {
int zero = 0;
int stat;
tagClass tag;
char *emptyStr = "";

  actWin = _actWin;
  tag.init ();
  tag.loadR ( "beginObjectProperties" );
  tag.loadR ( "x", &x );
  tag.loadR ( "y", &y );
  tag.loadR ( "w", &w );
  tag.loadR ( "h", &h );
  tag.loadR ( "title", 63, title);
  tag.loadR ( "titleDisplay", &titleDisplay, &zero );
  tag.loadR ( "titlePosition", 15, titlePosition);
  tag.loadR ( "fgColor", actWin->ci, &fgColor );
  tag.loadR ( "fgColorMode", &fgColorMode, &zero );
  tag.loadR ( "bgColor", actWin->ci, &bgColor );
  tag.loadR ( "bgColorMode", &bgColorMode, &zero );
  tag.loadR ( "controlColor", actWin->ci, &controlColor );
  tag.loadR ( "controlColorMode", &controlColorMode, &zero );
  tag.loadR ( "shadeColor", actWin->ci, &shadeColor );
  //tag.loadR ( "offlineColor", actWin->ci, &offlineColor );
  tag.loadR ( "titleFont", 63, titleFontTag );
  tag.loadR ( "displayFont", 63, displayFontTag );
  tag.loadR ( "outerRect", &outerRect, &zero );
  tag.loadR ( "controlPv", &controlPvName, emptyStr );
  tag.loadR ( "precisionFromDb", &precisionFromDb, &zero );
  tag.loadR ( "precision", &efPrecision );
  tag.loadR ( "leadingDigits", &efLeadingDigits );
  tag.loadR ( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadR ( "leadingPlus", &leadingPlus, &zero );
  tag.loadR ( "boundsVisible", &boundsVisible, &zero );
  tag.loadR ( "units", 15, units );
  tag.loadR ( "displayUnits", &displayUnits, &zero );
  tag.loadR ( "unitsFromDb", &unitsFromDb, &zero );
  tag.loadR ( "scaleMin", &efScaleMin );
  tag.loadR ( "scaleMax", &efScaleMax );
  tag.loadR ( "displayFormat", 15, displayFormat );
  tag.loadR ( "endObjectProperties" );

  stat = tag.readTags ( f, "endObjectProperties" );
  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage ( tag.errMsg () );
  }

  scaleMin = efScaleMin.value ();
  scaleMax = efScaleMax.value ();
  if (efPrecision.isNull ()) precision = 1;
  else precision = efPrecision.value ();
  if (efScaleMin.isNull ()) scaleMin = -1e+100;
  if (efScaleMax.isNull ()) scaleMax = 1e+100;
  if (efLeadingDigits.isNull ()) leadingDigits = 0;
  else leadingDigits = efLeadingDigits.value ();

  this->initSelectBox (); // call after getting x, y, w, h

  valueD = 0;
  strcpy (valueS, "0.0");

  if (fgColorMode) fgColor.setAlarmSensitive (); else fgColor.setAlarmInsensitive ();
  if (bgColorMode) bgColor.setAlarmSensitive (); else bgColor.setAlarmInsensitive ();
  if (controlColorMode) controlColor.setAlarmSensitive (); else controlColor.setAlarmInsensitive ();

  actWin->fi->loadFontTag ( displayFontTag );
  actWin->drawGc.setFontTag ( displayFontTag, actWin->fi );
  fsD = actWin->fi->getXFontStruct ( displayFontTag );
  updateFont ( valueS, displayFontTag, &fsD, &displayFontAscent, &displayFontDescent, &displayFontHeight,
   &displayStringWidth );
  actWin->fi->loadFontTag ( titleFontTag );
  actWin->drawGc.setFontTag ( titleFontTag, actWin->fi );
  fsT = actWin->fi->getXFontStruct ( titleFontTag );
  updateFont ( title, titleFontTag, &fsT, &titleFontAscent, &titleFontDescent, &titleFontHeight,
   &titleStringWidth );

  return stat;

}


// create properties dialog box
int wheelSwitchClass::genericEdit ( void ) {

  char editTitle[32], *ptr;
  // editBufType is where properties are held

/*  if ( !eBuf ) {
    eBuf = new editBufType;
  }*/

  // sets title of the dialog box
  ptr = actWin->obj.getNameFromClass ( CLASS_NAME );
  if ( ptr )
    strncpy ( editTitle, ptr, 31 );
  else
    strncpy ( editTitle, wheelSwitchClass_str1, 31 );

  Strncat ( editTitle, wheelSwitchClass_str2, 31 );


  // set some vars
  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;
  bufTitleDisplay = titleDisplay;
  bufEfPrecision = efPrecision;
  bufEfScaleMin = efScaleMin;
  bufEfScaleMax = efScaleMax;
  bufLimitsFromDb = limitsFromDb;
  bufPrecisionFromDb = precisionFromDb;
  bufFgColor = fgColor.pixelIndex ();
  bufBgColor = bgColor.pixelIndex ();
  bufControlColor = controlColor.pixelIndex ();
//  bufOfflineColor = offlineColor.pixelIndex ();
  bufFgColorMode = fgColorMode;
  bufBgColorMode = bgColorMode;
  bufControlColorMode = controlColorMode;
  bufOuterRect = outerRect;
  strncpy ( bufTitleFontTag, titleFontTag, 63 );
  bufTitleFontTag[63] = 0;
  strncpy ( bufDisplayFontTag, displayFontTag, 63 );
  bufDisplayFontTag[63] = 0;
  bufEfLeadingDigits = efLeadingDigits;
  bufLeadingPlus = leadingPlus;
  bufBoundsVisible = boundsVisible;
  bufDisplayUnits = displayUnits;
  bufUnitsFromDb = unitsFromDb;
  strcpy (bufUnits, units);

  strcpy (bufTitle, title);
  strcpy (bufTitlePosition, titlePosition);

  if ( controlPvName.getRaw () )
    strncpy ( controlBufPvName, controlPvName.getRaw (), PV_Factory::MAX_PV_NAME );
  else
    strncpy ( controlBufPvName, "", 39 );

  strncpy ( bufDisplayFormat, displayFormat, 15 );

  // create the dialog box
  ef.create ( actWin->top, actWin->appCtx->ci.getColorMap (),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   editTitle, NULL, NULL, NULL );

  // add textfields for those vars
  ef.addTextField ( wheelSwitchClass_str3, 35, &bufX );
  ef.addTextField ( wheelSwitchClass_str4, 35, &bufY );
  ef.addTextField ( wheelSwitchClass_str5, 35, &bufW );
  ef.addTextField ( wheelSwitchClass_str6, 35, &bufH );
  ef.addTextField ( wheelSwitchClass_str17, 35, bufTitle, 63);
  ef.addOption ( wheelSwitchClass_str18, wheelSwitchClass_str19, bufTitlePosition, 15 );
  ef.addToggle ( wheelSwitchClass_str20, &bufTitleDisplay );
  //ef.addToggle ( wheelSwitchClass_str30, &bufOuterRect );
  ef.addTextField ( wheelSwitchClass_str7, 35, controlBufPvName, PV_Factory::MAX_PV_NAME );
  ef.addOption ( wheelSwitchClass_str9, wheelSwitchClass_str10, bufDisplayFormat, 15 );
  ef.addToggle ( wheelSwitchClass_str24, &bufLeadingPlus );
  ef.addTextField ( wheelSwitchClass_str23, 35, &bufEfLeadingDigits );
  ef.addTextField ( wheelSwitchClass_str8, 35, &bufEfPrecision );
  ef.addToggle ( wheelSwitchClass_str14, &bufPrecisionFromDb );
  ef.addTextField ( wheelSwitchClass_str11, 35, &bufEfScaleMin );
  ef.addTextField ( wheelSwitchClass_str12, 35, &bufEfScaleMax );
  ef.addToggle ( wheelSwitchClass_str13, &bufLimitsFromDb );
  ef.addToggle ( wheelSwitchClass_str25, &bufBoundsVisible );
  ef.addTextField ( wheelSwitchClass_str26, 35, bufUnits, 15 );
  ef.addToggle ( wheelSwitchClass_str31, &bufUnitsFromDb );
  ef.addToggle ( wheelSwitchClass_str27, &bufDisplayUnits );
  ef.addColorButton ( wheelSwitchClass_str15, actWin->ci, &fgCb, &bufFgColor );
  ef.addToggle ( wheelSwitchClass_str29, &bufFgColorMode );
  ef.addColorButton ( wheelSwitchClass_str16, actWin->ci, &bgCb, &bufBgColor );
  ef.addToggle ( wheelSwitchClass_str29, &bufBgColorMode );
  ef.addColorButton ( wheelSwitchClass_str28, actWin->ci, &controlCb, &bufControlColor );
  ef.addToggle ( wheelSwitchClass_str29, &bufControlColorMode );
//  ef.addColorButton ( wheelSwitchClass_str32, actWin->ci, &offlineCb, &bufOfflineColor );
  ef.addFontMenu ( wheelSwitchClass_str21, actWin->fi, &fmT, bufTitleFontTag );
  ef.addFontMenu ( wheelSwitchClass_str22, actWin->fi, &fmD, bufDisplayFontTag );


  return 1;

}

// pop up properties dialog box for a newly created object
int wheelSwitchClass::editCreate ( void ) {
  this->genericEdit ();
  ef.finished ( slc_edit_ok, slc_edit_apply, slc_edit_cancel_delete, this );
  ef.popup ();

  return 1;

}

// pop up properties dialog box on double click
int wheelSwitchClass::edit ( void ) {

  this->genericEdit ();
  ef.finished (slc_edit_ok, slc_edit_apply, slc_edit_cancel, this );
  ef.popup ();

  return 1;

}


// erase widget
int wheelSwitchClass::erase ( void ) {

  //if ( deleteRequest ) return 1;

  XFillRectangle ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.eraseGC (), x, y, w + 1, h + 1 );

  return 1;

}


// draw widget
int wheelSwitchClass::draw ( void ) {

if ( deleteRequest ) return 1;

strcpy (valueS, "");
if (leadingPlus) strcat (valueS, "+");
if (leadingDigits != 0) for (int i = 0; i < leadingDigits; i++) strcat (valueS, "0"); else strcat (valueS, "0");
if (!precisionFromDb) {
if (precision != 0) {
    strcat (valueS, ".");
    for (int i = 0; i < precision; i++) strcat (valueS, "0");
}} else strcat (valueS, ".0");
if (strcmp (displayFormat, "Exponential") == 0) strcat (valueS, "e+00");

  actWin->drawGc.saveFg ();

  actWin->drawGc.setFG (bgColor.pixelColor ());
  XFillRectangle ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x, y, w, h );

  actWin->drawGc.setFG (fgColor.pixelColor ());
  actWin->drawGc.setBG (bgColor.pixelColor ());

  if (outerRect) XDrawRectangle ( actWin->d, XtWindow (actWin->drawWidget),
   actWin->drawGc.normGC (), x, y, w, h );

  int incr = 0;
  int unitsStringWidth = 0;
digitNumber = strlen (valueS);
if (displayUnits && !unitsFromDb) updateFont ( units, displayFontTag, &fsD, &displayFontAscent, &displayFontDescent, &displayFontHeight, &unitsStringWidth );
halfDigit = (w - unitsStringWidth) / (2 * (digitNumber + 1 + displayUnits) + 3); // one digit space for buttons, three half digit spaces for margins
offset = halfDigit / 4;

int oldW, oldX;
oldW = w;
oldX = x;

if (titleDisplay) {
    actWin->drawGc.setFontTag ( titleFontTag, actWin->fi );
    if (strcmp (titlePosition, "Top") == 0) {
        int yy, minY;
        minY = titleFontHeight;
        if (h / 6 < minY) yy = minY; else yy = h / 6;
        XDrawImageString ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.normGC (), x + halfDigit, y + yy, title, strlen (title));
    }
    else if (strcmp (titlePosition, "Left") == 0) {
        XDrawImageString ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.normGC (), x + offset, y + h / 2 + 3, title, strlen (title));
        w = w - titleStringWidth;
        x = x + oldW - w;
        halfDigit = (w - unitsStringWidth) / (2 * (digitNumber + 1 + displayUnits) + 3);
        offset = halfDigit / 4;
    }
}

//draw buttons
XPoint points[3];
points[0].x = x + w - 3 * halfDigit;
points[0].y = y + h / 2 - offset / 2;
points[1].x = x + w - halfDigit;
points[1].y = y + h / 2 - offset / 2;
points[2].x = x + w - 2 * halfDigit;
points[2].y = y + h / 6;
  XFillPolygon ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.normGC (), points, 3, Nonconvex, CoordModeOrigin);
for (int i = 0; i < 3; i++){
  points[i].y = 2 * y + h - points[i].y;
}
  XFillPolygon ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.normGC (), points, 3, Nonconvex, CoordModeOrigin);

//draw space for digits
for (int i = 0; i < digitNumber; i++){
  XDrawRectangle ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.normGC (), x + halfDigit + halfDigit * 2 * i, y + h / 4, halfDigit * 2, h / 2);
}

//draw digits
actWin->drawGc.setFG (controlColor.pixelColor ());
char digits[digitNumber + incr][1];
for (int i = 0; i < digitNumber; i++) digits[i][0] = valueS[i];
if (!unitsFromDb) for (int i = digitNumber; i < digitNumber + incr; i++) digits[i][0] = units[i - digitNumber];
else digits[digitNumber][0] = ' ';
actWin->drawGc.setFontTag ( displayFontTag, actWin->fi );
for (int i = 0; i < digitNumber; i++){
  XDrawImageString ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.normGC (), x + halfDigit * 2 *(i + 1) - 3, y + h / 2 + displayFontHeight / 4, digits[i], 1);
}
if (displayUnits && !unitsFromDb) {
    actWin->drawGc.setFG (fgColor.getColor ());
    XDrawImageString ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.normGC (), x + halfDigit + halfDigit * 2 * (digitNumber + 1) - 3, y + h / 2 + displayFontHeight / 4, units, strlen (units));
}

//draw bounds
if (boundsVisible) {
actWin->drawGc.setFG (fgColor.pixelColor ());
if (!limitsFromDb) {
    char minS[16], maxS[16], format[16];
    int maxStringWidth, minStringWidth;
    int pp, oldPrec;
    if (precisionFromDb) {
        oldPrec = precision;
        precision = 1;
    }
    if (precision == 0) pp = -1; else pp = precision;
    if (!efScaleMin.isNull ()) {
        if (!strcmp (displayFormat, "Float")) {
            if (scaleMin >= 0 && leadingPlus) {
                if (efLeadingDigits.isNull ()) sprintf (format, "+%%.%-df", precision);
                else sprintf (format, "+%%%-d.%-df", (leadingDigits + pp + 1), precision);
            }
            else {
                if (efLeadingDigits.isNull ()) sprintf (format, "%%.%-df", precision);
                else if (scaleMin < 0) sprintf (format, "%%%-d.%-df", (leadingDigits + pp + 2), precision);
                else sprintf (format, "%%%-d.%-df", (leadingDigits + pp + 1), precision);
            }
        }
        else if (!strcmp (displayFormat, "Exponential")) {
            if (scaleMin >= 0 && leadingPlus) {
                if (efLeadingDigits.isNull ()) sprintf (format, "+%%.%-de", precision);
                else sprintf (format, "+%%%-d.%-de", (leadingDigits + pp + 1), precision);
            }
            else {
                if (efLeadingDigits.isNull ()) sprintf (format, "%%.%-de", precision);
                else if (scaleMin < 0) sprintf (format, "%%%-d.%-de", (leadingDigits + pp + 2), precision);
                else sprintf (format, "%%%-d.%-de", (leadingDigits + pp + 1), precision);
            }
        }
        sprintf (minS, format, scaleMin);
        if (minS[0] == ' ' && scaleMin < 0) {
            int i = 0;
            while (minS[i] != '-' && minS[i] != '+') i++;
            if (minS[i] == '-') {
                minS[i] = minS[0];
                minS[0] = '-';
            }
        }
        for (int i = 0; i < strlen (minS); i++) if (minS[i] == ' ') minS[i] = '0';
        updateFont ( minS, displayFontTag, &fsD, &displayFontAscent, &displayFontDescent, &displayFontHeight, &minStringWidth );
        XDrawImageString ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.normGC (), x + halfDigit, y + h / 4 * 3 + displayFontHeight, minS, strlen (minS));
    }
    if (!efScaleMax.isNull ()) {
        if (!strcmp (displayFormat, "Float")) {
            if (scaleMax >= 0 && leadingPlus) {
                if (efLeadingDigits.isNull ()) sprintf (format, "+%%.%-df", precision);
                else sprintf (format, "+%%%-d.%-df", (leadingDigits + pp + 1), precision);
            }
            else {
                if (efLeadingDigits.isNull ()) sprintf (format, "%%.%-df", precision);
                else if (scaleMax < 0) sprintf (format, "%%%-d.%-df", (leadingDigits + pp + 2), precision);
                else sprintf (format, "%%%-d.%-df", (leadingDigits + pp + 1), precision);
            }
        }
        else if (!strcmp (displayFormat, "Exponential")) {
            if (scaleMax >= 0 && leadingPlus) {
                if (efLeadingDigits.isNull ()) sprintf (format, "+%%.%-de", precision);
                else sprintf (format, "+%%%-d.%-de", (leadingDigits + pp + 1), precision);
            }
            else {
                if (efLeadingDigits.isNull ()) sprintf (format, "%%.%-de", precision);
                else if (scaleMax < 0) sprintf (format, "%%%-d.%-de", (leadingDigits + pp + 2), precision);
                else sprintf (format, "%%%-d.%-de", (leadingDigits + pp + 1), precision);
            }
        }
        sprintf (maxS, format, scaleMax);
        if (maxS[0] == ' '&& scaleMax < 0) {
            int i = 0;
            while (maxS[i] != '-' && maxS[i] != '+') i++;
            if (maxS[i] == '-') {
                maxS[i] = maxS[0];
                maxS[0] = '-';
            }
        }
        for (int i = 0; i < strlen (maxS); i++) if (maxS[i] == ' ') maxS[i] = '0';
        updateFont ( maxS, displayFontTag, &fsD, &displayFontAscent, &displayFontDescent, &displayFontHeight, &maxStringWidth );
        XDrawImageString ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.normGC (), x + halfDigit + halfDigit * 2 * strlen (valueS) - maxStringWidth, y + h / 4 * 3 + displayFontHeight, maxS, strlen (maxS));
    }
    if (precisionFromDb) precision = oldPrec;
}
else {
    XDrawRectangle ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.normGC (), x + halfDigit, y + h / 4 * 3 + 5, halfDigit * 2 * strlen (valueS) / 4, displayFontHeight );
    XDrawRectangle ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.normGC (), x + halfDigit + halfDigit * 2 * strlen (valueS) * 3 / 4, y + h / 4 * 3 + 5, halfDigit * 2 * strlen (valueS) / 4, displayFontHeight );
}
}

  actWin->drawGc.restoreFg ();
  w = oldW;
  x = oldX;

  return 1;

}

// erase active widget
int wheelSwitchClass::eraseActive ( void ) {

  if ( deleteRequest ) return 1;
  XFillRectangle ( actWin->d, XtWindow (actWin->drawWidget), actWin->drawGc.eraseGC (), 0, 0, w + 1, h + 1 );
  return 1;

}

// draw active widget
int wheelSwitchClass::drawActive ( void ) {

if ( !init ) {
      XFillRectangle ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.eraseGC (), 0, 0, w + 1, h + 1 );
      if (needToDrawUnconnected) {
        // ***** SJS Modification to fix crash 13/01/04 - replace *****
    // Arg al[1];
        // ***** by *****
    Arg al[2];
        // ***** End of SJS Modification *****
    int ac = 0;
    XtSetArg (al[ac], XmNtopShadowColor, bgColor.getDisconnected ()); ac++;
    XtSetArg (al[ac], XmNbottomShadowColor, bgColor.getDisconnected ()); ac++;
    XtSetValues (frameWidget, al, ac);
      }
}

    else if (controlExists) {
        // ***** SJS Modification to fix crash 13/01/04 - replace *****
    // Arg al[1];
        // ***** by *****
    Arg al[2];
        // ***** End of SJS Modification *****
    int ac = 0;
    XtSetArg (al[ac], XmNtopShadowColor, shadeColor.pixelColor () ); ac++;
    XtSetArg (al[ac], XmNbottomShadowColor, BlackPixel ( actWin->display (),
       DefaultScreen (actWin->display ()) ) ); ac++;
    XtSetValues (frameWidget, al, ac);

    actWin->executeGc.setLineWidth ( 1 );
  actWin->executeGc.setLineStyle ( LineSolid );

  actWin->executeGc.saveFg ();

  actWin->executeGc.setFG (bgColor.getColor ());
  XFillRectangle (actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), 0, 0, w + 1, h + 1);

  actWin->executeGc.setFG (fgColor.getColor ());
  actWin->executeGc.setBG (bgColor.getColor ());

int oldW;
oldW = w;
xx = 0;
int unitsStringWidth = 0;
if (displayUnits) updateFont ( units, displayFontTag, &fsD, &displayFontAscent, &displayFontDescent, &displayFontHeight, &unitsStringWidth );
if (titleDisplay) {
    actWin->executeGc.setFontTag ( titleFontTag, actWin->fi );
    if (strcmp (titlePosition, "Top") == 0) {
        int yy, minY;
        minY = titleFontHeight;
        if (h / 6 < minY) yy = minY; else yy = h / 6;
        XDrawImageString ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), halfDigit, yy, title, strlen (title));
    }
    else if (strcmp (titlePosition, "Left") == 0) {
        XDrawImageString ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), offset, h / 2 + 3, title, strlen (title));
        w = w - titleStringWidth;
        xx = oldW - w;
        halfDigit = (w - unitsStringWidth) / (2 * (digitNumber + 1 + displayUnits) + 3);
        offset = halfDigit / 4;
    }
}

digitNumber = strlen (valueS);
halfDigit = (w - unitsStringWidth) / (2 * (digitNumber + 1 + displayUnits) + 3); // one digit space for buttons, three half digit spaces for margins
offset = halfDigit / 4;

//draw buttons
XPoint points[3];
points[0].x = xx + w - 3 * halfDigit + offset;
points[0].y = h / 2 - offset / 2;
points[1].x = xx + w - halfDigit - offset;
points[1].y = h / 2 - offset / 2;
points[2].x = xx + w - 2 * halfDigit;
points[2].y = h / 6;
  XFillPolygon ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), points, 3, Nonconvex, CoordModeOrigin);
for (int i = 0; i < 3; i++){
  points[i].y = h - points[i].y;
}
  XFillPolygon ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), points, 3, Nonconvex, CoordModeOrigin);

//draw space for digits
for (int i = 0; i < digitNumber; i++){
  XDrawRectangle ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), xx + halfDigit + halfDigit * 2 * i, h / 4, halfDigit * 2, h / 2);
}

//draw digits
char digits[digitNumber][1];
for (int i = 0; i < digitNumber; i++) digits[i][0] = valueS[i];

actWin->executeGc.setFontTag ( displayFontTag, actWin->fi );
actWin->executeGc.setFG (controlColor.getColor ());
for (int i = 0; i < digitNumber; i++){
  XDrawImageString ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), xx + halfDigit * 2 * (i + 1) - 3, h / 2 + 3, digits[i], 1);
}
actWin->executeGc.setFG (fgColor.getColor ());
if (displayUnits) {
  XDrawImageString ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), xx + halfDigit + halfDigit * 2 * (digitNumber + 1) - 3, h / 2 + 3, units, strlen (units));
}

if (selected != -1)  {
    actWin->executeGc.setFG (bgColor.getColor ());
    XDrawRectangle ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), xx + halfDigit + oldSelected * 2 * halfDigit + offset, h / 4 + offset, 2 * (halfDigit - offset), h / 2 - 2 * offset);
    actWin->executeGc.setFG (controlColor.getColor ());
    XDrawRectangle ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), xx + halfDigit + selected * 2 * halfDigit + offset, h / 4 + offset, 2 * (halfDigit - offset), h / 2 - 2 * offset);
}

//draw bounds
actWin->executeGc.setFG (fgColor.getColor ());
if (boundsVisible) {
    char minS[16], maxS[16], format[16];
    int maxStringWidth, minStringWidth;
    actWin->executeGc.setFontTag ( displayFontTag, actWin->fi );
    int pp;
    if (precision == 0) pp = -1; else pp = precision;
    if (!efScaleMin.isNull () || limitsFromDb != 0) {
        if (!strcmp (displayFormat, "Float")) {
            if (scaleMin >= 0 && leadingPlus) {
                if (efLeadingDigits.isNull ()) sprintf (format, "+%%.%-df", precision);
                else sprintf (format, "+%%%-d.%-df", (leadingDigits + pp + 1), precision);
            }
            else {
                if (efLeadingDigits.isNull ()) sprintf (format, "%%.%-df", precision);
                else if (scaleMin < 0) sprintf (format, "%%%-d.%-df", (leadingDigits + pp + 2), precision);
                else sprintf (format, "%%%-d.%-df", (leadingDigits + pp + 1), precision);
            }
        }
        else if (!strcmp (displayFormat, "Exponential")) {
            if (scaleMin >= 0 && leadingPlus) {
                if (efLeadingDigits.isNull ()) sprintf (format, "+%%.%-de", precision);
                else sprintf (format, "+%%%-d.%-de", (leadingDigits + pp + 1), precision);
            }
            else {
                if (efLeadingDigits.isNull ()) sprintf (format, "%%.%-de", precision);
                else if (scaleMin < 0) sprintf (format, "%%%-d.%-de", (leadingDigits + pp + 2), precision);
                else sprintf (format, "%%%-d.%-de", (leadingDigits + pp + 1), precision);
            }
        }
        sprintf (minS, format, scaleMin);
        if (minS[0] == ' ' && scaleMin < 0) {
            int i = 0;
            while (minS[i] != '-' && minS[i] != '+') i++;
            if (minS[i] == '-') {
                minS[i] = minS[0];
                minS[0] = '-';
            }
        }
        for (int i = 0; i < strlen (minS); i++) if (minS[i] == ' ') minS[i] = '0';
        updateFont ( minS, displayFontTag, &fsD, &displayFontAscent, &displayFontDescent, &displayFontHeight, &minStringWidth );
        XDrawImageString ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), xx + halfDigit, h / 4 * 3 + displayFontHeight, minS, strlen (minS));
    }
    if (!efScaleMax.isNull () || limitsFromDb != 0) {
        if (!strcmp (displayFormat, "Float")) {
            if (scaleMax >= 0 && leadingPlus) {
                if (efLeadingDigits.isNull ()) sprintf (format, "+%%.%-df", precision);
                else sprintf (format, "+%%%-d.%-df", (leadingDigits + pp + 1), precision);
            }
            else {
                if (efLeadingDigits.isNull ()) sprintf (format, "%%.%-df", precision);
                else if (scaleMax < 0) sprintf (format, "%%%-d.%-df", (leadingDigits + pp + 2), precision);
                else sprintf (format, "%%%-d.%-df", (leadingDigits + pp + 1), precision);
            }
        }
        else if (!strcmp (displayFormat, "Exponential")) {
            if (scaleMax >= 0 && leadingPlus) {
                if (efLeadingDigits.isNull ()) sprintf (format, "+%%.%-de", precision);
                else sprintf (format, "+%%%-d.%-de", (leadingDigits + pp + 1), precision);
            }
            else {
                if (efLeadingDigits.isNull ()) sprintf (format, "%%.%-de", precision);
                else if (scaleMax < 0) sprintf (format, "%%%-d.%-de", (leadingDigits + pp + 2), precision);
                else sprintf (format, "%%%-d.%-de", (leadingDigits + pp + 1), precision);
            }
        }
        sprintf (maxS, format, scaleMax);
        if (maxS[0] == ' '&& scaleMax < 0) {
            int i = 0;
            while (maxS[i] != '-' && maxS[i] != '+') i++;
            if (maxS[i] == '-') {
                maxS[i] = maxS[0];
                maxS[0] = '-';
            }
        }
        for (int i = 0; i < strlen (maxS); i++) if (maxS[i] == ' ') maxS[i] = '0';
        updateFont ( maxS, displayFontTag, &fsD, &displayFontAscent, &displayFontDescent, &displayFontHeight, &maxStringWidth );
        int xpos = xx + w - halfDigit * 5 - maxStringWidth;
        xpos = xx + halfDigit + halfDigit * 2 * strlen (valueS) - maxStringWidth;
        XDrawImageString ( actWin->d, XtWindow (wheelSwitchWidget), actWin->executeGc.normGC (), xpos, h / 4 * 3 + displayFontHeight, maxS, strlen (maxS));
    }
}

  actWin->executeGc.restoreFg ();
  w = oldW;

    }
    return 1;
}

//called when widget is to become active
int wheelSwitchClass::activate ( int pass, void *ptr ) {
  int opStat;

  switch ( pass ) {
  case 2:

      opStat = 1;
      initEnable ();

      frameWidget = XtVaCreateManagedWidget ( "", xmBulletinBoardWidgetClass,
       actWin->executeWidgetId (),
       XmNx, x,
       XmNy, y,
       XmNwidth, w,
       XmNheight, h,
       XmNmarginHeight, 0,
       XmNmarginWidth, 0,
       XmNshadowThickness, 2,
       XmNtopShadowColor, shadeColor.pixelColor (),
       XmNbottomShadowColor, BlackPixel ( actWin->display (),
       DefaultScreen (actWin->display ()) ),
       XmNshadowType, XmSHADOW_ETCHED_OUT,
       XmNmappedWhenManaged, False,
       NULL );

      if ( !frameWidget ) return 0;

      wheelSwitchWidget = XtVaCreateManagedWidget ( "", xmDrawingAreaWidgetClass,
       frameWidget,
       XmNx, 2,
       XmNy, 2,
       XmNwidth, w - 4,
       XmNheight, h - 4,
       XmNbackground, bgColor.pixelColor (),
       XmNkeyboardFocusPolicy, XmPOINTER,
       NULL );

      if ( !wheelSwitchWidget ) return 0;

      XtAddEventHandler ( wheelSwitchWidget,
       KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask |
       EnterWindowMask | LeaveWindowMask, False,
       wheelSwitchEventHandler, (XtPointer) this );

      if ( frameWidget ) XtMapWidget ( frameWidget );

      controlPvConnected = 0;
      aglPtr = ptr;
      activeMode = 1;
      selected = 1;
      oldStat = -1;
      oldSev = -1;
      strcpy (oldUnits, units);
      init = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut ( actWin->appCtx->appContext (),
         2000, unconnectedTimeout, this );
      }

     if ( !controlPvName.getExpanded () || blankOrComment ( controlPvName.getExpanded () ) ) controlExists = 0;
     else {
         controlExists = 1;
         fgColor.setConnectSensitive ();
         bgColor.setConnectSensitive ();
         controlColor.setConnectSensitive ();
     }
     if ( controlExists ) {
    controlPvId = the_PV_Factory->create ( controlPvName.getExpanded () );
    if ( controlPvId ) controlPvId->add_conn_state_callback ( sl_monitor_control_connect_state, this );
    else opStat = 0;
     }

     return opStat;
    break;
  }
  return 1;
}

//called when entering back to edit mode
int wheelSwitchClass::deactivate ( int pass ) {
    activeMode = 0;
    switch (pass) {
        case 1:
            XtRemoveEventHandler ( wheelSwitchWidget,
                ButtonPressMask|ButtonReleaseMask|PointerMotionMask|ExposureMask|
            EnterWindowMask|LeaveWindowMask, False,
            wheelSwitchEventHandler, (XtPointer) this );

            if ( controlExists ) {
                if ( controlPvId ) {
                    controlPvId->remove_conn_state_callback (
                        sl_monitor_control_connect_state, this );
                    controlPvId->release ();
                    controlPvId = NULL;
                }
                controlExists = 0;
            }
            if ( unconnectedTimer ) {
            XtRemoveTimeOut ( unconnectedTimer );
            unconnectedTimer = 0;
            }
            break;
        case 2:
            if (frameWidget) {
            XtUnmapWidget (frameWidget);
            XtDestroyWidget (frameWidget);
            }
            strcpy (valueS, "0.0");
            strcpy (units, oldUnits);
            break;
    }
}

//called during execution
void wheelSwitchClass::executeDeferred ( void ) {
    if (!activeMode) return;
    int st, sev;
    st = controlPvId->get_status ();
    sev = controlPvId->get_severity ();
    if ( ( st != oldStat ) || ( sev != oldSev ) ) {
        oldStat = st;
        oldSev = sev;
        bgColor.setStatus ( st, sev );
        fgColor.setStatus ( st, sev );
        controlColor.setStatus ( st, sev );
        actWin->appCtx->proc->lock ();
        actWin->addDefExeNode ( aglPtr );
        actWin->appCtx->proc->unlock ();
    }
    if (controlExists) {
        fgColor.setConnected ();
        bgColor.setConnected ();
        controlColor.setConnected ();
        valueD = controlPvId->get_double ();
            int pp;
            if (precision == 0) pp = -1; else pp = precision;
        if (!strcmp (displayFormat, "Float")) {
            if (valueD >= 0 && leadingPlus) {
                if (efLeadingDigits.isNull ()) sprintf (format, "+%%.%-df", precision);
                else sprintf (format, "+%%%-d.%-df", (leadingDigits + pp + 1), precision);
            }
            else {
                if (efLeadingDigits.isNull ()) sprintf (format, "%%.%-df", precision);
                else if (valueD < 0) sprintf (format, "%%%-d.%-df", (leadingDigits + pp + 2), precision);
                else sprintf (format, "%%%-d.%-df", (leadingDigits + pp + 1), precision);
            }
        }
        else if (!strcmp (displayFormat, "Exponential")) {
            if (valueD >= 0 && leadingPlus) {
                if (efLeadingDigits.isNull ()) sprintf (format, "+%%.%-de", precision);
                else sprintf (format, "+%%%-d.%-de", (leadingDigits + pp + 1), precision);
            }
            else {
                if (efLeadingDigits.isNull ()) sprintf (format, "%%.%-de", precision);
                else if (valueD < 0) sprintf (format, "%%%-d.%-de", (leadingDigits + pp + 2), precision);
                else sprintf (format, "%%%-d.%-de", (leadingDigits + pp + 1), precision);
            }
        }
        sprintf (valueS, format, valueD);
        if (valueS[0] == ' ' && valueD < 0) {
            int i = 0;
            while (valueS[i] != '-' && valueS[i] != '+') i++;
            if (valueS[i] == '-') {
                valueS[i] = valueS[0];
                valueS[0] = '-';
            }
        }
        for (int i = 0; i < strlen (valueS); i++) if (valueS[i] == ' ') valueS[i] = '0';
        checkSelection (selected, this);
        if (strcmp (valueS, value2) != 0) strcpy (oldValueS, valueS);
        strcpy (value2, valueS);
        if (unitsFromDb) {
            strncpy (units, controlPvId->get_units (), MAX_UNITS_SIZE);
            units[MAX_UNITS_SIZE] = 0;
        }
        drawActive ();
    }
}

int wheelSwitchClass::expand1st ( int numMacros, char *macros[], char *expansions[] ) {
  int retStat, stat;
  retStat = 1;
  stat = controlPvName.expand1st ( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  return retStat;
}

int wheelSwitchClass::expand2nd ( int numMacros, char *macros[], char *expansions[] ) {
  int retStat, stat;
  retStat = 1;
  stat = controlPvName.expand2nd ( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  return retStat;
}

int wheelSwitchClass::containsMacros ( void ) {
  if ( controlPvName.containsPrimaryMacros () ) return 1;
}

//checks if index to be selected is a number and adjust it if it's not
int checkSelection (int selected, XtPointer client) {
    wheelSwitchClass *slo;
    char c;
        slo = (wheelSwitchClass *)client;
    c = slo->valueS[selected];
    int un = 0;
    if (slo->displayUnits) un = strlen (slo->units);
    if (strlen (slo->oldValueS) < strlen (slo->valueS)) {
        if (!(c >= '0' && c <= '9')) selected++;
        c = slo->valueS[selected];
        while (!(c >= '0' && c <= '9') && selected > 0) {
            selected--;
            c = slo->valueS[selected];
        }
        while (!(c >= '0' && c <= '9') && selected < slo->digitNumber - un) {
            selected++;
            c = slo->valueS[selected];
        }
    }
    else if (strlen (slo->oldValueS) > strlen (slo->valueS)) {
        selected--;
        c = slo->valueS[selected];
        while (!(c >= '0' && c <= '9') && selected < slo->digitNumber - un) {
            selected++;
            c = slo->valueS[selected];
        }
        while (!(c >= '0' && c<= '9') && selected > 0) {
            selected--;
            c = slo->valueS[selected];
        }
    }
    else {
        while (!(c >= '0' && c <= '9') && selected < slo->digitNumber - un) {
            selected++;
            c = slo->valueS[selected];
        }
        while (!(c >= '0' && c <= '9') && selected > 0) {
            selected--;
            c = slo->valueS[selected];
        }
    }
    if (!(c >= '0' && c <= '9')) selected = -1;
    if (slo->selected != selected) slo->selected = selected;
    return selected;
}

//event handler
void wheelSwitchEventHandler (Widget w, XtPointer client, XEvent *e, Boolean *continueToDispatch) {
  XButtonEvent *be;
  XKeyEvent *ke;
  KeySym key;
  char keyBuf[20];
  const int keyBufSize = 20;
  XComposeStatus compose;
  int charCount;
  wheelSwitchClass *slo;
  int halfDigit, bx, by, ww, wh;

  slo = (wheelSwitchClass *)client;
  halfDigit = slo->halfDigit;
  ww = slo->w;
  wh = slo->h;
  int un = 0;
  if (slo->displayUnits) un = strlen (slo->units);
  if (slo->needToDrawUnconnected || !slo->init) return;
  if (e->type == ButtonPress) {
    be = (XButtonEvent *)e;
    bx = be->x;
    by = be->y;
    switch (be->button){
    case Button1:
      for (int i = 0; i < slo->digitNumber; i++) {
        if (bx > slo->xx + halfDigit + i * 2 * halfDigit && bx < slo->xx + halfDigit + (i + 1) * 2 * halfDigit && by > wh / 4 && by < wh / 4 * 3) {
            slo->oldSelected = slo->selected;
            slo->selected = i;
            checkSelection (slo->selected, client);
        }
      }
      if (bx < ww - halfDigit && bx > ww - 3 * halfDigit) {
          if (slo->selected != -1) {
              if (by < wh / 2) updown (slo->selected, client, 1);
              else if (by > wh / 2) updown (slo->selected, client, -1);
          }
      }
      slo->drawActive ();
      break;
    case Button3:
      for (int i = 0; i < slo->digitNumber; i++) {
        if (bx > slo->xx + halfDigit + i * 2 * halfDigit && bx < slo->xx + halfDigit + (i + 1) * 2 * halfDigit && by > wh / 4 && by < wh / 4 * 3) {
            slo->oldSelected = slo->selected;
            slo->selected = i;
            checkSelection (slo->selected, client);
        }
      }
      slo->kp.create ( slo->actWin->top, be->x_root, be->y_root, "", &slo->kpInt,
         (void *) slo,
         (XtCallbackProc) xtdoSetKpIntValue,
         (XtCallbackProc) xtdoCancelKp );
      slo->drawActive ();
      break;
    case Button4:
      if (slo->selected > -1) {
        updown (slo->selected, client, 1);
        slo->drawActive ();
      }
      break;
    case Button5:
      if (slo->selected > -1) {
        updown (slo->selected, client , -1);
        slo->drawActive ();
      }
      break;
    }
  }
  else if ( e->type == KeyPress ) {
    ke = (XKeyEvent *) e;
    charCount = XLookupString ( ke, keyBuf, keyBufSize, &key, &compose );
    if (key >= '0' && key <= '9' && slo->selected >= 0) {
        setd (slo->selected, client, key - '0');
        slo->selected++;
        checkSelection (slo->selected, client);
    }
    else if (key == XK_Left) {
        slo->selected--;
        while ((slo->valueS[slo->selected] < '0' || slo->valueS[slo->selected] > '9') && slo->selected > 0) slo->selected--;
        checkSelection (slo->selected, client);
    }
    else if (key == XK_Right) {
        slo->selected++;
        checkSelection (slo->selected, client);
    }
    else if (key == XK_Up) updown (slo->selected, client, 1);
    else if (key == XK_Down) updown (slo->selected, client, -1);
  }
}

//keypad handling
static void xtdoCancelKp ( Widget w, XtPointer client, XtPointer call ) {
  wheelSwitchClass *slo = (wheelSwitchClass *) client;
  slo->editDialogIsActive = 0;
}
static void xtdoSetKpIntValue ( Widget w, XtPointer client, XtPointer call ) {
  wheelSwitchClass *slo = (wheelSwitchClass *) client;
  slo->editDialogIsActive = 0;
  setd (slo->selected, client, slo->kpInt);
}

//set specific digit
static void setd ( int which, XtPointer client, int ud ) {
    wheelSwitchClass *slo;
    slo = (wheelSwitchClass *)client;
    double m, value;
    int pr;
    char c;
    pr = slo->valueS[slo->selected] - '0';
    if (!strcmp (slo->displayFormat, "Float")) {
      m = 1;
      for (int i = 0; i < slo->precision; i++) m *= 0.1;
      for (int i = slo->digitNumber - 1; i > slo->selected; i--) {
        c = slo->valueS[i];
        if (!(c >= '0' && c <= '9')) i--;
        if (i > slo->selected) m *= 10;
      }
      if (slo->valueD < 0) m *= -1;
      value = slo->valueD + (ud - pr) * m;
    }
    else if (!strcmp (slo->displayFormat, "Exponential")) {
     int posE;
     posE = slo->digitNumber - 1;
     while (slo->valueS[posE] != 'e') posE--;
      char expS[slo->digitNumber - posE - 2];
      int exp;
      double sign;
      for (int i = 0; i < slo->digitNumber - posE - 1; i++) expS[i] = slo->valueS[posE + 2 + i];
      if (slo->valueS[posE + 1] == '+') sign = 10;
      else if (slo->valueS[posE + 1] == '-') sign = 0.1;
      exp = atoi (expS);
     if (slo->selected > posE) {
      m = 1;
      value = slo->valueD;
      int ee;
      ee = slo->digitNumber - 1 - slo->selected;
      if (slo->valueS[posE + 1] == '-') m = pow (0.1, pow (10, slo->digitNumber - slo->selected - 1));
      else if (slo->valueS[posE + 1] == '+') m = pow (10, pow (10, slo->digitNumber - slo->selected - 1));
      if (ud > pr) for (int i = pr; i < ud; i++) value *= m;
      else if (ud < pr) for (int i = ud; i < pr; i++) value *= 1 / m;
     }
     else if (slo->selected < posE) {
      m = 1;
      for (int i = 0; i < slo->precision; i++) m *= 0.1;
      for (int i = posE - 1; i > slo->selected; i--) {
        c = slo->valueS[i];
        if (!(c >= '0' && c <= '9')) i--;
        if (i>slo->selected) m *= 10;
      }
      for (int i = 0; i < exp; i++) m *= sign;
      if (slo->valueD < 0) m *= -1;
      value = slo->valueD + (ud - pr) * m;
     }
    }
    if (value>slo->scaleMax) value = slo->scaleMax;
    else if (value < slo->scaleMin) value = slo->scaleMin;
    slo->controlPvId->put (value);
    checkSelection (slo->selected, client);
}
//digit increase or decrease
static void updown ( int which, XtPointer client, int ud ) {
    wheelSwitchClass *slo;
    slo = (wheelSwitchClass *)client;
    double m = 0, value;
    char c;
    if (!strcmp (slo->displayFormat, "Float")) {
      m = 1;
      for (int i = 0; i < slo->precision; i++) m *= 0.1;
      for (int i = slo->digitNumber - 1; i > slo->selected; i--) {
        c = slo->valueS[i];
        if (!(c >= '0' && c <= '9')) i--;
        if (i > slo->selected) m *= 10;
      }
      value = slo->valueD + m * ud;
    }
    else if (!strcmp (slo->displayFormat, "Exponential")) {
     int posE;
     posE = slo->digitNumber - 1;
     while (slo->valueS[posE] != 'e') posE--;
      char expS[slo->digitNumber - posE - 2];
      int exp;
      double sign;
      for (int i = 0; i < slo->digitNumber - posE - 1; i++) expS[i] = slo->valueS[posE + 2 + i];
      if (slo->valueS[posE + 1] == '+') sign = 10;
      else if (slo->valueS[posE + 1] == '-') sign = 0.1;
      exp = atoi (expS);
     if (slo->selected>posE) {
      if (ud == 1) m = 10;
      else if (ud == -1) m = 0.1;
      m = pow (m, pow (10, slo->digitNumber - slo->selected - 1));
      value = slo->valueD * m;
     }
     else if (slo->selected < posE) {
      m = 1;
      for (int i = 0; i < slo->precision; i++) m *= 0.1;
      for (int i = posE - 1; i > slo->selected; i--) {
        c = slo->valueS[i];
        if (!(c >= '0' && c<= '9')) i--;
        if (i>slo->selected) m *= 10;
      }
      for (int i = 0; i < exp; i++) m *= sign;
      value = slo->valueD + m * ud;
     }
    }
    if (value>slo->scaleMax) value = slo->scaleMax;
    else if (value < slo->scaleMin) value = slo->scaleMin;
    slo->controlPvId->put (value);
    checkSelection (slo->selected, client);
}

//ok button on properties dialog box
static void slc_edit_ok ( Widget w, XtPointer client, XtPointer call ) {
  wheelSwitchClass *slo = (wheelSwitchClass *) client;
  slc_edit_update ( w, client, call );
/*  if ( slo->eBuf ) {
    delete slo->eBuf;
    slo->eBuf = NULL;
  }
*/  slo->ef.popdown ();
  slo->operationComplete ();
}

//apply button on properties dialog box
static void slc_edit_apply ( Widget w, XtPointer client, XtPointer call ) {
  wheelSwitchClass *slo = (wheelSwitchClass *) client;
  slc_edit_update ( w, client, call );
  slo->refresh ( slo );
}

//cancel button on properties dialog box
static void slc_edit_cancel ( Widget w, XtPointer client, XtPointer call ) {
  wheelSwitchClass *slo = (wheelSwitchClass *) client;
  /*if ( slo->eBuf ) {
    delete slo->eBuf;
    slo->eBuf = NULL;
  }*/
  slo->ef.popdown ();
  slo->operationCancel ();
}

//cancel button on newly created objects' properties dialog box
static void slc_edit_cancel_delete ( Widget w, XtPointer client, XtPointer call ) {
  wheelSwitchClass *slo = (wheelSwitchClass *) client;
  /*if ( slo->eBuf ) {
    delete slo->eBuf;
    slo->eBuf = NULL;
  }*/
  slo->ef.popdown ();
  slo->operationCancel ();
  slo->erase ();
  slo->deleteRequest = 1;
  slo->drawAll ();
}

//updates vars from properties dialog box
static void slc_edit_update ( Widget w, XtPointer client, XtPointer call ) {
    wheelSwitchClass *slo = (wheelSwitchClass *) client;
    slo->actWin->setChanged ();
    slo->erase ();
    slo->eraseSelectBoxCorners ();

    slo->x = slo->bufX;
    slo->sboxX = slo->bufX;
    slo->y = slo->bufY;
    slo->sboxY = slo->bufY;
    slo->w = slo->bufW;
    slo->sboxW = slo->bufW;
    slo->h = slo->bufH;
    slo->sboxH = slo->bufH;

    strcpy (slo->title, slo->bufTitle);
    strcpy (slo->titlePosition, slo->bufTitlePosition);
    slo->titleDisplay = slo->bufTitleDisplay;
    slo->outerRect = slo->bufOuterRect;

    strncpy ( slo->titleFontTag, slo->fmT.currentFontTag (), 63 );
    slo->titleFontTag[63] = 0;
    slo->actWin->fi->loadFontTag ( slo->titleFontTag );
    slo->actWin->drawGc.setFontTag ( slo->titleFontTag, slo->actWin->fi );
    slo->fsT = slo->actWin->fi->getXFontStruct ( slo->titleFontTag );
    slo->updateFont ( slo->title, slo->titleFontTag, &slo->fsT,
        &slo->titleFontAscent, &slo->titleFontDescent, &slo->titleFontHeight,
        &slo->titleStringWidth );
    strncpy ( slo->displayFontTag, slo->fmD.currentFontTag (), 63 );
    slo->displayFontTag[63] = 0;
    slo->actWin->fi->loadFontTag ( slo->displayFontTag );
    slo->actWin->drawGc.setFontTag ( slo->displayFontTag, slo->actWin->fi );
    slo->fsD = slo->actWin->fi->getXFontStruct ( slo->displayFontTag );
    slo->updateFont ( slo->valueS, slo->displayFontTag, &slo->fsD,
        &slo->displayFontAscent, &slo->displayFontDescent, &slo->displayFontHeight,
        &slo->displayStringWidth );

    slo->fgColor.setColorIndex ( slo->bufFgColor, slo->actWin->ci );
    slo->fgColorMode = slo->bufFgColorMode;
    slo->bgColor.setColorIndex ( slo->bufBgColor, slo->actWin->ci );
    slo->bgColorMode = slo->bufBgColorMode;
    slo->controlColor.setColorIndex ( slo->bufControlColor, slo->actWin->ci );
    slo->controlColorMode = slo->bufControlColorMode;
    //slo->offlineColor.setColorIndex ( slo->bufOfflineColor, slo->actWin->ci );
    if (slo->fgColorMode) slo->fgColor.setAlarmSensitive (); else slo->fgColor.setAlarmInsensitive ();
    if (slo->bgColorMode) slo->bgColor.setAlarmSensitive (); else slo->bgColor.setAlarmInsensitive ();
    if (slo->controlColorMode) slo->controlColor.setAlarmSensitive (); else slo->controlColor.setAlarmInsensitive ();

    slo->controlPvName.setRaw ( slo->controlBufPvName );

    slo->efPrecision = slo->bufEfPrecision;
    slo->efScaleMin = slo->bufEfScaleMin;
    slo->efScaleMax = slo->bufEfScaleMax;
    slo->limitsFromDb = slo->bufLimitsFromDb;
    slo->precisionFromDb = slo->bufPrecisionFromDb;

    slo->scaleMin = slo->efScaleMin.value ();
    slo->scaleMax = slo->efScaleMax.value ();

    if (slo->efPrecision.isNull ()) slo->precision = 1;
    else slo->precision = slo->efPrecision.value ();
    if (slo->efScaleMin.isNull ()) slo->scaleMin = -1e+100;
    if (slo->efScaleMax.isNull ()) slo->scaleMax = 1e+100;

    slo->leadingPlus = slo->bufLeadingPlus;
    slo->displayUnits = slo->bufDisplayUnits;
    slo->unitsFromDb = slo->bufUnitsFromDb;
    strcpy (slo->units, slo->bufUnits);
    slo->boundsVisible = slo->bufBoundsVisible;
    slo->efLeadingDigits = slo->bufEfLeadingDigits;
    if (slo->efLeadingDigits.isNull ()) slo->leadingDigits = 0;
    else slo->leadingDigits = slo->efLeadingDigits.value ();

    strncpy ( slo->displayFormat, slo->bufDisplayFormat, 15 );
}

//connect pv
static void sl_monitor_control_connect_state ( ProcessVariable *pv, void *userarg ) {
  wheelSwitchClass *slo = (wheelSwitchClass *) userarg;
  if ( pv->is_valid () ) {
    if ( slo->limitsFromDb ) {
      slo->scaleMin = pv->get_lower_disp_limit ();
    }
    if ( slo->limitsFromDb ) {
      slo->scaleMax = pv->get_upper_disp_limit ();
    }
    if ( slo->precisionFromDb || slo->efPrecision.isNull () ) {
      slo->precision = pv->get_precision ();
    }
    slo->init = 1;
    slo->needToDrawUnconnected = 0;
  }
  else {
      slo->fgColor.setDisconnected ();
      slo->bgColor.setDisconnected ();
      slo->controlColor.setDisconnected ();
      slo->init = 0;
      slo->needToDrawUnconnected = 1;
  }
  slo->actWin->appCtx->proc->lock ();
  slo->actWin->addDefExeNode ( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock ();
}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

wheelSwitchClass *slo = (wheelSwitchClass *) client;
if (!slo->init) {
  slo->needToDrawUnconnected = 1;
  slo->unconnectedTimer = 0;
  slo->drawActive ();
}
}

#ifdef __cplusplus
extern "C" {
#endif

void *create_wheelSwitchClassPtr ( void ) {

wheelSwitchClass *ptr;

  ptr = new wheelSwitchClass;
  return (void *) ptr;

}

void *clone_wheelSwitchClassPtr (
  void *_srcPtr )
{

wheelSwitchClass *ptr, *srcPtr;

  srcPtr = (wheelSwitchClass *) _srcPtr;

  ptr = new wheelSwitchClass ( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
