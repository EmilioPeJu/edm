//  edm - extensible display manager

//  Copyright (C) 2005 Diamond Light Source

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

/********************************************************************************/
/*                                                                              */
/*  ExtendedRelatedDisplay.cc                                                                    */
/*                                                                              */
/*                                                                              */
/********************************************************************************/
/*                                                                              */
/*  Revision history: (comment and initial revisions)                           */
/*                                                                              */
/*  vers.       revised         modified by                                     */
/*  -----       -----------     -------------------------                       */
/*  1.0      .  02-Feb-2005     Ian Gillingham - DLS                            */
/*                              1. Initial release                              */
/*                                                                              */
/*                                                                              */
/********************************************************************************/
// $Author: sjs $
// $Date: 2005/03/04 16:04:45 $
// $Id: ExtendedRelatedDisplay.h,v 1.1 2005/03/04 16:04:45 sjs Exp $
// $Name:  $
// $Revision: 1.1 $

#ifndef __ExtendedRelatedDisplay_h
#define __ExtendedRelatedDisplay_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define RDC_ORIG_POS 0
#define RDC_BUTTON_POS 1
#define RDC_PARENT_OFS_POS 2

#define RDC_MAJOR_VERSION 4
#define RDC_MINOR_VERSION 0
#define RDC_RELEASE 0

typedef struct objAndIndexTag
    {
    void *obj;
    int index;
    } objAndIndexType;

#ifdef __ExtendedRelatedDisplay_cc

#include "ExtendedRelatedDisplay.str"


#ifdef COMMENT_OUT
static char *dragName[] =
    {
    ExtendedRelatedDisplayClass_str48
    };
#endif


static void doBlink (  void *ptr );

static void unconnectedTimeout ( XtPointer client, XtIntervalId *id );

static void menu_cb ( Widget w, XtPointer client, XtPointer call );

static void relDsp_monitor_dest_connect_state ( ProcessVariable *pv, void *userarg );

static void relDsp_monitor_color_connect_state ( ProcessVariable *pv, void *userarg );

static void relDsp_monitor_enab_connect_state ( ProcessVariable *pv, void *userarg );

static void relDsp_color_value_update ( ProcessVariable *pv, void *userarg );

static void relDsp_enab_value_update ( ProcessVariable *pv, void *userarg );

static void rdc_edit_ok1 ( Widget w, XtPointer client, XtPointer call );

static void rdc_edit_ok ( Widget w, XtPointer client, XtPointer call );

static void rdc_edit_update ( Widget w, XtPointer client, XtPointer call );

static void rdc_edit_apply ( Widget w, XtPointer client, XtPointer call );

static void rdc_edit_cancel ( Widget w, XtPointer client, XtPointer call );

static void rdc_edit_cancel_delete ( Widget w, XtPointer client, XtPointer call );

#endif

