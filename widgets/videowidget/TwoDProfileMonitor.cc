// g++ -shared -o TwoDProfileMonitor.so TwoDProfileMonitor.cc -g -O -Wall -ansi
// -pedantic -I/ade/epics/supTop/extensions/R3.14.4/src/edm/util
// -I/ade/epics/supTop/extensions/R3.14.4/src/edm/lib 
// -I/ade/epics/supTop/extensions/R3.14.4/src/edm/pvlib
// -I /ade/epics/supTop/base/R3.14.4/include -I/usr/X11R6/include
// -L/usr/X11R6/lib widget.cc -lXpm

// To Do: class initialization, serialization, configuration (all related)
// Any chance of allowing overlapped widgets to be on top?

#define VIDEO_MAX_LOAD_FACTOR 4
#define VIDEO_MAX_DATA_WIDTH 10000
#define VIDEO_MAX_DATA_HEIGHT 10000
#define VIDEO_MAJOR_VERSION 4
#define VIDEO_MINOR_VERSION 2
#define VIDEO_RELEASE 1

#include <time.h>
#include <stream.h>

#include <act_grf.h>
#include <act_win.h>
#include <app_pkg.h>
#include <entry_form.h>
#include <pv_factory.h>

#include "widget.h"

// our widget class
class TwoDProfileMonitor : public activeGraphicClass
{

    // standard colours for various PV states
    pvColorClass pvColour; 

    // width of data (fixed or from PV)
    int dataWidth, dataHeight;
    int maxDataWidth, maxDataHeight;
    int widthOffset, heightOffset;
    int gridSize;
    int pvBasedDataSize;
    int pvBasedOffsets;
    int pvBasedGridSize;
    int useFalseColour, pvBasedUseFalseColour;
    int showGrid, pvBasedShowGrid;
    int gridColour, pvBasedGridColour;
    // globals for the edit popup
    int xBuf;
    int yBuf;
    int wBuf;
    int hBuf;
    int maxDataWidthBuf;
    int maxDataHeightBuf;
    char dataPvBuf[activeGraphicClass::MAX_PV_NAME+1];
    char widthPvBuf[activeGraphicClass::MAX_PV_NAME+1];
    char heightPvBuf[activeGraphicClass::MAX_PV_NAME+1];
    char widthOffsetPvBuf[activeGraphicClass::MAX_PV_NAME+1];
    char heightOffsetPvBuf[activeGraphicClass::MAX_PV_NAME+1];
    char gridSizePvBuf[activeGraphicClass::MAX_PV_NAME+1];
    char useFalseColourPvBuf[activeGraphicClass::MAX_PV_NAME+1];
    char showGridPvBuf[activeGraphicClass::MAX_PV_NAME+1];
    char gridColourPvBuf[activeGraphicClass::MAX_PV_NAME+1];

    expStringClass dataPvStr, widthPvStr, heightPvStr, useFalseColourPvStr;
    expStringClass widthOffsetPvStr, heightOffsetPvStr, gridSizePvStr;
    expStringClass showGridPvStr;
    expStringClass gridColourPvStr;
    ProcessVariable *dataPv, *widthPv, *heightPv, *useFalseColourPv;
    ProcessVariable *widthOffsetPv, *heightOffsetPv, *gridSizePv;
    ProcessVariable *showGridPv;
    ProcessVariable *gridColourPv;

    // text stuff (for edit mode drawing)
    char *textFontTag;
    int textAlignment;
    int textColour;

    int initialDataConnection, initialWidthConnection, initialHeightConnection;
    int initialWidthOffsetConnection, initialHeightOffsetConnection;
    int initialGridSizeConnection;
    int initialUseFalseColourConnection, initialShowGridConnection;
    int initialGridColourConnection;
    int needConnectInit, needInfoInit, needDraw, needRefresh;
    unsigned int  pvNotConnectedMask;
    int dataPvExists, widthPvExists, heightPvExists, useFalseColourPvExists;
    int widthOffsetPvExists, heightOffsetPvExists, gridSizePvExists;
    int showGridPvExists;
    int gridColourPvExists;
    int init, active, activeMode;
    struct timeval lasttv;
    unsigned long average_time_usec;

    // widget-specific stuff
    widgetData wd;
    Widget twoDWidget;

    // constructor "helper" function
    void constructCommon (void);

public:

    // constructors/destructor
    TwoDProfileMonitor (void);
    TwoDProfileMonitor (const TwoDProfileMonitor &s);
    virtual ~TwoDProfileMonitor (void);

    // Called when the data process variable connects or disconnects
    static void monitorDataConnectState (ProcessVariable *pv,
                                         void *userarg );
    // Called when the width process variable connects or disconnects
    static void monitorWidthConnectState (ProcessVariable *pv,
                                          void *userarg );
    // Called when the height process variable connects or disconnects
    static void monitorHeightConnectState (ProcessVariable *pv,
                                          void *userarg );
    // Called when the use false colour process variable connects or disconnects
    static void monitorUseFalseColourConnectState (ProcessVariable *pv,
                                                   void *userarg );
    // Called when the show grid process variable connects or disconnects
    static void monitorShowGridConnectState (ProcessVariable *pv,
                                             void *userarg );
    // Called when the grid colour process variable connects or disconnects
    static void monitorGridColourConnectState (ProcessVariable *pv,
                                               void *userarg );
    // Called when the width offset process variable connects or disconnects
    static void monitorWidthOffsetConnectState (ProcessVariable *pv,
                                                void *userarg );
    // Called when the height offset process variable connects or disconnects
    static void monitorHeightOffsetConnectState (ProcessVariable *pv,
                                                 void *userarg );
    // Called when the grid size process variable connects or disconnects
    static void monitorGridSizeConnectState (ProcessVariable *pv,
                                             void *userarg );
  
    // Called when the value of any of the data, use false colour, show grid,
    // width offset, height offset or grid size process variables
    // changes
    static void dataUpdate (ProcessVariable *pv,
                            void *userarg );
    
    // Called when the value of the width or height process variable changes
    static void sizeUpdate (ProcessVariable *pv,
                            void *userarg );
  
    static void grabButtonEvent (Widget w, XtPointer closure, XEvent* event,
                                 Boolean* b)
    {
    
        TwoDProfileMonitor *me = (TwoDProfileMonitor *) closure;
        // normalize dimensions (button and motion events store x, y in the
        // same location)
        event->xbutton.x += me->getX0 ();
        event->xbutton.y += me->getY0 ();
    
        // now send this event to EDM
        XtDispatchEventToWidget (me->actWin->executeWidget, event);
        *b = False; // terminate this "dispatch path"
    }
  

    virtual int draw ( void ); 
  
    // called in response to "cut" command
    virtual int erase ( void );  
  
    virtual int activate (int pass);
  
    // apply the results of either "Apply" or "OK" buttons
    void applyEditChanges (void);
  
    // user hit the "OK" button on the edit popup
    static void editOK (Widget w,
                        XtPointer client,
                        XtPointer call );
    // user hit the "Apply" button on the edit popup
    static void editApply (Widget w,
                           XtPointer client,
                           XtPointer call );
    // user hit the "Cancel" button on the edit popup
    static void editCancel (Widget w,
                            XtPointer client,
                            XtPointer call );
 
    // user hit the "Cancel" button on the edit popup during widget creation
    static void editCancelCreate (Widget w,
                                  XtPointer client,
                                  XtPointer call );
 
    // "helper" function for editing widget under a variety of circumstances
    void editCommon ( activeWindowClass *actWin, entryFormClass *ef,
                      int create = 0 ) ;

    // user created object from GUI (after drawing rectangle)
    virtual int createInteractive (activeWindowClass *actWin,
                                   int x,
                                   int y,
                                   int w,
                                   int h );

    // object created from saved description on disk
    virtual int createFromFile (FILE *fptr,
                                char *name,
                                activeWindowClass *actWin );

    // object created from ????
    virtual int importFromXchFile (FILE *fptr,
                                   char *name,
                                   activeWindowClass *actWin );

    // save to disk
    virtual int save ( FILE *fptr );

    virtual int edit ( void );
  
    // ========================================================
    // execute mode widget functions
  
    virtual int deactivate ( int pass );
  
    virtual int initDefExeNode ( void *ptr )
    { 
        aglPtr = ptr; /* why isn't this done for me? */
        return activeGraphicClass::initDefExeNode (ptr);
    }
  
    virtual int expand1st (int numMacros,
                           char *macros[],
                           char *expansions[] ) 
    {
        int stat; 
        stat = dataPvStr.expand1st (numMacros, macros, expansions);
        if (stat)
            stat = widthPvStr.expand1st (numMacros, macros, expansions);
        if (stat)
            stat = heightPvStr.expand1st (numMacros, macros, expansions);
        if (stat)
            stat = widthOffsetPvStr.expand1st (numMacros, macros, expansions);
        if (stat)
            stat = heightOffsetPvStr.expand1st (numMacros, macros, expansions);
        if (stat)
            stat = gridSizePvStr.expand1st (numMacros, macros, expansions);
        if (stat)
            stat = useFalseColourPvStr.expand1st (numMacros, macros,
                                                  expansions);
        if (stat)
            stat = showGridPvStr.expand1st (numMacros, macros,
                                            expansions);
        if (stat)
            stat = gridColourPvStr.expand1st (numMacros, macros,
                                            expansions);
        return stat;
    
    }
  
    // currently only used by mux devices (which we are not) 
    virtual int expand2nd (int numMacros,
                           char *macros[],
                           char *expansions[] )
    {
        int stat; 
        stat = dataPvStr.expand2nd (numMacros, macros, expansions);
        if (stat)
            stat = widthPvStr.expand2nd (numMacros, macros, expansions);
        if (stat)
            stat = heightPvStr.expand2nd (numMacros, macros, expansions);
        if (stat)
            stat = widthOffsetPvStr.expand2nd (numMacros, macros, expansions);
        if (stat)
            stat = heightOffsetPvStr.expand2nd (numMacros, macros, expansions);
        if (stat)
            stat = gridSizePvStr.expand2nd (numMacros, macros, expansions);
        if (stat)
            stat = useFalseColourPvStr.expand2nd (numMacros, macros,
                                                  expansions);
        if (stat)
            stat = showGridPvStr.expand2nd (numMacros, macros,
                                            expansions);
        if (stat)
            stat = gridColourPvStr.expand2nd (numMacros, macros,
                                            expansions);
        return stat;
    
    }
  
    // currently only used by mux devices (which we are not) 
    virtual int containsMacros ( void )
    {
        return dataPvStr.containsPrimaryMacros () ? 1 : 0;
    }

    template<class T> double * to_double (unsigned size, const T * data)
    {
        double * temp = (double *) malloc (sizeof (double) * size);
        for (unsigned s = 0; s < size; ++s)
        {
            temp[s] = data[s];
        }
        return temp;
    }

    static double *int_to_double (size_t s, const int *i)
    {

        double *d = (double *) malloc (sizeof (double)* s);
        if (!d) return d;
        for (size_t index = 0; index < s; index++)
        {
            d[index] = i[index];
        }
        return d;
    }
    // here is where we deal with updating the execute-mode widget (data and
    //  connect state)
    virtual void executeDeferred ( void )
    {
        int ni, nc, nr;
        struct timeval tv;
#ifdef DEBUG
        printf ("Start of TwoDProfMon::executeDeferred\n");
#endif
        if (actWin->isIconified) 
        {
#ifdef DEBUG
             printf ("TwoDProfMon::execDeferred return - window iconified\n");
#endif
             return;
        }
        // The widget may not be able to handle video data as fast as it is
        // produced (particularly if it is displaying a large image on a
        // remote X display).  If this happens, unprocessed channel access
        // data is queued for transmission in the IOC and eventually some 
        // has to be discarded.  What is lost is not necessarily video data, 
        // with the result that other edm widgets appear to stop working.
        // To avoid this happening, we must make sure we receive all screen
        // images that are produced and ignore any we haven't time to put out.
        // The following code does this.
        gettimeofday (&tv, 0);
        unsigned long elapsedusec = (tv.tv_sec - lasttv.tv_sec) * 1000000
                                     + tv.tv_usec - lasttv.tv_usec;
#ifdef DEBUG
        printf ("TwoDProfMon::executeDeferred - elapsed time = %lu\n",
                elapsedusec);
#endif
        if (elapsedusec < average_time_usec * VIDEO_MAX_LOAD_FACTOR)
        {
#ifdef DEBUG
            printf ("TwoDProfMon::execDef return - elapsed time too short\n");
#endif
            return;
        }
        lasttv.tv_sec = tv.tv_sec;
        lasttv.tv_usec = tv.tv_usec;

#ifdef DEBUG
        printf ("Start of processing for TwoDProfMon::executeDeferred\n");
#endif
        actWin->appCtx->proc->lock ();
        nc = needConnectInit; needConnectInit = 0;
        ni = needInfoInit; needInfoInit = 0;
        nr = needRefresh; needRefresh = 0;
        actWin->remDefExeNode (aglPtr);
        actWin->appCtx->proc->unlock ();
        if (!activeMode)
        {
#ifdef DEBUG
            printf ("TwoDProfMon::executeDeferred return - not active mode\n");
#endif
            return;
        }
        lasttv.tv_sec = tv.tv_sec;
        lasttv.tv_usec = tv.tv_usec;


        if (nc)
        {
            ni = 1;
        }
 
        if (ni)
        {
            active = 1;
            init = 1;
            if (initialDataConnection)
            {
                initialDataConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add data value callback\n");
#endif
                dataPv->add_value_callback ( dataUpdate, this );
            }
            if (initialWidthConnection)
            {
                initialWidthConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add width value callback\n");
#endif
                widthPv->add_value_callback ( sizeUpdate, this );
            }
            if (initialHeightConnection)
            {
                initialHeightConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add height value callback\n");
#endif
                heightPv->add_value_callback ( sizeUpdate, this );
            }
            if (initialWidthOffsetConnection)
            {
                initialWidthOffsetConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add width offset value"
                    " callback\n");
#endif
                widthOffsetPv->add_value_callback ( dataUpdate, this );
            }
            if (initialHeightOffsetConnection)
            {
                initialHeightOffsetConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add height offset value"
                    " callback\n");
#endif
                heightOffsetPv->add_value_callback ( dataUpdate, this );
            }
            if (initialGridSizeConnection)
            {
                initialGridSizeConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add grid size value"
                    " callback\n");
#endif
                gridSizePv->add_value_callback ( dataUpdate, this );
            }
            if (initialUseFalseColourConnection)
            {
                initialUseFalseColourConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add use false colour"
                    " value callback\n");
#endif
                useFalseColourPv->add_value_callback ( dataUpdate,
                                                       this );
            }
            if (initialShowGridConnection)
            {
                initialShowGridConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add show grid value"
                    " callback\n");
#endif
                showGridPv->add_value_callback ( dataUpdate,
                                                 this );
            }
            if (initialGridColourConnection)
            {
                initialGridColourConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add grid colour value"
                    " callback\n");
#endif
                gridColourPv->add_value_callback ( dataUpdate,
                                                   this );
            }
        }
        
        // need to check if we're being updated because of width change
#ifdef DEBUG
        printf ("executeDeferred (TwoDMon.cc) - pvBasedDataWidth = %d\n",
                 pvBasedDataWidth);
        printf ("executeDeferred (TwoDMon.cc) - widthPv = %x\n", widthPv);
        printf ("executeDeferred (TwoDMon.cc) - is_valid = %d\n",
                 widthPv->is_valid ());