class ExtendedRelatedDisplayClass : public activeGraphicClass
    {

    public:

        static const int NUMPVS = 4;
        static const int maxDsps = 24;

    private:

        friend void doBlink ( void *ptr );

        friend void unconnectedTimeout ( XtPointer client, XtIntervalId *id );

        friend void menu_cb ( Widget w, XtPointer client, XtPointer call );

        friend void relDsp_monitor_dest_connect_state ( ProcessVariable *pv, void *userarg );

        friend void relDsp_monitor_color_connect_state ( ProcessVariable *pv, void *userarg );

        friend void relDsp_monitor_enab_connect_state ( ProcessVariable *pv, void *userarg );

        friend void relDsp_color_value_update ( ProcessVariable *pv, void *userarg );

        friend void relDsp_enab_value_update ( ProcessVariable *pv, void *userarg );

        friend void openDisplay ( Widget w, XtPointer client, XtPointer call );

        friend void rdc_edit_ok1 ( Widget w, XtPointer client, XtPointer call );

        friend void rdc_edit_update1 ( Widget w, XtPointer client, XtPointer call );

        friend void rdc_edit_apply1 ( Widget w, XtPointer client, XtPointer call );

        friend void rdc_edit_cancel1 ( Widget w, XtPointer client, XtPointer call );

        friend void rdc_edit_ok ( Widget w, XtPointer client, XtPointer call );

        friend void rdc_edit_update ( Widget w, XtPointer client, XtPointer call );

        friend void rdc_edit_apply ( Widget w, XtPointer client, XtPointer call );

        friend void rdc_edit_cancel ( Widget w, XtPointer client, XtPointer call );

        friend void rdc_edit_cancel_delete ( Widget w, XtPointer client, XtPointer call );

        typedef struct bufTag
            {
            int bufUseFocus;
            int bufX;
            int bufY;
            int bufW;
            int bufH;
            int bufTopShadowColor;
            int bufBotShadowColor;
            int bufFgColor;
            int bufBgColor;
            int bufInvisible;
            int bufNoEdit;
            int bufCloseAction[maxDsps];
            int bufSetPostion[maxDsps];
            int bufAllowDups[maxDsps];
            int bufCascade[maxDsps];
            int bufPropagateMacros[maxDsps];
            char bufDisplayFileName[maxDsps][127 + 1];
            char bufSymbols[maxDsps][255 + 1];
            int bufReplaceSymbols[maxDsps];
            char bufButtonLabel[127 + 1];
            char bufLabel[maxDsps][127 + 1];
            char bufFontTag[63 + 1];
            char bufColorPvName[PV_Factory::MAX_PV_NAME + 1];
            char bufDestPvName[NUMPVS][PV_Factory::MAX_PV_NAME + 1];
            char bufSource[NUMPVS][39 + 1];
            char bufEnabPvName[PV_Factory::MAX_PV_NAME + 1];
            int bufOfsX;
            int bufOfsY;
            int bufButton3Popup;
            int bufIcon;
            } bufType, *bufPtr;

        colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;

        int numDsps, dspIndex;

        bufPtr buf;

        activeWindowClass *aw;
        int useFocus, needClose, needConnect, needUpdate, needRefresh;
        int needToDrawUnconnected, needToEraseUnconnected;
        int unconnectedTimer;

        int topShadowColor;
        int botShadowColor;
        pvColorClass fgColor, bgColor;
        int invisible, noEdit;

        int closeAction[maxDsps];
        int setPostion[maxDsps];
        int allowDups[maxDsps];
        int cascade[maxDsps];
        int propagateMacros[maxDsps];

        expStringClass displayFileName[maxDsps];

        expStringClass symbolsExpStr[maxDsps];
        char symbols[maxDsps][255 + 1];

        int replaceSymbols[maxDsps]; // else append

        expStringClass buttonLabel;

        expStringClass label[maxDsps];

        fontMenuClass fm;
        char fontTag[63 + 1];
        XmFontList fontList;
        XFontStruct *fs;
        int fontAscent, fontDescent, fontHeight;

        ProcessVariable *colorPvId, *enabPvId, *destPvId[NUMPVS];
        int initialConnection[NUMPVS];

        objAndIndexType objAndIndex[NUMPVS];

        int opComplete[NUMPVS], singleOpComplete,
            colorExists, enabExists, destExists[NUMPVS],
            atLeastOneExists, destType[NUMPVS];

        static const int enabPvConnection = 2;
        pvConnectionClass connection;

        expStringClass colorPvExpString;

        expStringClass destPvExpString[NUMPVS];

        expStringClass sourceExpString[NUMPVS];

        expStringClass enabPvExpString;

        int activeMode, active, init;

        Widget popUpMenu, pullDownMenu, pb[maxDsps];

        entryFormClass *ef1;

        int posX, posY;

        int ofsX, ofsY;

        int button3Popup;

        int icon;

    public:

        ExtendedRelatedDisplayClass ( void );

        ExtendedRelatedDisplayClass ( const ExtendedRelatedDisplayClass *source );

        ~ExtendedRelatedDisplayClass ( void );

        char *objName ( void ) { return name;}

        int createInteractive ( activeWindowClass *aw_obj, int x, int y, int w, int h );

        int save ( FILE *f );

        int old_save ( FILE *f );

        int createFromFile ( FILE *fptr, char *name, activeWindowClass *actWin );

        int old_createFromFile ( FILE *fptr, char *name, activeWindowClass *actWin );

        int importFromXchFile ( FILE *f, char *name, activeWindowClass *_actWin );

        int createSpecial ( char *fname, activeWindowClass *_actWin );

        void sendMsg ( char *param );

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

        int expand1st ( int numMacros, char *macros[], char *expansions[] );

        int expand2nd ( int numMacros, char *macros[], char *expansions[] );

        int containsMacros ( void );

        void btnUp ( XButtonEvent *be, int x, int y, int buttonState, int buttonNumber, int *action );

        void popupDisplay ( int index );

        void btnDown (
        XButtonEvent *be,
        int x,
        int y,
        int buttonState,
        int buttonNumber,
        int *action );

        int getButtonActionRequest (
        int *up,
        int *down,
        int *drag,
        int *focus );

        void changeDisplayParams (
        unsigned int flag,
        char *fontTag,
        int alignment,
        char *ctlFontTag,
        int ctlAlignment,
        char *btnFontTag,
        int btnAlignment,
        int textFgColor,
        int fg1Color,
        int fg2Color,
        int offsetColor,
        int bgColor,
        int topShadowColor,
        int botShadowColor );

        void pointerIn (
        XMotionEvent *me,
        int _x,
        int _y,
        int buttonState );

        void pointerOut (
        XMotionEvent *me,
        int _x,
        int _y,
        int buttonState );

        void mousePointerIn (
        XMotionEvent *me,
        int _x,
        int _y,
        int buttonState );

        void mousePointerOut (
        XMotionEvent *me,
        int _x,
        int _y,
        int buttonState );

        void executeDeferred ( void );

        static void enabPvConnectStateCallback (ProcessVariable *pv, void *userarg);
        static void enabPvValueCallback (ProcessVariable *pv, void *userarg);

    };

#ifdef __cplusplus
extern "C" {
#endif

void *create_ExtendedRelatedDisplayClassPtr ( void );
void *clone_ExtendedRelatedDisplayClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