#endif
        if (pvBasedDataSize && widthPv && widthPv->is_valid ())
        {
#ifdef DEBUG
            printf ("executeDeferred (TwoDMon.cc) - pv based width\n");
#endif
            switch (widthPv->get_type ().type)
            {
            case ProcessVariable::Type::real:
                dataWidth = (int) widthPv->get_double ();
                break;
            case ProcessVariable::Type::integer:
                dataWidth = (int) widthPv->get_int ();
                break;
            default:
                dataWidth = -1;
                break;
            }
#ifdef DEBUG
            printf ("executeDeferred (TwoDMon.cc) - width = %d\n", dataWidth);
#endif
            if (heightPv && heightPv->is_valid ())
            {
                switch (heightPv->get_type ().type)
                {
                case ProcessVariable::Type::real:
                    dataHeight = (int) heightPv->get_double ();
                    break;
                case ProcessVariable::Type::integer:
                    dataHeight = (int) heightPv->get_int ();
                    break;
                default:
                    dataHeight = -1;
                }
            }
        }
        if (pvBasedOffsets)
        {
            if (widthOffsetPv && widthOffsetPv->is_valid ())
            {
#ifdef DEBUG
                printf ("executeDeferred (TwoDMon.cc) - pv based width"
                        " offset\n");
#endif
                switch (widthOffsetPv->get_type ().type)
                {
                case ProcessVariable::Type::real:
                    widthOffset = (int) widthOffsetPv->get_double ();
                    break;
                case ProcessVariable::Type::integer:
                    widthOffset = (int) widthOffsetPv->get_int ();
                    break;
                default:
                    widthOffset = 0;
                    break;
                }
#ifdef DEBUG
                printf ("executeDeferred (TwoDMon.cc) - width offset = %d\n",
                        widthOffset);
#endif
            }
            if (heightOffsetPv && heightOffsetPv->is_valid ())
            {
#ifdef DEBUG
                printf ("executeDeferred (TwoDMon.cc) - pv based height"
                        " offset\n");
#endif
                switch (heightOffsetPv->get_type ().type)
                {
                case ProcessVariable::Type::real:
                    heightOffset = (int) heightOffsetPv->get_double ();
                    break;
                case ProcessVariable::Type::integer:
                    heightOffset = (int) heightOffsetPv->get_int ();
                    break;
                default:
                    heightOffset = 0;
                    break;
                }
#ifdef DEBUG
                printf ("executeDeferred (TwoDMon.cc) - height offset = %d\n",
                        heightOffset);
#endif
            }
        }
        if (pvBasedGridSize && gridSizePv && gridSizePv->is_valid ())
        {
#ifdef DEBUG
            printf ("executeDeferred (TwoDMon.cc) - pv based grid size\n");
#endif
            switch (gridSizePv->get_type ().type)
            {
            case ProcessVariable::Type::real:
                gridSize = (int) gridSizePv->get_double ();
                break;
            case ProcessVariable::Type::integer:
                gridSize = (int) gridSizePv->get_int ();
                break;
            default:
                gridSize = 0;
                break;
            }
#ifdef DEBUG
            printf ("executeDeferred (TwoDMon.cc) - grid size = %d\n",
                    grid size);
#endif
        }
        if (pvBasedUseFalseColour && useFalseColourPv &&
            useFalseColourPv->is_valid ())
        {
            switch (useFalseColourPv->get_type ().type)
            {
            case ProcessVariable::Type::real:
                useFalseColour = (int) useFalseColourPv->get_double ();
                break;
            case ProcessVariable::Type::integer:
                useFalseColour = (int) useFalseColourPv->get_int ();
                break;
            case ProcessVariable::Type::enumerated:
                useFalseColour = (int) useFalseColourPv->get_int ();
                break;
            default:
                useFalseColour = 0;
            }
        }
        if (showGridPv && showGridPv->is_valid ())
        {
            switch (showGridPv->get_type ().type)
            {
            case ProcessVariable::Type::real:
                showGrid = (int) showGridPv->get_double ();
                break;
            case ProcessVariable::Type::integer:
                showGrid = (int) showGridPv->get_int ();
                break;
            case ProcessVariable::Type::enumerated:
                showGrid = (int) showGridPv->get_int ();
                break;
            default:
                showGrid = 0;
            }
        }
        if (gridColourPv && gridColourPv->is_valid ())
        {
            switch (gridColourPv->get_type ().type)
            {
            case ProcessVariable::Type::real:
                gridColour = (int) gridColourPv->get_double ();
                break;
            case ProcessVariable::Type::integer:
                gridColour = (int) gridColourPv->get_int ();
                break;
            case ProcessVariable::Type::enumerated:
                gridColour = (int) gridColourPv->get_int ();
                break;
            default:
                gridColour = 0;
            }
        }
    
        if (dataWidth <= 0 || dataWidth > VIDEO_MAX_DATA_WIDTH ||
            dataHeight < 0 || dataHeight > VIDEO_MAX_DATA_HEIGHT ||
            widthOffset < 0 || widthOffset > VIDEO_MAX_DATA_WIDTH ||
            heightOffset < 0 || heightOffset > VIDEO_MAX_DATA_WIDTH ||
            gridSize < 2 || // 0 would crash, 1 would obliterate image.
            useFalseColour < 0 || useFalseColour > 1 ||
            showGrid < 0 || showGrid > 1 ||
            gridColour < 0 || gridColour > 1)
        {
            printf (
                "TwoDProfMon::execDef - return dataWidth %d dataHeight %d"
                " useFalseColour %d showGrid %d\n",
                dataWidth, dataHeight, useFalseColour, showGrid);
            printf (
                "TwoDProfMon::execDef - widthOffset %d heightOffset %d"
                " gridSize %d gridColour %d\n",
                widthOffset, heightOffset, gridSize, gridColour);
            return; 
        }   
        if ((dataHeight != 0) &&
            ((dataHeight * dataWidth) != (int)dataPv->get_dimension ()))
        {
            // Data height or width has changed.  Ignore the current data which
            // is the wrong size.  Cancel subscription and restart it to make
            // IOC / Channel Access send data of the correct size.
#ifdef DEBUG
            printf (
                "TwoDProfMon::execDef - resubscribe - new dataW %d dataH %d\n",
                dataWidth, dataHeight);
#endif
            actWin->appCtx->proc->lock ();
            dataPv->remove_value_callback ( dataUpdate, this );
            dataPv->remove_conn_state_callback (monitorDataConnectState, this);
            dataPv->release ();
            dataPv = the_PV_Factory->create ( dataPvStr.getExpanded () );
            dataPv->add_conn_state_callback ( monitorDataConnectState, this );
            dataPv->add_value_callback ( dataUpdate, this );
            actWin->appCtx->proc->unlock ();
            return;
        }
        actWin->appCtx->proc->lock ();

#ifdef DEBUG
        printf ("executeDeferred (TwoDMon.cc) - before data PV type switch\n");
#endif
        if (dataPv && dataPv->is_valid ())
        {
      
            switch (dataPv->get_type ().type)
            {
        
            case ProcessVariable::Type::real:
                // printf ("real\n");
                widgetNewDisplayData (
                    wd, dataPv->get_time_t (), dataPv->get_nano (),
                    (unsigned long) w, (unsigned long) h, dataWidth,
                    (dataHeight > 0 ? dataHeight 
                                    : dataPv->get_dimension () / dataWidth),
                    maxDataWidth,
                    maxDataHeight,
                    widthOffset,
                    heightOffset,
                    gridSize,
                    (const double *) dataPv->get_double_array (),
                    useFalseColour,
                    showGrid,
                    gridColour
                    );
                break;

            case ProcessVariable::Type::text:
                {
                    double * temp = to_double<char>(
                                        dataPv->get_dimension (),
                                        dataPv->get_char_array ());
#ifdef DEBUG
                    printf ("TwoDProfMon::execDef calling widgetNewDispData\n");
#endif
                    widgetNewDisplayData (
                        wd, dataPv->get_time_t (), dataPv->get_nano (),
                        (unsigned long)w, (unsigned long) h, dataWidth,
                        (dataHeight > 0 ? dataHeight 
                                        : dataPv->get_dimension () / dataWidth),
                        maxDataWidth,
                        maxDataHeight,
                        widthOffset,
                        heightOffset,
                        gridSize,
                        temp,
                        useFalseColour,
                        showGrid,
                        gridColour
                        );
                   free (temp);
                }
                break;

            case ProcessVariable::Type::integer:
                // printf ("int\n");
                {
                    double* temp = int_to_double (dataPv->get_dimension (),
                                                  dataPv->get_int_array ());
                    widgetNewDisplayData (
                        wd, dataPv->get_time_t (), dataPv->get_nano (),
                        (unsigned long) w, (unsigned long) h, dataWidth,
                        (dataHeight > 0 ? dataHeight 
                                        : dataPv->get_dimension () / dataWidth),
                        maxDataWidth,
                        maxDataHeight,
                        widthOffset,
                        heightOffset,
                        gridSize,
                        temp,
                        useFalseColour,
                        showGrid,
                        gridColour
                        );
                    free (temp);
                }
                break;

            default:
                // nothing to do!
                break;
            }
            widgetNewDisplayInfo (wd, true, dataPv->get_status (),
                                  dataPv->get_severity ());
        }
        else
        {
            widgetNewDisplayInfo (wd, false, 0, 0);
        }
    
        // actWin->remDefExeNode (aglPtr);
    
        actWin->appCtx->proc->unlock ();
    
        // Get approx average elapsed time for call - no point in being precise
        gettimeofday (&tv, 0);
        elapsedusec = (tv.tv_sec - lasttv.tv_sec) * 1000000 +
                       tv.tv_usec - lasttv.tv_usec;
        if (!average_time_usec)
            average_time_usec = elapsedusec;
        else
            average_time_usec = (average_time_usec * 9 + elapsedusec) / 10;
#ifdef DEBUG
        printf ("End of TwoDProfMon::execDef - average elapsed time = %lu\n",
                average_time_usec);
#endif
    }
  
    // let the user select among a field of functional names for drag-n-drop
    virtual char *firstDragName ( void ){ return "data PV"; };
    virtual char *nextDragName ( void ){ return NULL; } ;
  

    virtual char *dragValue ( int i )
    { return i ? NULL : dataPvStr.getExpanded (); };
  
    // yes we use PVs, therefore we support drag-n-drop and info dialogs
    virtual int atLeastOneDragPv (int x,
                                  int y ){ return 1; };
 
    // this one is to support an info dialog about widget-related PVs
    virtual void getPvs (int max,
                         ProcessVariable *pvs[],
                         int *n ){ *n = 1; pvs[0] = dataPv;};
  
    // This is a funny interface. It seems that the idea is to have a generic
    // interface to all widgets with a "standard" set of parameters (e.g.
    // control PV).  However, there doesn't seem to be a clean way to associate
    // widget values with their generic equivalents. I.e. the generic "control
    // PV" name is maintained in actWin->allSelectedCtlPvName[0]. One *could*
    // use that data storage for the control PV for a widget, but it is not
    // obvious (to me) that that interface is encouraged (or is guaranteed to
    // be supported in the future).

    // what I will do here is what most of the widgets do, which is to pick
    // out of the user supplied values anything that I see an obvious equivalent
    // in my "private" parameters, and copy in that data. Note that my "private"
    // equivalent might change (via the "edit" popup, and that change will not
    // be reflected in this popup. I *could* "blank out" the fields I use after
    // copying out the data, but that is not the behavior implemented in other
    // widgets.

    // I think that the *good* thing about this interface is that it hints at
    // what parameters each widget should support

    virtual void changePvNames (int flag,
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

        if ((flag & ACTGRF_READBACKPVS_MASK) && numReadbackPvs)
            dataPvStr.setRaw (readbackPvs[0]);

    }

    // see previous comments
  
    virtual void changeDisplayParams (unsigned int flag,
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
                                      int botShadowColour )
    {

        if (flag & ACTGRF_FONTTAG_MASK) textFontTag = fontTag; // strcpy???
        if (flag & ACTGRF_ALIGNMENT_MASK) textAlignment = alignment; 
        if (flag & ACTGRF_TEXTFGCOLOR_MASK) textColour = textFgColour;
    }
  
private:
  
    TwoDProfileMonitor &operator=(const TwoDProfileMonitor &s);
  
};


// class for read/write tags
// I like to break this out because it forces me to
// enumerate all the data memebers that are saved
// It is more work, but less error-prone (IMHO)
class TwoDProfileMonitorTags : public tagClass 
{

public:
    TwoDProfileMonitorTags (void){ init (); }
    ~TwoDProfileMonitorTags (){}
  
    int read (TwoDProfileMonitor* mon,
              FILE *fptr,
              int *x, int *y, int *w, int *h,
              expStringClass *dataPvStr,
              expStringClass *widthPvStr,
              expStringClass *heightPvStr,
              expStringClass *widthOffsetPvStr,
              expStringClass *heightOffsetPvStr,
              expStringClass *gridSizePvStr,
              expStringClass *useFalseColourPvStr,
              expStringClass *showGridPvStr,
              expStringClass *gridColourPvStr,
              int *pvBasedDataSize,
              int *maxDataWidth,
              int *maxDataHeight,
              int *pvBasedOffsets,
              int *pvBasedGridSize,
              int *pvBasedUseFalseColour,
              int *pvBasedShowGrid,
              int *pvBasedGridColour)
    {
        int major, minor, release;
        int stat;
        loadR ("beginObjectProperties" );
        loadR ( "major", &major );
        loadR ( "minor", &minor );
        loadR ( "release", &release );
        loadR ( "x", x );
        loadR ( "y", y );
        loadR ( "w", w );
        loadR ( "h", h );
        loadR ( "dataPvStr", dataPvStr, (char *) "" );
        loadR ( "widthPvStr", widthPvStr, (char *) "" );
        loadR ( "heightPvStr", heightPvStr, (char *) "" ); 
        loadR ( "widthOffsetPvStr", widthOffsetPvStr, (char *) "" );
        loadR ( "heightOffsetPvStr", heightOffsetPvStr, (char *) "" );
        loadR ( "gridSizePvStr", gridSizePvStr, (char *) "" );
        loadR ( "useFalseColourPvStr", useFalseColourPvStr, (char *) "" ); 
        loadR ( "showGridPvStr", showGridPvStr, (char *) "" ); 
        loadR ( "gridColourPvStr", gridColourPvStr, (char *) "" ); 
        loadR ( "pvBasedDataSize", pvBasedDataSize);
        loadR ( "maxDataWidth", maxDataWidth);
        loadR ( "maxDataHeight", maxDataHeight);
        loadR ( "pvBasedOffsets", pvBasedOffsets);
        loadR ( "pvBasedGridSize", pvBasedGridSize);
        loadR ( "pvBasedUseFalseColour", pvBasedUseFalseColour);
        loadR ( "pvBasedShowGrid", pvBasedShowGrid);
        loadR ( "pvBasedGridColour", pvBasedGridColour);
        stat = readTags ( fptr, "endObjectProperties" );
        if (major > VIDEO_MAJOR_VERSION ||
            (major == VIDEO_MAJOR_VERSION && minor > VIDEO_MINOR_VERSION))
        {
             // edl file was produced by a more recent version of edm than 
             // this and we can't predict the future
             mon->postIncompatable ();
             return 0;
        }
        if (major < VIDEO_MAJOR_VERSION)
        {
             // Major version changes render old edl files incompatible
             mon->postIncompatable ();
             return 0;
        }
        return stat;
    }
    int write (FILE *fptr,
               int *x, int *y, int *w, int *h,
               expStringClass *dataPvStr,
               expStringClass *widthPvStr,
               expStringClass *heightPvStr,
               expStringClass *widthOffsetPvStr,
               expStringClass *heightOffsetPvStr,
               expStringClass *gridSizePvStr,
               expStringClass *useFalseColourPvStr,
               expStringClass *showGridPvStr,
               expStringClass *gridColourPvStr,
               int *pvBasedDataSize,
               int *maxDataWidth,
               int *maxDataHeight,
               int *pvBasedOffsets,
               int *pvBasedGridSize,
               int *pvBasedUseFalseColour,
               int *pvBasedShowGrid,
               int *pvBasedGridColour)
    {
        int major, minor, release;
        major = VIDEO_MAJOR_VERSION;
        minor = VIDEO_MINOR_VERSION;
        release = VIDEO_RELEASE;
        loadW ("beginObjectProperties" );
        loadW ( "major", &major );
        loadW ( "minor", &minor );
        loadW ( "release", &release );
        loadW ( "x", x );
        loadW ( "y", y );
        loadW ( "w", w );
        loadW ( "h", h );
        loadW ( "dataPvStr", dataPvStr, (char *) "" );
        loadW ( "widthPvStr", widthPvStr, (char *) "" );
        loadW ( "heightPvStr", heightPvStr, (char *) "" ); 
        loadW ( "widthOffsetPvStr", widthOffsetPvStr, (char *) "" );
        loadW ( "heightOffsetPvStr", heightOffsetPvStr, (char *) "" );
        loadW ( "gridSizePvStr", gridSizePvStr, (char *) "" );
        loadW ( "useFalseColourPvStr", useFalseColourPvStr, (char *) "" ); 
        loadW ( "showGridPvStr", showGridPvStr, (char *) "" ); 
        loadW ( "gridColourPvStr", gridColourPvStr, (char *) "" ); 
        loadW ( "pvBasedDataSize", pvBasedDataSize);
        loadW ( "maxDataWidth", maxDataWidth);
        loadW ( "maxDataHeight", maxDataHeight);
        loadW ( "pvBasedOffsets", pvBasedOffsets);
        loadW ( "pvBasedGridSize", pvBasedGridSize);
        loadW ( "pvBasedUseFalseColour", pvBasedUseFalseColour);
        loadW ( "pvBasedShowGrid", pvBasedShowGrid);
        loadW ( "pvBasedGridColour", pvBasedGridColour);
        loadW ( "endObjectProperties" );
        loadW ( "" );
  
        return writeTags ( fptr );
    }

private:
    TwoDProfileMonitorTags (const TwoDProfileMonitorTags &s);
    TwoDProfileMonitorTags &operator= (const TwoDProfileMonitorTags &s);

};


// stuff needed for EDM to load from DLL
extern "C" 
{

    void *create_TwoDProfileMonitorClassPtr ( void )
    {

        return (new TwoDProfileMonitor ());

    }

}

extern "C"
{

    void *clone_TwoDProfileMonitorClassPtr ( void *s )
    {

        return (new TwoDProfileMonitor (*(TwoDProfileMonitor *)s));

    }

}

// Support registration
#include "environment.str"

typedef struct libRecTag
{
    char *className;
    char *typeName;
    char *text;
} libRecType, *libRecPtr;

static int libRecIndex = 0;

static libRecType libRec[] = 
{
    { "TwoDProfileMonitor", global_str2, "New Monitor" }
};

extern "C" 
{

    int firstRegRecord (char **className,
                        char **typeName,
                        char **text )
    {
    
        libRecIndex = 0;
    
        *className = libRec[libRecIndex].className;
        *typeName = libRec[libRecIndex].typeName;
        *text = libRec[libRecIndex].text;
    
        return 0; // OK
    
    }
  
    int nextRegRecord (char **className,
                       char **typeName,
                       char **text )
    {
    
        if (libRecIndex >= sizeof (libRec)/sizeof (libRecType) - 1)
            return -1; // done
        ++libRecIndex;
    
        *className = libRec[libRecIndex].className;
        *typeName = libRec[libRecIndex].typeName;
        *text = libRec[libRecIndex].text;
    
        return 0; // OK
    
    }
  
}

void TwoDProfileMonitor::constructCommon (void)
{

    /* start off not knowing image data width */
    pvBasedDataSize = 0;
    useFalseColour = 0;
    pvBasedUseFalseColour = 0;
    showGrid = 0;
    pvBasedShowGrid = 0;
    gridColour = 0;
    pvBasedGridColour = 0;
    dataWidth = -1;
    dataHeight = 0; // 0 if no PV supplied which is OK, -1 if invalid PV
    maxDataWidth = 0;
    maxDataHeight = 0;
    widthOffset = 0;
    heightOffset = 0;
    pvBasedOffsets = 0;
    gridSize = 100; // Safe default - 0 would crash if grid turned on.
    pvBasedGridSize = 0;
    activeMode = 0;

    wd = widgetCreate ();
    twoDWidget = NULL;

    name = "TwoDProfileMonitorClass";
  
    dataPvStr.setRaw ("");
    widthPvStr.setRaw ("");
    heightPvStr.setRaw ("");
    widthOffsetPvStr.setRaw ("");
    heightOffsetPvStr.setRaw ("");
    gridSizePvStr.setRaw ("");
    useFalseColourPvStr.setRaw ("");
    showGridPvStr.setRaw ("");
    gridColourPvStr.setRaw ("");
  
    dataPv = NULL;
    widthPv = NULL;
    heightPv = NULL;
    widthOffsetPv = NULL;
    heightOffsetPv = NULL;
    gridSizePv = NULL;
    useFalseColourPv = NULL;
    showGridPv = NULL;
    gridColourPv = NULL;

    strcpy (dataPvBuf, ""); // just to be safe
    strcpy (widthPvBuf, ""); // just to be safe
    strcpy (heightPvBuf, ""); // just to be safe
    strcpy (widthOffsetPvBuf, "");
    strcpy (heightOffsetPvBuf, "");
    strcpy (gridSizePvBuf, "");
    strcpy (useFalseColourPvBuf, ""); // just to be safe
    strcpy (showGridPvBuf, ""); // just to be safe
    strcpy (gridColourPvBuf, ""); // just to be safe
    twoDWidget = NULL;

    average_time_usec = 0;

#if (0)
    // text stuff (for edit mode drawing)
    char *textFontTag;
    int textAlignment;
    int textColour;
#endif

}

TwoDProfileMonitor::TwoDProfileMonitor (void) : activeGraphicClass () 
{ 
  
    constructCommon ();
}


TwoDProfileMonitor::TwoDProfileMonitor (const TwoDProfileMonitor &s)
{ 
  
    // clone base class data
    // why doesn't activeGraphicClass copy constructor do this?
    activeGraphicClass::clone ( &s );

    constructCommon ();

    // does the copy constructor work?
    // dataPvStr = s.dataPvStr;
    // widthPvStr = s.widthPvStr;
    dataPvStr.setRaw (s.dataPvStr.rawString);
    widthPvStr.setRaw (s.widthPvStr.rawString);
    heightPvStr.setRaw (s.heightPvStr.rawString);
    widthOffsetPvStr.setRaw (s.widthOffsetPvStr.rawString);
    heightOffsetPvStr.setRaw (s.heightOffsetPvStr.rawString);
    gridSizePvStr.setRaw (s.gridSizePvStr.rawString);
    useFalseColourPvStr.setRaw (s.useFalseColourPvStr.rawString);
    showGridPvStr.setRaw (s.showGridPvStr.rawString);
    gridColourPvStr.setRaw (s.gridColourPvStr.rawString);

    pvBasedDataSize = s.pvBasedDataSize;
    maxDataWidth = s.maxDataWidth;
    maxDataHeight = s.maxDataHeight;
    useFalseColour = s.useFalseColour;
    pvBasedUseFalseColour = s.pvBasedUseFalseColour;
    showGrid = s.showGrid;
    pvBasedShowGrid = s.pvBasedShowGrid;
    gridColour = s.gridColour;
    pvBasedGridColour = s.pvBasedGridColour;
    dataWidth = s.dataWidth; 
    widthOffset = s.widthOffset;
    heightOffset = s.heightOffset;
    pvBasedOffsets = s.pvBasedOffsets;
    gridSize = s.gridSize;
    pvBasedGridSize = s.pvBasedGridSize;
}

TwoDProfileMonitor::~TwoDProfileMonitor (void) {widgetDestroy (wd);}

// called when widget is made active as edm changes to "execute" mode,
// pass values are 0-6
int TwoDProfileMonitor::activate ( int pass )
{

    switch (pass)
    {
    case 1:
#ifdef DEBUG
        printf ("TwoDProfileMonitor::activate pass 1\n");
#endif
        initialDataConnection = 1;
        initialWidthConnection = 0;
        initialHeightConnection = 0;
        initialWidthOffsetConnection = 0;
        initialHeightOffsetConnection = 0;
        initialGridSizeConnection = 0;
        initialUseFalseColourConnection = 0;
        initialShowGridConnection = 0;
        initialGridColourConnection = 0;
        needConnectInit = needInfoInit = needRefresh = 0;
        pvNotConnectedMask = active = init = 0;
        activeMode = 1;

        if (!dataPvStr.getExpanded () ||
            blankOrComment (dataPvStr.getExpanded ()))
        {
            dataPvExists = 0;
        }
        else
        {
            dataPvExists = 1;
            pvNotConnectedMask |= 1;
        }

        if (!pvBasedDataSize)
        {
            widthPvExists = 0;
            dataWidth = atoi (widthPvStr.getRaw ());
            heightPvExists = 0;
        }
        else
        {
            if (!widthPvStr.getExpanded () ||
                blankOrComment (widthPvStr.getExpanded ()))
            {
                widthPvExists = 0;
            }
            else
            {
                widthPvExists = 1;
                initialWidthConnection = 1;
                pvNotConnectedMask |= 2;
            }
            if (!heightPvStr.getExpanded () ||
                blankOrComment (heightPvStr.getExpanded ()))
            {
                heightPvExists = 0;
            }
            else
            {
                heightPvExists = 1;
                initialHeightConnection = 1;
                pvNotConnectedMask |= 4;
            }
        }

        if (!pvBasedUseFalseColour)
        {
            useFalseColourPvExists = 0;
            useFalseColour = atoi (useFalseColourPvStr.getRaw ());
        }
        else
        {
            if (!useFalseColourPvStr.getExpanded () ||
                blankOrComment (useFalseColourPvStr.getExpanded ()))
            {
                useFalseColourPvExists = 0;
            }
            else
            {
                useFalseColourPvExists = 1;
                initialUseFalseColourConnection = 1;
                pvNotConnectedMask |= 8;
            }
        }

        if (!pvBasedShowGrid)
        {
            showGridPvExists = 0;
            showGrid = atoi (showGridPvStr.getRaw ());
        }
        else
        {
            if (!showGridPvStr.getExpanded () ||
                blankOrComment (showGridPvStr.getExpanded ()))
            {
                showGridPvExists = 0;
            }
            else
            {
                showGridPvExists = 1;
                initialShowGridConnection = 1;
                pvNotConnectedMask |= 16;
            }
        }

        if (!pvBasedOffsets)
        {
            widthOffsetPvExists = 0;
            heightOffsetPvExists = 0;
            widthOffset = atoi (widthOffsetPvStr.getRaw ());
            heightOffset = atoi (heightOffsetPvStr.getRaw ());
        }
        else
        {
            if (!widthOffsetPvStr.getExpanded () ||
                blankOrComment (widthOffsetPvStr.getExpanded ()))
            {
                widthOffsetPvExists = 0;
            }
            else
            {
                widthOffsetPvExists = 1;
                initialWidthOffsetConnection = 1;
                pvNotConnectedMask |= 32;
            }
            if (!heightOffsetPvStr.getExpanded () ||
                blankOrComment (heightOffsetPvStr.getExpanded ()))
            {
                heightOffsetPvExists = 0;
            }
            else
            {
                heightOffsetPvExists = 1;
                initialHeightOffsetConnection = 1;
                pvNotConnectedMask |= 64;
            }
        }

        if (!pvBasedGridSize)
        {
            gridSizePvExists = 0;
            gridSize = atoi (gridSizePvStr.getRaw ());
        }
        else
        {
            if (!gridSizePvStr.getExpanded () ||
                blankOrComment (gridSizePvStr.getExpanded ()))
            {
                gridSizePvExists = 0;
            }
            else
            {
                gridSizePvExists = 1;
                initialGridSizeConnection = 1;
                pvNotConnectedMask |= 128;
            }
        }

        if (!pvBasedGridColour)
        {
            gridColourPvExists = 0;
            gridColour = atoi (gridColourPvStr.getRaw ());
        }
        else
        {
            if (!gridColourPvStr.getExpanded () ||
                blankOrComment (gridColourPvStr.getExpanded ()))
            {
                gridColourPvExists = 0;
            }
            else
            {
                gridColourPvExists = 1;
                initialGridColourConnection = 1;
                pvNotConnectedMask |= 256;
            }
        }

#ifdef DEBUG
        printf (
            "TwoDProfileMonitor::activate pass 1 - pvNotConnectedMask = %d\n",
            pvNotConnectedMask);
#endif
        break;
    
        // connect PVs during pass 2
    case 2:
        {
#ifdef DEBUG
            printf ("TwoDProfileMonitor::activate pass 2\n");
#endif
            // assume the best!
            pvColour.setColorIndex ( actWin->defaultTextFgColor, actWin->ci );
      

            if (!dataPvExists) 
            {
#ifdef DEBUG
                printf (
                    "TwoDProfileMonitor::activate pass 2 - dataPvExists = 0\n");
#endif
                break; // don't bother the factory
            }

            dataPv = the_PV_Factory->create ( dataPvStr.getExpanded () );
            if ( dataPv )
            {
#ifdef DEBUG             
                printf (
                    "TwoDProfMon::activate pass 2 - add data connect cb\n"); 
#endif
                dataPv->add_conn_state_callback (monitorDataConnectState,
                                                 this);
                //dataPv->add_value_callback ( pvUpdate, this );
            }
     
            if (widthPvExists)
            {
                // printf ("activate (TwoDMon.cc) - width PV = %s = %s\n", 
                //         widthPvStr.getRaw (),
                //         widthPvStr.getExpanded ());
                widthPv = the_PV_Factory->create ( widthPvStr.getExpanded () );
                if ( widthPv )
                {
#ifdef DEBUG
                    printf (
                        "TwoDProfMon::activate pass 2 - adding width connect"
                        " cb\n");
#endif
                    widthPv->add_conn_state_callback (monitorWidthConnectState,
                                                      this);
                    //widthPv->add_value_callback ( pvUpdate, this );
                }
            }
            if (heightPvExists)
            {
                // printf ("activate (TwoDMon.cc) - height PV = %s = %s\n", 
                //         heightPvStr.getRaw (),
                //         heightPvStr.getExpanded ());
                heightPv = the_PV_Factory->create ( heightPvStr.getExpanded ());
                if ( heightPv )
                {
#ifdef DEBUG
                    printf (
                        "TwoDProfMon::activate pass 2 - adding height connect"
                        " cb\n");
#endif
                    heightPv->add_conn_state_callback (
                                                    monitorHeightConnectState,
                                                    this);
                    //heightPv->add_value_callback ( pvUpdate, this );
                }
            }
            if (widthOffsetPvExists)
            {
                // printf ("activate (TwoDMon.cc) - width offset PV"
                //         " = %s = %s\n", 
                //         widthOffsetPvStr.getRaw (),
                //         widthOffsetPvStr.getExpanded ());
                widthOffsetPv = the_PV_Factory->create (
                                            widthOffsetPvStr.getExpanded () );
                if ( widthOffsetPv )
                {
#ifdef DEBUG
                    printf ( "TwoDProfMon::activate pass 2 - adding width"
                             " offset connect cb\n");
#endif
                    widthOffsetPv->add_conn_state_callback (
                                          monitorWidthOffsetConnectState,
                                          this);
                    //widthOffsetPv->add_value_callback ( pvUpdate, this );
                }
            }
            if (heightOffsetPvExists)
            {
                // printf ("activate (TwoDMon.cc) - height offset PV"
                //         " = %s = %s\n", 
                //         heightOffsetPvStr.getRaw (),
                //         heightOffsetPvStr.getExpanded ());
                heightOffsetPv = the_PV_Factory->create (
                                          heightOffsetPvStr.getExpanded () );
                if ( heightOffsetPv )
                {
#ifdef DEBUG
                    printf ( "TwoDProfMon::activate pass 2 - adding height"
                             " offset connect cb\n");
#endif
                    heightOffsetPv->add_conn_state_callback (
                                               monitorHeightOffsetConnectState,
                                               this);
                    //heightOffsetPv->add_value_callback ( pvUpdate, this );
                }
            }
            if (gridSizePvExists)
            {
                // printf ("activate (TwoDMon.cc) - grid size PV = %s = %s\n", 
                //         gridSizePvStr.getRaw (),
                //         gridSizePvStr.getExpanded ());
                gridSizePv = the_PV_Factory->create (
                                                 gridSizePvStr.getExpanded ());
                if ( gridSizePv )
                {
#ifdef DEBUG
                    printf ( "TwoDProfMon::activate pass 2 - adding grid size"
                             " connect cb\n");
#endif
                    gridSizePv->add_conn_state_callback (
                                              monitorGridSizeConnectState,
                                              this);
                    //gridSizePv->add_value_callback ( pvUpdate, this );
                }
            }
#ifdef DEBUG
            printf ("TwoDProfileMonitor::activate pass 2 -"
                    " useFalseColourPvExists = %d\n", useFalseColourPvExists);
#endif
            if (useFalseColourPvExists)
            {
                // printf ("activate (TwoDMon.cc) - use false colour"
                //         " PV = %s = %s\n", 
                //         useFalseColourPvStr.getRaw (),
                //         useFalseColourPvStr.getExpanded ());
                useFalseColourPv = the_PV_Factory->create (
                                        useFalseColourPvStr.getExpanded () );
                if ( useFalseColourPv )
                {
#ifdef DEBUG
                    printf (
                        "TwoDProfMon::activate pass 2 - adding use false colour"
                        " connect cb\n");
#endif
                    useFalseColourPv->add_conn_state_callback (
                                             monitorUseFalseColourConnectState,
                                             this);
                    //useFalseColouPv->add_value_callback ( pvUpdate, this );
                }
            }
#ifdef DEBUG
            printf ("TwoDProfileMonitor::activate pass 2 -"
                    " showGridPvExists = %d\n", showGridPvExists);
#endif
            if (showGridPvExists)
            {
                // printf ("activate (TwoDMon.cc) - show grid PV = %s = %s\n", 
                //         showGridPvStr.getRaw (),
                //         showGridPvStr.getExpanded ());
                showGridPv = the_PV_Factory->create (
                                        showGridPvStr.getExpanded () );
                if ( showGridPv )
                {
#ifdef DEBUG
                    printf (
                        "TwoDProfMon::activate pass 2 - adding show grid"
                        " connect cb\n");
#endif
                    showGridPv->add_conn_state_callback (
                                             monitorShowGridConnectState,
                                             this);
                    //showGridPv->add_value_callback ( pvUpdate, this );
                }
            }
#ifdef DEBUG
            printf ("TwoDProfileMonitor::activate pass 2 -"
                    " gridColourPvExists = %d\n", gridColourPvExists);
#endif
            if (gridColourPvExists)
            {
                // printf ("activate (TwoDMon.cc) - grid col PV = %s = %s\n", 
                //         gridColourPvStr.getRaw (),
                //         gridColourPvStr.getExpanded ());
                gridColourPv = the_PV_Factory->create (
                                        gridColourPvStr.getExpanded () );
                if ( gridColourPv )
                {
#ifdef DEBUG
                    printf (
                        "TwoDProfMon::activate pass 2 - adding grid colour"
                        " connect cb\n");
#endif
                    gridColourPv->add_conn_state_callback (
                                             monitorGridColourConnectState,
                                             this);
                    //gridColourPv->add_value_callback ( pvUpdate, this );
                }
            }
#ifdef DEBUG
            printf ("activate (TwoDMon.cc) - dataPv->is_valid = %d\n",
                    dataPv->is_valid ());
            printf ("activate (TwoDMon.cc) - widthPv->is_valid = %d\n",
                    widthPv->is_valid ());
            printf ("activate (TwoDMon.cc) - heightPv->is_valid = %d\n",
                    heightPv->is_valid ());
            printf ("activate (TwoDMon.cc) - widthOffsetPv->is_valid = %d\n",
                    widthOffsetPv->is_valid ());
            printf ("activate (TwoDMon.cc) - heightOffsetPv->is_valid = %d\n",
                    heightOffsetPv->is_valid ());
            printf ("activate (TwoDMon.cc) - useFalseColourPv->is_valid = %d\n",
                    useFalseColourPv->is_valid ());
            printf ("activate (TwoDMon.cc) - showGridPv->is_valid = %d\n",
                    showGridPv->is_valid ());
            printf ("activate (TwoDMon.cc) - gridSizePv->is_valid = %d\n",
                    gridSizePv->is_valid ());
            printf ("activate (TwoDMon.cc) - gridColourPv->is_valid = %d\n",
                    gridColourPv->is_valid ());
#endif
        }
        break;

        // OK, now create the widget
    case 6:

        // create the execute-mode widget using XRT or other
        // standard Motif 2-D data widget
        twoDWidget = widgetCreateWidget (
                         wd, actWin->appCtx->appContext (), actWin->d,
                         actWin->ci->getColorMap (), actWin->executeWidget,
                         x, y, h, w);

        // capture events to pass on to EDM
        XtAddEventHandler (
            twoDWidget,
            LeaveWindowMask | EnterWindowMask | PointerMotionMask |
            ButtonPressMask |ButtonReleaseMask,
            False, grabButtonEvent, (XtPointer) this);
   
        // hand over control 
        XtManageChild (twoDWidget);
   
        break;


    default:
        break;
    
    }
  
    return 1;
}

// user hit the "OK" button on the edit popup
void TwoDProfileMonitor::editOK (Widget w,
                                 XtPointer client,
                                 XtPointer call )
{
  
    TwoDProfileMonitor *me = (TwoDProfileMonitor *) client;

    // first apply any changes
    editApply (w, client, call); 

    me->ef.popdown ();
    me->operationComplete ();
  
}
  
// user hit the "Apply" button on the edit popup
void TwoDProfileMonitor::editApply (Widget w,
                                    XtPointer client,
                                    XtPointer call )
{
  
    TwoDProfileMonitor *me = (TwoDProfileMonitor *) client;

    me->eraseSelectBoxCorners ();
    me->erase ();

    me->x = me->xBuf;
    me->y = me->yBuf;
    me->w = me->wBuf;
    me->h = me->hBuf;
    me->sboxX = me->xBuf;
    me->sboxY = me->yBuf;
    me->sboxW = me->wBuf;
    me->sboxH = me->hBuf;
    me->maxDataWidth = me->maxDataWidthBuf;
    me->maxDataHeight = me->maxDataHeightBuf;

    // do the PV name(s)
    me->dataPvStr.setRaw ( me->dataPvBuf );

    // now the width: if fixed is indicated, interpret PV string as int
    me->widthPvStr.setRaw ( me->widthPvBuf );
    if (!me->pvBasedDataSize)
         me->dataWidth = atoi ( me->widthPvBuf );
    else
         me->dataWidth = -1; // just to be safe

    me->heightPvStr.setRaw ( me->heightPvBuf );
    me->maxDataWidth = me->maxDataWidthBuf;
    me->maxDataHeight = me->maxDataHeightBuf;
    me->dataHeight = 0;  // correct if PV null, will be overwritten otherwise

    // now the offsets: if fixed is indicated, interpret PV string as int
    me->widthOffsetPvStr.setRaw ( me->widthOffsetPvBuf );
    me->heightOffsetPvStr.setRaw ( me->heightOffsetPvBuf );
    if (!me->pvBasedOffsets)
    {
         me->widthOffset = atoi ( me->widthOffsetPvBuf );
         me->heightOffset = atoi ( me->heightOffsetPvBuf );
    }
    else
    {
         me->widthOffset = 0; // just to be safe
         me->heightOffset = 0; // just to be safe
    }

    // now the grid size: if fixed is indicated, interpret PV string as int
    me->gridSizePvStr.setRaw ( me->gridSizePvBuf );
    if (!me->pvBasedGridSize)
         me->gridSize = atoi ( me->gridSizePvBuf );
    else
         me->gridSize = 100; // just to be safe

    // now the 'use false colour': if fixed is indicated, interpret PV string
    // as int
    me->useFalseColourPvStr.setRaw ( me->useFalseColourPvBuf );
    if (!me->pvBasedUseFalseColour)
    {
         me->useFalseColour = atoi ( me->useFalseColourPvBuf );
    }
    else
         me->useFalseColour = 0; // just to be safe

    // now the 'show grid': if fixed is indicated, interpret PV string as int
    me->showGridPvStr.setRaw ( me->showGridPvBuf );
    if (!me->pvBasedShowGrid)
         me->showGrid = atoi ( me->showGridPvBuf );
    else
         me->showGrid = 0; // just to be safe

    // now the grid colour: if fixed is indicated, interpret PV string as int
    me->gridColourPvStr.setRaw ( me->gridColourPvBuf );
    if (!me->pvBasedGridColour)
         me->gridColour = atoi ( me->gridColourPvBuf );
    else
         me->gridColour = 0; // just to be safe

    // support auto-save
    me->actWin->setChanged ();

  // let EDM know that "Apply" was invoked
  me->refresh ();
  
}

// user hit the "Cancel" button on the edit popup
void TwoDProfileMonitor::editCancel (Widget w,
                                     XtPointer client,
                                     XtPointer call )
{
  
    TwoDProfileMonitor *me = (TwoDProfileMonitor *) client;
    
    me->ef.popdown ();

    // no need for EDM to do anything
    me->operationCancel ();
    
}

// user hit the "Cancel" button on the edit popup
void TwoDProfileMonitor::editCancelCreate (Widget w,
                                           XtPointer client,
                                           XtPointer call )
{
  
    TwoDProfileMonitor *me = (TwoDProfileMonitor *) client;
  
    me->ef.popdown ();
  
    // remove all traces of our existence!
    me->erase ();
    me->deleteRequest = 1;
  
    // no need for EDM to do anything
    me->operationCancel ();
  
}

void TwoDProfileMonitor::editCommon ( activeWindowClass *actWin,
                                      entryFormClass *_ef, int create )
{

    // create edit box
    ef.create ( actWin->top, actWin->appCtx->ci.getColorMap (),
                &actWin->appCtx->entryFormX,
                &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
                &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
                "2D Profile Monitor Properties", NULL, NULL, NULL );

    xBuf = x;
    yBuf = y;
    wBuf = w;
    hBuf = h;

    ef.addTextField ("X", 30, &xBuf);
    ef.addTextField ("Y", 30, &yBuf);
    ef.addTextField ("Widget Width", 30, &wBuf);
    ef.addTextField ("Widget Height", 30, &hBuf);
    // copy out, we'll copy in during "Apply"
    strncpy (dataPvBuf, dataPvStr.getRaw (), sizeof (dataPvBuf) - 1);
    ef.addTextField ("Data PV", 30, dataPvBuf, sizeof (dataPvBuf) - 1);

    // copy out, we'll copy in during "Apply"
    strncpy (widthPvBuf, widthPvStr.getRaw (), sizeof (widthPvBuf) - 1);
    ef.addTextField ("Data Width (Fixed/PV)", 30, widthPvBuf,
                     sizeof (widthPvBuf) - 1);
    strncpy (heightPvBuf, heightPvStr.getRaw (), sizeof (heightPvBuf) - 1);
    ef.addTextField ("Data Height PV (ignored for fixed size)",
                     30, heightPvBuf, sizeof (heightPvBuf) - 1);
    maxDataWidthBuf = maxDataWidth;
    ef.addTextField ("Max. Data Width (needed for grid)",
                     30, &maxDataWidthBuf);
    maxDataHeightBuf = maxDataHeight;
    ef.addTextField ("Max. Data Height (needed for grid)",
                     30, &maxDataHeightBuf);
    strncpy (widthOffsetPvBuf, widthOffsetPvStr.getRaw (),
             sizeof (widthOffsetPvBuf) - 1);
    ef.addTextField ("Width Offset (Fixed/PV) (needed for grid)", 30,
                     widthOffsetPvBuf, sizeof (widthOffsetPvBuf) - 1);
    strncpy (heightOffsetPvBuf, heightOffsetPvStr.getRaw (),
             sizeof (heightOffsetPvBuf) - 1);
    ef.addTextField ("Height Offset (Fixed/PV) (needed for grid)", 30,
                     heightOffsetPvBuf, sizeof (heightOffsetPvBuf) - 1);
    strncpy (gridSizePvBuf, gridSizePvStr.getRaw (),
             sizeof (gridSizePvBuf) - 1);
    ef.addTextField ("Grid Size (Fixed/PV)", 30,
                     gridSizePvBuf, sizeof (gridSizePvBuf) - 1);
    strncpy (useFalseColourPvBuf, useFalseColourPvStr.getRaw (),
             sizeof (useFalseColourPvBuf) - 1);
    ef.addTextField ("Use False Colour (0/1/PV)", 30, useFalseColourPvBuf,
                     sizeof (useFalseColourPvBuf) - 1);
    strncpy (showGridPvBuf, showGridPvStr.getRaw (),
             sizeof (showGridPvBuf) - 1);
    ef.addTextField ("Show Grid (0/1/PV)", 30, showGridPvBuf,
                     sizeof (showGridPvBuf) - 1);
    strncpy (gridColourPvBuf, gridColourPvStr.getRaw (),
             sizeof (gridColourPvBuf) - 1);
    ef.addTextField ("Grid Colour (0/1/PV)", 30, gridColourPvBuf,
                     sizeof (gridColourPvBuf) - 1);
    ef.addOption ("Data Size Type", "Fixed|PV-based", &pvBasedDataSize);
    ef.addOption ("Width/Height Offset Type", "Fixed|PV-based",
                                                      &pvBasedOffsets);
    ef.addOption ("Use False Colour Type", "Fixed|PV-based",
                  &pvBasedUseFalseColour);
    ef.addOption ("Show Grid Type", "Fixed|PV-based",
                  &pvBasedShowGrid);
    ef.addOption ("Grid Size Type", "Fixed|PV-based",
                  &pvBasedGridSize);
    ef.addOption ("Grid Colour Type", "Fixed|PV-based",
                  &pvBasedGridColour);
    // ef.addToggle ("PV-based Width", &height);

    // Map dialog box form buttons to callbacks
    ef.finished ( editOK, editApply, create ? editCancelCreate : editCancel,
                  this );

    // Required by display engine
    actWin->currentEf = _ef;

    // popup the dialog box
    ef.popup ();

}

int TwoDProfileMonitor::edit ( void )
{

    editCommon ( actWin, &ef );

    return 1;
}

int TwoDProfileMonitor::createInteractive (activeWindowClass *actWin,
                                           int x,
                                           int y,
                                           int w,
                                           int h )
{
  
    this->actWin = actWin;
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;

    draw ();
  
    editCommon ( actWin, NULL, ~0 );

    return 1;
}

int TwoDProfileMonitor::createFromFile (FILE *fptr,
                                        char *name,
                                        activeWindowClass *actWin )
{
  
    this->actWin = actWin;
  
    // use tag class and name to read from file
    TwoDProfileMonitorTags tag;
  
    if ( !(1 & tag.read ( this,
                          fptr,
                          &x, &y, &w, &h,
                          &dataPvStr,
                          &widthPvStr, &heightPvStr, 
                          &widthOffsetPvStr, &heightOffsetPvStr,
                          &gridSizePvStr,
                          &useFalseColourPvStr,
                          &showGridPvStr,
                          &gridColourPvStr,
                          &pvBasedDataSize,
                          &maxDataWidth, &maxDataHeight,
                          &pvBasedOffsets,
                          &pvBasedGridSize,
                          &pvBasedUseFalseColour,
                          &pvBasedShowGrid,
                          &pvBasedGridColour ) ) )
    {
        actWin->appCtx->postMessage ( tag.errMsg () );
    }
  
    updateDimensions ();
    initSelectBox ();
  
    return 1;
}

// What is an exchange file?
int TwoDProfileMonitor::importFromXchFile (FILE *fptr,
                                           char *name,
                                           activeWindowClass *actWin )
{

    cerr << "Import from eXchange file not supported" << endl;

    return 0;
}

int TwoDProfileMonitor::save ( FILE *fptr )
{
    // use tag class to serialize data
    TwoDProfileMonitorTags tag;
    return tag.write ( fptr, &x, &y, &w, &h, &dataPvStr,
                       &widthPvStr, &heightPvStr,
                       &widthOffsetPvStr, &heightOffsetPvStr,
                       &gridSizePvStr,
                       &useFalseColourPvStr,
                       &showGridPvStr,
                       &gridColourPvStr,
                       &pvBasedDataSize,
                       &maxDataWidth, &maxDataHeight,
                       &pvBasedOffsets,
                       &pvBasedGridSize,
                       &pvBasedUseFalseColour,
                       &pvBasedShowGrid,
                       &pvBasedGridColour );
  
}

// called any time the widget needs to draw or re-draw itself
// in edit-mode 
int TwoDProfileMonitor::draw (void)
{ 
  
    // draw rectangle using X primitives
    XFillRectangle ( actWin->d, XtWindow (actWin->drawWidget),
                     actWin->drawGc.eraseGC (), x, y, w, h );
    XDrawRectangle ( actWin->d, XtWindow (actWin->drawWidget),
                     actWin->drawGc.normGC (), x, y, w, h );
    // Draw label text (crude because we can escape widget boundaries)
    XDrawImageString ( actWin->d, XtWindow (actWin->drawWidget),
                       actWin->drawGc.normGC (), x + 5, y + h / 2,
                       dataPvStr.getRaw (), strlen (dataPvStr.getRaw ()) );
  
    return activeGraphicClass::draw (); 
} 

// erase widget in responce to "cut" command
// in edit-mode 
int TwoDProfileMonitor::erase (void)
{ 

    // draw rectangle using X primitives
    XDrawRectangle ( actWin->d, XtWindow (actWin->drawWidget),
                    actWin->drawGc.eraseGC (), x, y, w, h );

    return activeGraphicClass::erase (); 
}

// returning to edit mode, pass values are 1 and 2
int TwoDProfileMonitor::deactivate ( int pass )
{ 
    active = 0;
    activeMode = 0;

    if ( dataPv != NULL )
    {
        actWin->appCtx->proc->lock ();
        dataPv->remove_conn_state_callback ( monitorDataConnectState, this );
#ifdef DEBUG
        printf ("TwoDProfileMonitor::deactivate - removing data callback\n");
#endif
        dataPv->remove_value_callback ( dataUpdate, this );
        dataPv->release ();
        dataPv = NULL;
        actWin->appCtx->proc->unlock ();
    }

    if ( widthPv != NULL )
    {

        actWin->appCtx->proc->lock ();
        widthPv->remove_conn_state_callback ( monitorWidthConnectState, this );
        widthPv->remove_value_callback ( sizeUpdate, this );
        widthPv->release ();
        widthPv = NULL;
        actWin->appCtx->proc->unlock ();
    }

    if ( heightPv != NULL )
    {

        actWin->appCtx->proc->lock ();
        heightPv->remove_conn_state_callback (monitorHeightConnectState, this);
        heightPv->remove_value_callback ( sizeUpdate, this );
        heightPv->release ();
        heightPv = NULL;
        actWin->appCtx->proc->unlock ();
    }

    if ( widthOffsetPv != NULL )
    {

        actWin->appCtx->proc->lock ();
        widthOffsetPv->remove_conn_state_callback (
                                    monitorWidthOffsetConnectState, this );
        widthOffsetPv->remove_value_callback ( dataUpdate, this );
        widthOffsetPv->release ();
        widthOffsetPv = NULL;
        actWin->appCtx->proc->unlock ();
    }

    if ( heightOffsetPv != NULL )
    {

        actWin->appCtx->proc->lock ();
        heightOffsetPv->remove_conn_state_callback (
                                    monitorHeightOffsetConnectState, this );
        heightOffsetPv->remove_value_callback ( dataUpdate, this );
        heightOffsetPv->release ();
        heightOffsetPv = NULL;
        actWin->appCtx->proc->unlock ();
    }

    if ( gridSizePv != NULL )
    {

        actWin->appCtx->proc->lock ();
        gridSizePv->remove_conn_state_callback (
                                    monitorGridSizeConnectState, this );
        gridSizePv->remove_value_callback ( dataUpdate, this );
        gridSizePv->release ();
        gridSizePv = NULL;
        actWin->appCtx->proc->unlock ();
    }

    if ( useFalseColourPv != NULL )
    {

        actWin->appCtx->proc->lock ();
        useFalseColourPv->remove_conn_state_callback (
                               monitorUseFalseColourConnectState, this );
        useFalseColourPv->remove_value_callback ( dataUpdate, this );
        useFalseColourPv->release ();
        useFalseColourPv = NULL;
        actWin->appCtx->proc->unlock ();
    }
    if ( showGridPv != NULL )
    {

        actWin->appCtx->proc->lock ();
        showGridPv->remove_conn_state_callback (
                               monitorShowGridConnectState, this );
        showGridPv->remove_value_callback ( dataUpdate, this );
        showGridPv->release ();
        showGridPv = NULL;
        actWin->appCtx->proc->unlock ();
    }
    if ( gridColourPv != NULL )
    {

        actWin->appCtx->proc->lock ();
        gridColourPv->remove_conn_state_callback (
                               monitorGridColourConnectState, this );
        gridColourPv->remove_value_callback ( dataUpdate, this );
        gridColourPv->release ();
        gridColourPv = NULL;
        actWin->appCtx->proc->unlock ();
    }
    // disconnect PV timeout on pass 1
    if ( pass == 1 )
    {
        // disable deferred processing before anything else
        actWin->appCtx->proc->lock ();
        actWin->remDefExeNode (aglPtr);
        actWin->appCtx->proc->unlock ();
    }

    // now turn off the Widget
    if ( pass == 2 )
    {
#ifdef DEBUG
        printf ("TwoDProfileMonitor::deactivate - pass 2\n");
#endif
        widgetNewDisplayInfo (wd, false, 0, 0);
        XtUnmanageChild (twoDWidget);
        XtDestroyWidget (twoDWidget);
        widgetDestroyWidget (wd);

    }
  
    return activeGraphicClass::deactivate (pass); 
}


void TwoDProfileMonitor::monitorDataConnectState (ProcessVariable *pv,
                                                   void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid ())
        {
            me->pvNotConnectedMask &= ~((unsigned int) 1); 
#ifdef DEBUG
            printf ("TwoDProfMon::monDataConState - set pvNotConnMask to %d\n",
                    me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 1;
            me->active = 0;
            me->bufInvalidate ();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::monitorWidthConnectState (ProcessVariable *pv,
                                                   void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid ())
        {
            me->pvNotConnectedMask &= ~((unsigned int) 2); 
#ifdef DEBUG
            printf ("TwoDProfMon::monWidthConState - set pvNotConMask to %d\n",
                    me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 2;
            me->active = 0;
            me->bufInvalidate ();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::monitorHeightConnectState (ProcessVariable *pv,
                                                   void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid ())
        {
            me->pvNotConnectedMask &= ~((unsigned int) 4); 
#ifdef DEBUG
            printf ("TwoDProfMon::monHeightConState - set pvNotConMsk to %d\n",
                    me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 4;
            me->active = 0;
            me->bufInvalidate ();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::monitorUseFalseColourConnectState (
                                                   ProcessVariable *pv,
                                                   void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid ())
        {
            me->pvNotConnectedMask &= ~((unsigned int) 8); 
#ifdef DEBUG
            printf (
               "TwoDProfMon::monUseFalseColConState - set pvNotConMask to %d\n",
               me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 8;
            me->active = 0;
            me->bufInvalidate ();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::monitorShowGridConnectState (
                                                   ProcessVariable *pv,
                                                   void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid ())
        {
            me->pvNotConnectedMask &= ~((unsigned int) 16); 
#ifdef DEBUG
            printf (
               "TwoDProfMon::monShowGridConState - set pvNotConMask to %d\n",
               me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 16;
            me->active = 0;
            me->bufInvalidate ();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::monitorGridColourConnectState (
                                                   ProcessVariable *pv,
                                                   void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid ())
        {
            me->pvNotConnectedMask &= ~((unsigned int) 256); 
#ifdef DEBUG
            printf (
               "TwoDProfMon::monGridColConState - set pvNotConMask to %d\n",
               me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 256;
            me->active = 0;
            me->bufInvalidate ();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::monitorWidthOffsetConnectState (ProcessVariable *pv,
                                                         void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid ())
        {
            me->pvNotConnectedMask &= ~((unsigned int) 32); 
#ifdef DEBUG
            printf ("TwoDProfMon::monWidthOffsetConState - set pvNotConMask"
                    " to %d\n", me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 32;
            me->active = 0;
            me->bufInvalidate ();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::monitorHeightOffsetConnectState (ProcessVariable *pv,
                                                          void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid ())
        {
            me->pvNotConnectedMask &= ~((unsigned int) 64); 
#ifdef DEBUG
            printf ("TwoDProfMon::monHeightOffsetConState - set pvNotConMask"
                    " to %d\n", me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 64;
            me->active = 0;
            me->bufInvalidate ();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::monitorGridSizeConnectState (ProcessVariable *pv,
                                                      void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid ())
        {
            me->pvNotConnectedMask &= ~((unsigned int) 128); 
#ifdef DEBUG
            printf ("TwoDProfMon::monWidthOffsetConState - set pvNotConMask"
                    " to %d\n", me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 128;
            me->active = 0;
            me->bufInvalidate ();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::dataUpdate (ProcessVariable *pv,
                                   void *userarg )
{
    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    me->actWin->addDefExeNode (me->aglPtr);
    me->actWin->appCtx->proc->unlock ();
}


void TwoDProfileMonitor::sizeUpdate (ProcessVariable *pv,
                                      void *userarg )
{
    // This function no longer does anything.  Leave it in in case we need it
    // again later.
#ifdef COMMENT_OUT
    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;
    // Delete the data PV and recreate it.
    // Hopefully this will reinitialise the element count to match
    // the new size.
    me->actWin->appCtx->proc->lock ();
    me->dataPv->remove_value_callback ( dataUpdate, me );
    me->dataPv->remove_conn_state_callback ( monitorDataConnectState, me );
    me->dataPv->release ();
    me->dataPv = the_PV_Factory->create ( me->dataPvStr.getExpanded () );
    me->dataPv->add_conn_state_callback ( monitorDataConnectState, me );
    me->dataPv->add_value_callback ( dataUpdate, me );
    me->actWin->appCtx->proc->unlock ();
#endif
}


