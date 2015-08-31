/* $XConsortium: Xlib.h,v 11.237 94/09/01 18:44:49 kaleb Exp $ */
/* 

Copyright (c) 1985, 1986, 1987, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/


/*
 *      Xlib.h - Header definition and support file for the C subroutine
 *      interface library (Xlib) to the X Window System Protocol (V11).
 *      Structures and symbols starting with "_" are private to the library.
 */
#define XlibSpecificationRelease 6

#include <stdlib.h>
#if !defined(WIN32)
#include <unistd.h>
#else
#if defined(_VISUALC_)
#       if defined(_MT) && defined(_DLL) && !defined(_LIB)
#               define MagickExport __declspec(dllexport)
#       else
#               define MagickExport
#       endif
#pragma warning(disable : 4035)
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)
#pragma warning(disable : 4142)
#pragma warning(disable : 4716)
#else
#       define MagickExport
#endif
#endif

#ifdef USG
#ifndef __TYPES__
#include <sys/types.h>                  /* forgot to protect it... */
#define __TYPES__
#endif /* __TYPES__ */
#else
#if defined(_POSIX_SOURCE) && defined(MOTOROLA)
#undef _POSIX_SOURCE
#include <sys/types.h>
#define _POSIX_SOURCE
#else
#ifdef __MWERKS__
#include <types.h>
#else
#include <sys/types.h>
#endif
#endif
#endif /* USG */

#ifdef __MWERKS__
#   include <X.h>
#   define Cursor XCursor
#   define cursor xcursor
#   define Region XRegion
#else
#   include <X11/X.h>
#endif

/* applications should not depend on these two headers being included! */
#ifdef __MWERKS__
#include <Xfuncproto.h>
#include <Xosdefs.h>
#else
#include <X11/Xfuncproto.h>
#include <X11/Xosdefs.h>
#endif

#ifndef X_WCHAR
#ifdef X_NOT_STDC_ENV
#define X_WCHAR
#endif
#endif

#ifndef X_WCHAR
#include <stddef.h>
#else
/* replace this with #include or typedef appropriate for your system */
typedef unsigned long wchar_t;
#endif

typedef char *XPointer;

#define Bool int
#define Status int
#define True 1
#define False 0

#define QueuedAlready 0
#define QueuedAfterReading 1
#define QueuedAfterFlush 2

#define ConnectionNumber(dpy)   (((_XPrivDisplay)dpy)->fd)
#define RootWindow(dpy, scr)    (ScreenOfDisplay(dpy,scr)->root)
#define DefaultScreen(dpy)      (((_XPrivDisplay)dpy)->default_screen)
#define DefaultRootWindow(dpy)  (ScreenOfDisplay(dpy,DefaultScreen(dpy))->root)
#define DefaultVisual(dpy, scr) (ScreenOfDisplay(dpy,scr)->root_visual)
#define DefaultGC(dpy, scr)     (ScreenOfDisplay(dpy,scr)->default_gc)
#define BlackPixel(dpy, scr)    (ScreenOfDisplay(dpy,scr)->black_pixel)
#define WhitePixel(dpy, scr)    (ScreenOfDisplay(dpy,scr)->white_pixel)
#define AllPlanes               ((unsigned long)~0L)
#define QLength(dpy)            (((_XPrivDisplay)dpy)->qlen)
#define DisplayWidth(dpy, scr)  (ScreenOfDisplay(dpy,scr)->width)
#define DisplayHeight(dpy, scr) (ScreenOfDisplay(dpy,scr)->height)
#define DisplayWidthMM(dpy, scr)(ScreenOfDisplay(dpy,scr)->mwidth)
#define DisplayHeightMM(dpy, scr)(ScreenOfDisplay(dpy,scr)->mheight)
#define DisplayPlanes(dpy, scr) (ScreenOfDisplay(dpy,scr)->root_depth)
#define DisplayCells(dpy, scr)  (DefaultVisual(dpy,scr)->map_entries)
#define ScreenCount(dpy)        (((_XPrivDisplay)dpy)->nscreens)
#define ServerVendor(dpy)       (((_XPrivDisplay)dpy)->vendor)
#define ProtocolVersion(dpy)    (((_XPrivDisplay)dpy)->proto_major_version)
#define ProtocolRevision(dpy)   (((_XPrivDisplay)dpy)->proto_minor_version)
#define VendorRelease(dpy)      (((_XPrivDisplay)dpy)->release)
#define DisplayString(dpy)      (((_XPrivDisplay)dpy)->display_name)
#define DefaultDepth(dpy, scr)  (ScreenOfDisplay(dpy,scr)->root_depth)
#define DefaultColormap(dpy, scr)(ScreenOfDisplay(dpy,scr)->cmap)
#define BitmapUnit(dpy)         (((_XPrivDisplay)dpy)->bitmap_unit)
#define BitmapBitOrder(dpy)     (((_XPrivDisplay)dpy)->bitmap_bit_order)
#define BitmapPad(dpy)          (((_XPrivDisplay)dpy)->bitmap_pad)
#define ImageByteOrder(dpy)     (((_XPrivDisplay)dpy)->byte_order)
#ifdef CRAY /* unable to get WORD64 without pulling in other symbols */
#define NextRequest(dpy)        XNextRequest(dpy)
#else
#define NextRequest(dpy)        (((_XPrivDisplay)dpy)->request + 1)
#endif
#define LastKnownRequestProcessed(dpy)  (((_XPrivDisplay)dpy)->last_request_read)

/* macros for screen oriented applications (toolkit) */
#define ScreenOfDisplay(dpy, scr)(&((_XPrivDisplay)dpy)->screens[scr])
#define DefaultScreenOfDisplay(dpy) ScreenOfDisplay(dpy,DefaultScreen(dpy))
#define DisplayOfScreen(s)      ((s)->display)
#define RootWindowOfScreen(s)   ((s)->root)
#define BlackPixelOfScreen(s)   ((s)->black_pixel)
#define WhitePixelOfScreen(s)   ((s)->white_pixel)
#define DefaultColormapOfScreen(s)((s)->cmap)
#define DefaultDepthOfScreen(s) ((s)->root_depth)
#define DefaultGCOfScreen(s)    ((s)->default_gc)
#define DefaultVisualOfScreen(s)((s)->root_visual)
#define WidthOfScreen(s)        ((s)->width)
#define HeightOfScreen(s)       ((s)->height)
#define WidthMMOfScreen(s)      ((s)->mwidth)
#define HeightMMOfScreen(s)     ((s)->mheight)
#define PlanesOfScreen(s)       ((s)->root_depth)
#define CellsOfScreen(s)        (DefaultVisualOfScreen((s))->map_entries)
#define MinCmapsOfScreen(s)     ((s)->min_maps)
#define MaxCmapsOfScreen(s)     ((s)->max_maps)
#define DoesSaveUnders(s)       ((s)->save_unders)
#define DoesBackingStore(s)     ((s)->backing_store)
#define EventMaskOfScreen(s)    ((s)->root_input_mask)

/*
 * Extensions need a way to hang private data on some structures.
 */
typedef struct _XExtData {
        int number;             /* number returned by XRegisterExtension */
        struct _XExtData *next; /* next item on list of data for structure */
        int (*free_private)();  /* called to free private storage */
        XPointer private_data;  /* data private to this extension. */
} XExtData;

/*
 * This file contains structures used by the extension mechanism.
 */
typedef struct {                /* public to extension, cannot be changed */
        int extension;          /* extension number */
        int major_opcode;       /* major op-code assigned by server */
        int first_event;        /* first event number for the extension */
        int first_error;        /* first error number for the extension */
} XExtCodes;

/*
 * Data structure for retrieving info about pixmap formats.
 */

typedef struct {
    int depth;
    int bits_per_pixel;
    int scanline_pad;
} XPixmapFormatValues;


/*
 * Data structure for setting graphics context.
 */
typedef struct {
        int function;           /* logical operation */
        unsigned long plane_mask;/* plane mask */
        unsigned long foreground;/* foreground pixel */
        unsigned long background;/* background pixel */
        int line_width;         /* line width */
        int line_style;         /* LineSolid, LineOnOffDash, LineDoubleDash */
        int cap_style;          /* CapNotLast, CapButt, 
                                   CapRound, CapProjecting */
        int join_style;         /* JoinMiter, JoinRound, JoinBevel */
        int fill_style;         /* FillSolid, FillTiled, 
                                   FillStippled, FillOpaeueStippled */
        int fill_rule;          /* EvenOddRule, WindingRule */
        int arc_mode;           /* ArcChord, ArcPieSlice */
        Pixmap tile;            /* tile pixmap for tiling operations */
        Pixmap stipple;         /* stipple 1 plane pixmap for stipping */
        int ts_x_origin;        /* offset for tile or stipple operations */
        int ts_y_origin;
        Font font;              /* default text font for text operations */
        int subwindow_mode;     /* ClipByChildren, IncludeInferiors */
        Bool graphics_exposures;/* boolean, should exposures be generated */
        int clip_x_origin;      /* origin for clipping */
        int clip_y_origin;
        Pixmap clip_mask;       /* bitmap clipping; other calls for rects */
        int dash_offset;        /* patterned/dashed line information */
        char dashes;
} XGCValues;

/*
 * Graphics context.  The contents of this structure are implementation
 * dependent.  A GC should be treated as opaque by application code.
 */

typedef struct _XGC
#ifdef XLIB_ILLEGAL_ACCESS
{
    XExtData *ext_data; /* hook for extension to hang data */
    GContext gid;       /* protocol ID for graphics context */
    /* there is more to this structure, but it is private to Xlib */
}
#endif
*GC;

/*
 * Visual structure; contains information about colormapping possible.
 */
typedef struct {
        XExtData *ext_data;     /* hook for extension to hang data */
        VisualID visualid;      /* visual id of this visual */
#if defined(__cplusplus) || defined(c_plusplus)
        int c_class;            /* C++ class of screen (monochrome, etc.) */
#else
        int class;              /* class of screen (monochrome, etc.) */
#endif
        unsigned long red_mask, green_mask, blue_mask;  /* mask values */
        int bits_per_rgb;       /* log base 2 of distinct color values */
        int map_entries;        /* color map entries */
} Visual;

/*
 * Depth structure; contains information for each possible depth.
 */     
typedef struct {
        int depth;              /* this depth (Z) of the depth */
        int nvisuals;           /* number of Visual types at this depth */
        Visual *visuals;        /* list of visuals possible at this depth */
} Depth;

/*
 * Information about the screen.  The contents of this structure are
 * implementation dependent.  A Screen should be treated as opaque
 * by application code.
 */

struct _XDisplay;               /* Forward declare before use for C++ */

typedef struct {
        XExtData *ext_data;     /* hook for extension to hang data */
        struct _XDisplay *display;/* back pointer to display structure */
        Window root;            /* Root window id. */
        int width, height;      /* width and height of screen */
        int mwidth, mheight;    /* width and height of  in millimeters */
        int ndepths;            /* number of depths possible */
        Depth *depths;          /* list of allowable depths on the screen */
        int root_depth;         /* bits per pixel */
        Visual *root_visual;    /* root visual */
        GC default_gc;          /* GC for the root root visual */
        Colormap cmap;          /* default color map */
        unsigned long white_pixel;
        unsigned long black_pixel;      /* White and Black pixel values */
        int max_maps, min_maps; /* max and min color maps */
        int backing_store;      /* Never, WhenMapped, Always */
        Bool save_unders;       
        long root_input_mask;   /* initial root input mask */
} Screen;

/*
 * Format structure; describes ZFormat data the screen will understand.
 */
typedef struct {
        XExtData *ext_data;     /* hook for extension to hang data */
        int depth;              /* depth of this image format */
        int bits_per_pixel;     /* bits/pixel at this depth */
        int scanline_pad;       /* scanline must padded to this multiple */
} ScreenFormat;

/*
 * Data structure for setting window attributes.
 */
typedef struct {
    Pixmap background_pixmap;   /* background or None or ParentRelative */
    unsigned long background_pixel;     /* background pixel */
    Pixmap border_pixmap;       /* border of the window */
    unsigned long border_pixel; /* border pixel value */
    int bit_gravity;            /* one of bit gravity values */
    int win_gravity;            /* one of the window gravity values */
    int backing_store;          /* NotUseful, WhenMapped, Always */
    unsigned long backing_planes;/* planes to be preseved if possible */
    unsigned long backing_pixel;/* value to use in restoring planes */
    Bool save_under;            /* should bits under be saved? (popups) */
    long event_mask;            /* set of events that should be saved */
    long do_not_propagate_mask; /* set of events that should not propagate */
    Bool override_redirect;     /* boolean value for override-redirect */
    Colormap colormap;          /* color map to be associated with window */
    Cursor cursor;              /* cursor to be displayed (or None) */
} XSetWindowAttributes;

typedef struct {
    int x, y;                   /* location of window */
    int width, height;          /* width and height of window */
    int border_width;           /* border width of window */
    int depth;                  /* depth of window */
    Visual *visual;             /* the associated visual structure */
    Window root;                /* root of screen containing window */
#if defined(__cplusplus) || defined(c_plusplus)
    int c_class;                /* C++ InputOutput, InputOnly*/
#else
    int class;                  /* InputOutput, InputOnly*/
#endif
    int bit_gravity;            /* one of bit gravity values */
    int win_gravity;            /* one of the window gravity values */
    int backing_store;          /* NotUseful, WhenMapped, Always */
    unsigned long backing_planes;/* planes to be preserved if possible */
    unsigned long backing_pixel;/* value to be used when restoring planes */
    Bool save_under;            /* boolean, should bits under be saved? */
    Colormap colormap;          /* color map to be associated with window */
    Bool map_installed;         /* boolean, is color map currently installed*/
    int map_state;              /* IsUnmapped, IsUnviewable, IsViewable */
    long all_event_masks;       /* set of events all people have interest in*/
    long your_event_mask;       /* my event mask */
    long do_not_propagate_mask; /* set of events that should not propagate */
    Bool override_redirect;     /* boolean value for override-redirect */
    Screen *screen;             /* back pointer to correct screen */
} XWindowAttributes;

/*
 * Data structure for host setting; getting routines.
 *
 */

typedef struct {
        int family;             /* for example FamilyInternet */
        int length;             /* length of address, in bytes */
        char *address;          /* pointer to where to find the bytes */
} XHostAddress;

/*
 * Data structure for "image" data, used by image manipulation routines.
 */
typedef struct _XImage {
    int width, height;          /* size of image */
    int xoffset;                /* number of pixels offset in X direction */
    int format;                 /* XYBitmap, XYPixmap, ZPixmap */
    char *data;                 /* pointer to image data */
    int byte_order;             /* data byte order, LSBFirst, MSBFirst */
    int bitmap_unit;            /* quant. of scanline 8, 16, 32 */
    int bitmap_bit_order;       /* LSBFirst, MSBFirst */
    int bitmap_pad;             /* 8, 16, 32 either XY or ZPixmap */
    int depth;                  /* depth of image */
    int bytes_per_line;         /* accelarator to next line */
    int bits_per_pixel;         /* bits per pixel (ZPixmap) */
    unsigned long red_mask;     /* bits in z arrangment */
    unsigned long green_mask;
    unsigned long blue_mask;
    XPointer obdata;            /* hook for the object routines to hang on */
    struct funcs {              /* image manipulation routines */
        struct _XImage *(*create_image)();
#if NeedFunctionPrototypes
        int (*destroy_image)        (struct _XImage *);
        unsigned long (*get_pixel)  (struct _XImage *, int, int);
        int (*put_pixel)            (struct _XImage *, int, int, unsigned long);
        struct _XImage *(*sub_image)(struct _XImage *, int, int, unsigned int, unsigned int);
        int (*add_pixel)            (struct _XImage *, long);
#else
        int (*destroy_image)();
        unsigned long (*get_pixel)();
        int (*put_pixel)();
        struct _XImage *(*sub_image)();
        int (*add_pixel)();
#endif
        } f;
} XImage;

/* 
 * Data structure for XReconfigureWindow
 */
typedef struct {
    int x, y;
    int width, height;
    int border_width;
    Window sibling;
    int stack_mode;
} XWindowChanges;

/*
 * Data structure used by color operations
 */
typedef struct {
        unsigned long pixel;
        unsigned short red, green, blue;
        char flags;  /* do_red, do_green, do_blue */
        char pad;
} XColor;

/* 
 * Data structures for graphics operations.  On most machines, these are
 * congruent with the wire protocol structures, so reformatting the data
 * can be avoided on these architectures.
 */
typedef struct {
    short x1, y1, x2, y2;
} XSegment;

typedef struct {
    short x, y;
} XPoint;
    
typedef struct {
    short x, y;
    unsigned short width, height;
} XRectangle;
    
typedef struct {
    short x, y;
    unsigned short width, height;
    short angle1, angle2;
} XArc;


/* Data structure for XChangeKeyboardControl */

typedef struct {
        int key_click_percent;
        int bell_percent;
        int bell_pitch;
        int bell_duration;
        int led;
        int led_mode;
        int key;
        int auto_repeat_mode;   /* On, Off, Default */
} XKeyboardControl;

/* Data structure for XGetKeyboardControl */

typedef struct {
        int key_click_percent;
        int bell_percent;
        unsigned int bell_pitch, bell_duration;
        unsigned long led_mask;
        int global_auto_repeat;
        char auto_repeats[32];
} XKeyboardState;

/* Data structure for XGetMotionEvents.  */

typedef struct {
        Time time;
        short x, y;
} XTimeCoord;

/* Data structure for X{Set,Get}ModifierMapping */

typedef struct {
        int max_keypermod;      /* The server's max # of keys per modifier */
        KeyCode *modifiermap;   /* An 8 by max_keypermod array of modifiers */
} XModifierKeymap;


/*
 * Display datatype maintaining display specific data.
 * The contents of this structure are implementation dependent.
 * A Display should be treated as opaque by application code.
 */
#ifndef XLIB_ILLEGAL_ACCESS
typedef struct _XDisplay Display;
#endif

struct _XPrivate;               /* Forward declare before use for C++ */
struct _XrmHashBucketRec;

typedef struct 
#ifdef XLIB_ILLEGAL_ACCESS
_XDisplay
#endif
{
        XExtData *ext_data;     /* hook for extension to hang data */
        struct _XPrivate *private1;
        int fd;                 /* Network socket. */
        int private2;
        int proto_major_version;/* major version of server's X protocol */
        int proto_minor_version;/* minor version of servers X protocol */
        char *vendor;           /* vendor of the server hardware */
        XID private3;
        XID private4;
        XID private5;
        int private6;
        XID (*resource_alloc)();/* allocator function */
        int byte_order;         /* screen byte order, LSBFirst, MSBFirst */
        int bitmap_unit;        /* padding and data requirements */
        int bitmap_pad;         /* padding requirements on bitmaps */
        int bitmap_bit_order;   /* LeastSignificant or MostSignificant */
        int nformats;           /* number of pixmap formats in list */
        ScreenFormat *pixmap_format;    /* pixmap format list */
        int private8;
        int release;            /* release of the server */
        struct _XPrivate *private9, *private10;
        int qlen;               /* Length of input event queue */
        unsigned long last_request_read; /* seq number of last event read */
        unsigned long request;  /* sequence number of last request. */
        XPointer private11;
        XPointer private12;
        XPointer private13;
        XPointer private14;
        unsigned max_request_size; /* maximum number 32 bit words in request*/
        struct _XrmHashBucketRec *db;
        int (*private15)();
        char *display_name;     /* "host:display" string used on this connect*/
        int default_screen;     /* default screen for operations */
        int nscreens;           /* number of screens on this server*/
        Screen *screens;        /* pointer to list of screens */
        unsigned long motion_buffer;    /* size of motion buffer */
        unsigned long private16;
        int min_keycode;        /* minimum defined keycode */
        int max_keycode;        /* maximum defined keycode */
        XPointer private17;
        XPointer private18;
        int private19;
        char *xdefaults;        /* contents of defaults from server */
        /* there is more to this structure, but it is private to Xlib */
}
#ifdef XLIB_ILLEGAL_ACCESS
Display, 
#endif
*_XPrivDisplay;

#if NeedFunctionPrototypes      /* prototypes require event type definitions */
#undef _XEVENT_
#endif
#ifndef _XEVENT_
/*
 * Definitions of specific events.
 */
typedef struct {
        int type;               /* of event */
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;          /* "event" window it is reported relative to */
        Window root;            /* root window that the event occured on */
        Window subwindow;       /* child window */
        Time time;              /* milliseconds */
        int x, y;               /* pointer x, y coordinates in event window */
        int x_root, y_root;     /* coordinates relative to root */
        unsigned int state;     /* key or button mask */
        unsigned int keycode;   /* detail */
        Bool same_screen;       /* same screen flag */
} XKeyEvent;
typedef XKeyEvent XKeyPressedEvent;
typedef XKeyEvent XKeyReleasedEvent;

typedef struct {
        int type;               /* of event */
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;          /* "event" window it is reported relative to */
        Window root;            /* root window that the event occured on */
        Window subwindow;       /* child window */
        Time time;              /* milliseconds */
        int x, y;               /* pointer x, y coordinates in event window */
        int x_root, y_root;     /* coordinates relative to root */
        unsigned int state;     /* key or button mask */
        unsigned int button;    /* detail */
        Bool same_screen;       /* same screen flag */
} XButtonEvent;
typedef XButtonEvent XButtonPressedEvent;
typedef XButtonEvent XButtonReleasedEvent;

typedef struct {
        int type;               /* of event */
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;          /* "event" window reported relative to */
        Window root;            /* root window that the event occured on */
        Window subwindow;       /* child window */
        Time time;              /* milliseconds */
        int x, y;               /* pointer x, y coordinates in event window */
        int x_root, y_root;     /* coordinates relative to root */
        unsigned int state;     /* key or button mask */
        char is_hint;           /* detail */
        Bool same_screen;       /* same screen flag */
} XMotionEvent;
typedef XMotionEvent XPointerMovedEvent;

typedef struct {
        int type;               /* of event */
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;          /* "event" window reported relative to */
        Window root;            /* root window that the event occured on */
        Window subwindow;       /* child window */
        Time time;              /* milliseconds */
        int x, y;               /* pointer x, y coordinates in event window */
        int x_root, y_root;     /* coordinates relative to root */
        int mode;               /* NotifyNormal, NotifyGrab, NotifyUngrab */
        int detail;
        /*
         * NotifyAncestor, NotifyVirtual, NotifyInferior, 
         * NotifyNonlinear,NotifyNonlinearVirtual
         */
        Bool same_screen;       /* same screen flag */
        Bool focus;             /* boolean focus */
        unsigned int state;     /* key or button mask */
} XCrossingEvent;
typedef XCrossingEvent XEnterWindowEvent;
typedef XCrossingEvent XLeaveWindowEvent;

typedef struct {
        int type;               /* FocusIn or FocusOut */
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;          /* window of event */
        int mode;               /* NotifyNormal, NotifyGrab, NotifyUngrab */
        int detail;
        /*
         * NotifyAncestor, NotifyVirtual, NotifyInferior, 
         * NotifyNonlinear,NotifyNonlinearVirtual, NotifyPointer,
         * NotifyPointerRoot, NotifyDetailNone 
         */
} XFocusChangeEvent;
typedef XFocusChangeEvent XFocusInEvent;
typedef XFocusChangeEvent XFocusOutEvent;

/* generated on EnterWindow and FocusIn  when KeyMapState selected */
typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;
        char key_vector[32];
} XKeymapEvent; 

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;
        int x, y;
        int width, height;
        int count;              /* if non-zero, at least this many more */
} XExposeEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Drawable drawable;
        int x, y;
        int width, height;
        int count;              /* if non-zero, at least this many more */
        int major_code;         /* core is CopyArea or CopyPlane */
        int minor_code;         /* not defined in the core */
} XGraphicsExposeEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Drawable drawable;
        int major_code;         /* core is CopyArea or CopyPlane */
        int minor_code;         /* not defined in the core */
} XNoExposeEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;
        int state;              /* Visibility state */
} XVisibilityEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window parent;          /* parent of the window */
        Window window;          /* window id of window created */
        int x, y;               /* window location */
        int width, height;      /* size of window */
        int border_width;       /* border width */
        Bool override_redirect; /* creation should be overridden */
} XCreateWindowEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window event;
        Window window;
} XDestroyWindowEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window event;
        Window window;
        Bool from_configure;
} XUnmapEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window event;
        Window window;
        Bool override_redirect; /* boolean, is override set... */
} XMapEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window parent;
        Window window;
} XMapRequestEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window event;
        Window window;
        Window parent;
        int x, y;
        Bool override_redirect;
} XReparentEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window event;
        Window window;
        int x, y;
        int width, height;
        int border_width;
        Window above;
        Bool override_redirect;
} XConfigureEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window event;
        Window window;
        int x, y;
} XGravityEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;
        int width, height;
} XResizeRequestEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window parent;
        Window window;
        int x, y;
        int width, height;
        int border_width;
        Window above;
        int detail;             /* Above, Below, TopIf, BottomIf, Opposite */
        unsigned long value_mask;
} XConfigureRequestEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window event;
        Window window;
        int place;              /* PlaceOnTop, PlaceOnBottom */
} XCirculateEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window parent;
        Window window;
        int place;              /* PlaceOnTop, PlaceOnBottom */
} XCirculateRequestEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;
        Atom atom;
        Time time;
        int state;              /* NewValue, Deleted */
} XPropertyEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;
        Atom selection;
        Time time;
} XSelectionClearEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window owner;
        Window requestor;
        Atom selection;
        Atom target;
        Atom property;
        Time time;
} XSelectionRequestEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window requestor;
        Atom selection;
        Atom target;
        Atom property;          /* ATOM or None */
        Time time;
} XSelectionEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;
        Colormap colormap;      /* COLORMAP or None */
#if defined(__cplusplus) || defined(c_plusplus)
        Bool c_new;             /* C++ */
#else
        Bool new;
#endif
        int state;              /* ColormapInstalled, ColormapUninstalled */
} XColormapEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;
        Atom message_type;
        int format;
        union {
                char b[20];
                short s[10];
                long l[5];
                } data;
} XClientMessageEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;       /* Display the event was read from */
        Window window;          /* unused */
        int request;            /* one of MappingModifier, MappingKeyboard,
                                   MappingPointer */
        int first_keycode;      /* first keycode */
        int count;              /* defines range of change w. first_keycode*/
} XMappingEvent;

typedef struct {
        int type;
        Display *display;       /* Display the event was read from */
        XID resourceid;         /* resource id */
        unsigned long serial;   /* serial number of failed request */
        unsigned char error_code;       /* error code of failed request */
        unsigned char request_code;     /* Major op-code of failed request */
        unsigned char minor_code;       /* Minor op-code of failed request */
} XErrorEvent;

typedef struct {
        int type;
        unsigned long serial;   /* # of last request processed by server */
        Bool send_event;        /* true if this came from a SendEvent request */
        Display *display;/* Display the event was read from */
        Window window;  /* window on which event was requested in event mask */
} XAnyEvent;

/*
 * this union is defined so Xlib can always use the same sized
 * event structure internally, to avoid memory fragmentation.
 */
typedef union _XEvent {
        int type;               /* must not be changed; first element */
        XAnyEvent xany;
        XKeyEvent xkey;
        XButtonEvent xbutton;
        XMotionEvent xmotion;
        XCrossingEvent xcrossing;
        XFocusChangeEvent xfocus;
        XExposeEvent xexpose;
        XGraphicsExposeEvent xgraphicsexpose;
        XNoExposeEvent xnoexpose;
        XVisibilityEvent xvisibility;
        XCreateWindowEvent xcreatewindow;
        XDestroyWindowEvent xdestroywindow;
        XUnmapEvent xunmap;
        XMapEvent xmap;
        XMapRequestEvent xmaprequest;
        XReparentEvent xreparent;
        XConfigureEvent xconfigure;
        XGravityEvent xgravity;
        XResizeRequestEvent xresizerequest;
        XConfigureRequestEvent xconfigurerequest;
        XCirculateEvent xcirculate;
        XCirculateRequestEvent xcirculaterequest;
        XPropertyEvent xproperty;
        XSelectionClearEvent xselectionclear;
        XSelectionRequestEvent xselectionrequest;
        XSelectionEvent xselection;
        XColormapEvent xcolormap;
        XClientMessageEvent xclient;
        XMappingEvent xmapping;
        XErrorEvent xerror;
        XKeymapEvent xkeymap;
        long pad[24];
} XEvent;
#endif

#define XAllocID(dpy) ((*((_XPrivDisplay)dpy)->resource_alloc)((dpy)))

/*
 * per character font metric information.
 */
typedef struct {
    short       lbearing;       /* origin to left edge of raster */
    short       rbearing;       /* origin to right edge of raster */
    short       width;          /* advance to next char's origin */
    short       ascent;         /* baseline to top edge of raster */
    short       descent;        /* baseline to bottom edge of raster */
    unsigned short attributes;  /* per char flags (not predefined) */
} XCharStruct;

/*
 * To allow arbitrary information with fonts, there are additional properties
 * returned.
 */
typedef struct {
    Atom name;
    unsigned long card32;
} XFontProp;

typedef struct {
    XExtData    *ext_data;      /* hook for extension to hang data */
    Font        fid;            /* Font id for this font */
    unsigned    direction;      /* hint about direction the font is painted */
    unsigned    min_char_or_byte2;/* first character */
    unsigned    max_char_or_byte2;/* last character */
    unsigned    min_byte1;      /* first row that exists */
    unsigned    max_byte1;      /* last row that exists */
    Bool        all_chars_exist;/* flag if all characters have non-zero size*/
    unsigned    default_char;   /* char to print for undefined character */
    int         n_properties;   /* how many properties there are */
    XFontProp   *properties;    /* pointer to array of additional properties*/
    XCharStruct min_bounds;     /* minimum bounds over all existing char*/
    XCharStruct max_bounds;     /* maximum bounds over all existing char*/
    XCharStruct *per_char;      /* first_char to last_char information */
    int         ascent;         /* log. extent above baseline for spacing */
    int         descent;        /* log. descent below baseline for spacing */
} XFontStruct;

/*
 * PolyText routines take these as arguments.
 */
typedef struct {
    char *chars;                /* pointer to string */
    int nchars;                 /* number of characters */
    int delta;                  /* delta between strings */
    Font font;                  /* font to print it in, None don't change */
} XTextItem;

typedef struct {                /* normal 16 bit characters are two bytes */
    unsigned char byte1;
    unsigned char byte2;
} XChar2b;

typedef struct {
    XChar2b *chars;             /* two byte characters */
    int nchars;                 /* number of characters */
    int delta;                  /* delta between strings */
    Font font;                  /* font to print it in, None don't change */
} XTextItem16;


typedef union { Display *display;
                GC gc;
                Visual *visual;
                Screen *screen;
                ScreenFormat *pixmap_format;
                XFontStruct *font; } XEDataObject;

typedef struct {
    XRectangle      max_ink_extent;
    XRectangle      max_logical_extent;
} XFontSetExtents;

typedef void (*XOMProc)();

typedef struct _XOM *XOM;
typedef struct _XOC *XOC, *XFontSet;

typedef struct {
    char           *chars;
    int             nchars;
    int             delta;
    XFontSet        font_set;
} XmbTextItem;

typedef struct {
    wchar_t        *chars;
    int             nchars;
    int             delta;
    XFontSet        font_set;
} XwcTextItem;

#define XNRequiredCharSet "requiredCharSet"
#define XNQueryOrientation "queryOrientation"
#define XNBaseFontName "baseFontName"
#define XNOMAutomatic "omAutomatic"
#define XNMissingCharSet "missingCharSet"
#define XNDefaultString "defaultString"
#define XNOrientation "orientation"
#define XNDirectionalDependentDrawing "directionalDependentDrawing"
#define XNContextualDrawing "contextualDrawing"
#define XNFontInfo "fontInfo"

typedef struct {
    int charset_count;
    char **charset_list;
} XOMCharSetList;

typedef enum {
    XOMOrientation_LTR_TTB,
    XOMOrientation_RTL_TTB,
    XOMOrientation_TTB_LTR,
    XOMOrientation_TTB_RTL,
    XOMOrientation_Context
} XOrientation;

typedef struct {
    int num_orient;
    XOrientation *orient;       /* Input Text description */
} XOMOrientation;

typedef struct {
    int num_font;
    XFontStruct **font_struct_list;
    char **font_name_list;
} XOMFontInfo;

typedef void (*XIMProc)();

typedef struct _XIM *XIM;
typedef struct _XIC *XIC;

typedef unsigned long XIMStyle;

typedef struct {
    unsigned short count_styles;
    XIMStyle *supported_styles;
} XIMStyles;

#define XIMPreeditArea          0x0001L
#define XIMPreeditCallbacks     0x0002L
#define XIMPreeditPosition      0x0004L
#define XIMPreeditNothing       0x0008L
#define XIMPreeditNone          0x0010L
#define XIMStatusArea           0x0100L
#define XIMStatusCallbacks      0x0200L
#define XIMStatusNothing        0x0400L
#define XIMStatusNone           0x0800L

#define XNVaNestedList "XNVaNestedList"
#define XNQueryInputStyle "queryInputStyle"
#define XNClientWindow "clientWindow"
#define XNInputStyle "inputStyle"
#define XNFocusWindow "focusWindow"
#define XNResourceName "resourceName"
#define XNResourceClass "resourceClass"
#define XNGeometryCallback "geometryCallback"
#define XNDestroyCallback "destroyCallback"
#define XNFilterEvents "filterEvents"
#define XNPreeditStartCallback "preeditStartCallback"
#define XNPreeditDoneCallback "preeditDoneCallback"
#define XNPreeditDrawCallback "preeditDrawCallback"
#define XNPreeditCaretCallback "preeditCaretCallback"
#define XNPreeditStateNotifyCallback "preeditStateNotifyCallback"
#define XNPreeditAttributes "preeditAttributes"
#define XNStatusStartCallback "statusStartCallback"
#define XNStatusDoneCallback "statusDoneCallback"
#define XNStatusDrawCallback "statusDrawCallback"
#define XNStatusAttributes "statusAttributes"
#define XNArea "area"
#define XNAreaNeeded "areaNeeded"
#define XNSpotLocation "spotLocation"
#define XNColormap "colorMap"
#define XNStdColormap "stdColorMap"
#define XNForeground "foreground"
#define XNBackground "background"
#define XNBackgroundPixmap "backgroundPixmap"
#define XNFontSet "fontSet"
#define XNLineSpace "lineSpace"
#define XNCursor "cursor"

#define XNQueryIMValuesList "queryIMValuesList"
#define XNQueryICValuesList "queryICValuesList"
#define XNVisiblePosition "visiblePosition"
#define XNR6PreeditCallback "r6PreeditCallback"
#define XNStringConversionCallback "stringConversionCallback"
#define XNStringConversion "stringConversion"
#define XNResetState "resetState"
#define XNHotKey "hotKey"
#define XNHotKeyState "hotKeyState"
#define XNPreeditState "preeditState"
#define XNSeparatorofNestedList "separatorofNestedList"

#define XBufferOverflow         -1
#define XLookupNone             1
#define XLookupChars            2
#define XLookupKeySym           3
#define XLookupBoth             4

#if NeedFunctionPrototypes
typedef void *XVaNestedList;
#else
typedef XPointer XVaNestedList;
#endif

typedef struct {
    XPointer client_data;
    XIMProc callback;
} XIMCallback;

typedef unsigned long XIMFeedback;

#define XIMReverse              1L
#define XIMUnderline            (1L<<1) 
#define XIMHighlight            (1L<<2)
#define XIMPrimary              (1L<<5)
#define XIMSecondary            (1L<<6)
#define XIMTertiary             (1L<<7)
#define XIMVisibleToForward     (1L<<8)
#define XIMVisibleToBackword    (1L<<9)
#define XIMVisibleToCenter      (1L<<10)

typedef struct _XIMText {
    unsigned short length;
    XIMFeedback *feedback;
    Bool encoding_is_wchar; 
    union {
        char *multi_byte;
        wchar_t *wide_char;
    } string; 
} XIMText;

typedef unsigned long    XIMPreeditState;

#define XIMPreeditUnKnown       0L
#define XIMPreeditEnable        1L
#define XIMPreeditDisable       (1L<<1)

typedef struct  _XIMPreeditStateNotifyCallbackStruct {
    XIMPreeditState state;
} XIMPreeditStateNotifyCallbackStruct;

typedef unsigned long    XIMResetState;

#define XIMInitialState         1L
#define XIMPreserveState        (1L<<1)

typedef unsigned long XIMStringConversionFeedback;

#define XIMStringConversionLeftEdge     (0x00000001)
#define XIMStringConversionRightEdge    (0x00000002)
#define XIMStringConversionTopEdge      (0x00000004)
#define XIMStringConversionBottomEdge   (0x00000008)
#define XIMStringConversionConcealed    (0x00000010)
#define XIMStringConversionWrapped      (0x00000020)

typedef struct _XIMStringConversionText {
    unsigned short length;
    XIMStringConversionFeedback *feedback;
    Bool encoding_is_wchar; 
    union {
        char *mbs;
        wchar_t *wcs;
    } string; 
} XIMStringConversionText;

typedef unsigned short  XIMStringConversionPosition;

typedef unsigned short  XIMStringConversionType;

#define XIMStringConversionBuffer       (0x0001)
#define XIMStringConversionLine         (0x0002)
#define XIMStringConversionWord         (0x0003)
#define XIMStringConversionChar         (0x0004)

typedef unsigned short  XIMStringConversionOperation;

#define XIMStringConversionSubstitution (0x0001)
#define XIMStringConversionRetrival     (0x0002)

typedef struct _XIMStringConversionCallbackStruct {
    XIMStringConversionPosition position;
    XIMStringConversionType type;
    XIMStringConversionOperation operation;
    unsigned short factor;
    XIMStringConversionText *text;
} XIMStringConversionCallbackStruct;

typedef struct _XIMPreeditDrawCallbackStruct {
    int caret;          /* Cursor offset within pre-edit string */
    int chg_first;      /* Starting change position */
    int chg_length;     /* Length of the change in character count */
    XIMText *text;
} XIMPreeditDrawCallbackStruct;

typedef enum {
    XIMForwardChar, XIMBackwardChar,
    XIMForwardWord, XIMBackwardWord,
    XIMCaretUp, XIMCaretDown,
    XIMNextLine, XIMPreviousLine,
    XIMLineStart, XIMLineEnd, 
    XIMAbsolutePosition,
    XIMDontChange
} XIMCaretDirection;

typedef enum {
    XIMIsInvisible,     /* Disable caret feedback */ 
    XIMIsPrimary,       /* UI defined caret feedback */
    XIMIsSecondary      /* UI defined caret feedback */
} XIMCaretStyle;

typedef struct _XIMPreeditCaretCallbackStruct {
    int position;                /* Caret offset within pre-edit string */
    XIMCaretDirection direction; /* Caret moves direction */
    XIMCaretStyle style;         /* Feedback of the caret */
} XIMPreeditCaretCallbackStruct;

typedef enum {
    XIMTextType,
    XIMBitmapType
} XIMStatusDataType;
        
typedef struct _XIMStatusDrawCallbackStruct {
    XIMStatusDataType type;
    union {
        XIMText *text;
        Pixmap  bitmap;
    } data;
} XIMStatusDrawCallbackStruct;

typedef struct _XIMHotKeyTrigger {
    KeySym       keysym;
    int          modifier;
    int          modifier_mask;
} XIMHotKeyTrigger;

typedef struct _XIMHotKeyTriggers {
    int                  num_hot_key;
    XIMHotKeyTrigger    *key;
} XIMHotKeyTriggers;

typedef unsigned long    XIMHotKeyState;

#define XIMHotKeyStateON        (0x0001L)
#define XIMHotKeyStateOFF       (0x0002L)

typedef struct {
    unsigned short count_values;
    char **supported_values;
} XIMValuesList;

#if defined(WIN32) && !defined(_XLIBINT_)
#define _Xdebug (*_Xdebug_p)
#endif

int _Xdebug;

#if defined(_VISUALC_)
extern MagickExport XFontStruct *XLoadQueryFont(
#else
XFontStruct *XLoadQueryFont(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* name */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFontStruct *XQueryFont(
#else
XFontStruct *XQueryFont(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XID b               /* font_ID */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XTimeCoord *XGetMotionEvents(
#else
XTimeCoord *XGetMotionEvents(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Time c              /* start */,
    Time d              /* stop */,
    int* e              /* nevents_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XModifierKeymap *XDeleteModifiermapEntry(
#else
XModifierKeymap *XDeleteModifiermapEntry(
#endif
#if NeedFunctionPrototypes
    XModifierKeymap* a  /* modmap */,
#if NeedWidePrototypes
    unsigned int b      /* keycode_entry */,
#else
    KeyCode     c       /* keycode_entry */,
#endif
    int d               /* modifier */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XModifierKeymap     *XGetModifierMapping(
#else
XModifierKeymap *XGetModifierMapping(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XModifierKeymap     *XInsertModifiermapEntry(
#else
XModifierKeymap *XInsertModifiermapEntry(
#endif
#if NeedFunctionPrototypes
    XModifierKeymap* b  /* modmap */,
#if NeedWidePrototypes
    unsigned int c      /* keycode_entry */,
#else
    KeyCode     d       /* keycode_entry */,
#endif
    int e               /* modifier */    
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XModifierKeymap *XNewModifiermap(
#else
XModifierKeymap *XNewModifiermap(
#endif
#if NeedFunctionPrototypes
    int a               /* max_keys_per_mod */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XImage *XCreateImage(
#else
XImage *XCreateImage(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Visual*     b       /* visual */,
    unsigned int c      /* depth */,
    int d               /* format */,
    int e               /* offset */,
    char* f             /* data */,
    unsigned int g      /* width */,
    unsigned int h      /* height */,
    int i               /* bitmap_pad */,
    int j               /* bytes_per_line */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Status XInitImage(
#else
Status XInitImage(
#endif
#if NeedFunctionPrototypes
    XImage*     a       /* image */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport XImage *XGetImage(
#else
XImage *XGetImage(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* d */,
    int c               /* x */,
    int d               /* y */,
    unsigned int e      /* width */,
    unsigned int f      /* height */,
    unsigned long g     /* plane_mask */,
    int h               /* format */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport XImage *XGetSubImage(
#else
XImage *XGetSubImage(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* d */,
    int c               /* x */,
    int d               /* y */,
    unsigned int e      /* width */,
    unsigned int f      /* height */,
    unsigned long g     /* plane_mask */,
    int h               /* format */,
    XImage*     i       /* dest_image */,
    int j               /* dest_x */,
    int k               /* dest_y */
#endif
){}

/* 
 * X function declarations.
 */
#if defined(_VISUALC_)
extern MagickExport Display *XOpenDisplay(server_name)
#else
Display *XOpenDisplay(server_name)
#endif
const char
  *server_name;
{
  return((Display *) NULL);
}

#if defined(_VISUALC_)
extern MagickExport void XrmInitialize(
#else
void XrmInitialize(
#endif
#if NeedFunctionPrototypes
    void * a
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XFetchBytes(
#else
char *XFetchBytes(
#endif
#if NeedFunctionPrototypes
    Display* b          /* display */,
    int* c              /* nbytes_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char *XFetchBuffer(
#else
char *XFetchBuffer(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int* b              /* nbytes_return */,
    int c               /* buffer */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char *XGetAtomName(
#else
char *XGetAtomName(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Atom b              /* atom */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Status XGetAtomNames(
#else
Status XGetAtomNames(
#endif
#if NeedFunctionPrototypes
    Display* a          /* dpy */,
    Atom* b             /* atoms */,
    int c               /* count */,
    char** d            /* names_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char *XGetDefault(
#else
char *XGetDefault(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* program */,
    _Xconst char* c     /* option */              
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char *XDisplayName(_Xconst char* a)
#else
char *XDisplayName(_Xconst char* a)
#endif
{}
#if defined(_VISUALC_)
extern MagickExport char *XKeysymToString(
#else
char *XKeysymToString(
#endif
#if NeedFunctionPrototypes
    KeySym      a       /* keysym */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int (*XSynchronize(
#else
int (*XSynchronize(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Bool b              /* onoff */
#endif
)){};
#if defined(_VISUALC_)
extern MagickExport int (*XSetAfterFunction(
#else
int (*XSetAfterFunction(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int (* b) (
#if NeedNestedPrototypes
             Display* a /* display */
#endif
            )           /* procedure */
#endif
)){};
#if defined(_VISUALC_)
extern MagickExport Atom XInternAtom(
#else
Atom XInternAtom(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* atom_name */,
    Bool c              /* only_if_exists */             
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Status XInternAtoms(
#else
Status XInternAtoms(
#endif
#if NeedFunctionPrototypes
    Display* a          /* dpy */,
    char** b            /* names */,
    int c               /* count */,
    Bool d              /* onlyIfExists */,
    Atom* e             /* atoms_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Colormap XCopyColormapAndFree(
#else
Colormap XCopyColormapAndFree(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Colormap XCreateColormap(
#else
Colormap XCreateColormap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Visual*     c       /* visual */,
    int d               /* alloc */                      
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Cursor XCreatePixmapCursor(
#else
Cursor XCreatePixmapCursor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Pixmap b            /* source */,
    Pixmap c    /* mask */,
    XColor*     d       /* foreground_color */,
    XColor*     e       /* background_color */,
    unsigned int f      /* x */,
    unsigned int g      /* y */                    
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Cursor XCreateGlyphCursor(
#else
Cursor XCreateGlyphCursor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Font b              /* source_font */,
    Font c              /* mask_font */,
    unsigned int d      /* source_char */,
    unsigned int e      /* mask_char */,
    XColor*     f       /* foreground_color */,
    XColor*     g       /* background_color */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Cursor XCreateFontCursor(
#else
Cursor XCreateFontCursor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    unsigned int b      /* shape */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Font XLoadFont(
#else
Font XLoadFont(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* name */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport GC XCreateGC(
#else
GC XCreateGC(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* d */,
    unsigned long c     /* valuemask */,
    XGCValues* d                /* values */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport GContext XGContextFromGC(
#else
GContext XGContextFromGC(
#endif
#if NeedFunctionPrototypes
    GC a                        /* gc */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport void XFlushGC(
#else
void XFlushGC(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC  b               /* gc */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Pixmap XCreatePixmap(
#else
Pixmap XCreatePixmap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* d */,
    unsigned int c      /* width */,
    unsigned int d      /* height */,
    unsigned int e      /* depth */                     
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Pixmap XCreateBitmapFromData(
#else
Pixmap XCreateBitmapFromData(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* d */,
    _Xconst char* c     /* data */,
    unsigned int d      /* width */,
    unsigned int e      /* height */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Pixmap XCreatePixmapFromBitmapData(
#else
Pixmap XCreatePixmapFromBitmapData(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* d */,
    char* c             /* data */,
    unsigned int d      /* width */,
    unsigned int e      /* height */,
    unsigned long f     /* fg */,
    unsigned long g     /* bg */,
    unsigned int h      /* depth */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Window XCreateSimpleWindow(
#else
Window XCreateSimpleWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* parent */,
    int c               /* x */,
    int d               /* y */,
    unsigned int e      /* width */,
    unsigned int f      /* height */,
    unsigned int g      /* border_width */,
    unsigned long h     /* border */,
    unsigned long i     /* background */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Window XGetSelectionOwner(
#else
Window XGetSelectionOwner(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Atom b              /* selection */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Window XCreateWindow(
#else
Window XCreateWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* parent */,
    int c               /* x */,
    int d               /* y */,
    unsigned int e      /* width */,
    unsigned int f      /* height */,
    unsigned int g      /* border_width */,
    int h               /* depth */,
    unsigned int i      /* class */,
    Visual*     j       /* visual */,
    unsigned long k     /* valuemask */,
    XSetWindowAttributes* l     /* attributes */
#endif
){} 
#if defined(_VISUALC_)
extern MagickExport Colormap *XListInstalledColormaps(
#else
Colormap *XListInstalledColormaps(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    int* c              /* num_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char **XListFonts(
#else
char **XListFonts(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* pattern */,
    int c               /* maxnames */,
    int* d              /* actual_count_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char **XListFontsWithInfo(
#else
char **XListFontsWithInfo(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* pattern */,
    int c               /* maxnames */,
    int* d              /* count_return */,
    XFontStruct** e     /* info_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char **XGetFontPath(
#else
char **XGetFontPath(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int* b              /* npaths_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char **XListExtensions(
#else
char **XListExtensions(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int* b              /* nextensions_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Atom *XListProperties(
#else
Atom *XListProperties(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    int* c              /* num_prop_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport XHostAddress *XListHosts(
#else
XHostAddress *XListHosts(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int* b              /* nhosts_return */,
    Bool* c             /* state_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport KeySym XKeycodeToKeysym(
#else
KeySym XKeycodeToKeysym(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
#if NeedWidePrototypes
    unsigned int b      /* keycode */,
#else
    KeyCode     c       /* keycode */,
#endif
    int d               /* index */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport KeySym XLookupKeysym(
#else
KeySym XLookupKeysym(
#endif
#if NeedFunctionPrototypes
    XKeyEvent* a                /* key_event */,
    int b               /* index */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport KeySym *XGetKeyboardMapping(
#else
KeySym *XGetKeyboardMapping(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
#if NeedWidePrototypes
    unsigned int b      /* first_keycode */,
#else
    KeyCode     c       /* first_keycode */,
#endif
    int d               /* keycode_count */,
    int* e              /* keysyms_per_keycode_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport KeySym XStringToKeysym(
#else
KeySym XStringToKeysym(
#endif
#if NeedFunctionPrototypes
    _Xconst char* a     /* string */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport long XMaxRequestSize(
#else
long XMaxRequestSize(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport long XExtendedMaxRequestSize(
#else
long XExtendedMaxRequestSize(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char *XResourceManagerString(
#else
char *XResourceManagerString(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char *XScreenResourceString(
#else
char *XScreenResourceString(
#endif
#if NeedFunctionPrototypes
        Screen* a       /* screen */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport unsigned long XDisplayMotionBufferSize(
#else
unsigned long XDisplayMotionBufferSize(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport VisualID XVisualIDFromVisual(
#else
VisualID XVisualIDFromVisual(
#endif
#if NeedFunctionPrototypes
    Visual*     a       /* visual */
#endif
){}

/* multithread routines */

#if defined(_VISUALC_)
extern MagickExport Status XInitThreads(
#else
Status XInitThreads(
#endif
#if NeedFunctionPrototypes
    void *a
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XLockDisplay(
#else
void XLockDisplay(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XUnlockDisplay(
#else
void XUnlockDisplay(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

/* routines for dealing with extensions */

#if defined(_VISUALC_)
extern MagickExport XExtCodes *XInitExtension(
#else
XExtCodes *XInitExtension(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* name */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XExtCodes *XAddExtension(
#else
XExtCodes *XAddExtension(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport XExtData *XFindOnExtensionList(
#else
XExtData *XFindOnExtensionList(
#endif
#if NeedFunctionPrototypes
    XExtData** a                /* structure */,
    int b               /* number */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport XExtData **XEHeadOfExtensionList(
#else
XExtData **XEHeadOfExtensionList(
#endif
#if NeedFunctionPrototypes
    XEDataObject a      /* object */
#endif
){}

/* these are routines for which there are also macros */
#if defined(_VISUALC_)
extern MagickExport Window XRootWindow(Display* a,int b)
#else
Window XRootWindow(Display* a,int b)
#endif
{}
#if defined(_VISUALC_)
extern MagickExport Window XDefaultRootWindow(
#else
Window XDefaultRootWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Window XRootWindowOfScreen(
#else
Window XRootWindowOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     a       /* screen */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Visual *XDefaultVisual(
#else
Visual *XDefaultVisual(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Visual *XDefaultVisualOfScreen(
#else
Visual *XDefaultVisualOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     a       /* screen */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport GC XDefaultGC(
#else
GC XDefaultGC(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport GC XDefaultGCOfScreen(
#else
GC XDefaultGCOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     a       /* screen */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport unsigned long XBlackPixel(
#else
unsigned long XBlackPixel(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport unsigned long XWhitePixel(
#else
unsigned long XWhitePixel(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport unsigned long XAllPlanes(
#else
unsigned long XAllPlanes(
#endif
#if NeedFunctionPrototypes
    void *a
#endif
){}
#if defined(_VISUALC_)
extern MagickExport unsigned long XBlackPixelOfScreen(
#else
unsigned long XBlackPixelOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     a       /* screen */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport unsigned long XWhitePixelOfScreen(
#else
unsigned long XWhitePixelOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     a       /* screen */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport unsigned long XNextRequest(
#else
unsigned long XNextRequest(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport unsigned long XLastKnownRequestProcessed(
#else
unsigned long XLastKnownRequestProcessed(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char *XServerVendor(
#else
char *XServerVendor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport char *XDisplayString(
#else
char *XDisplayString(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Colormap XDefaultColormap(
#else
Colormap XDefaultColormap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Colormap XDefaultColormapOfScreen(
#else
Colormap XDefaultColormapOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     a       /* screen */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Display *XDisplayOfScreen(
#else
Display *XDisplayOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     a       /* screen */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Screen *XScreenOfDisplay(
#else
Screen *XScreenOfDisplay(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Screen *XDefaultScreenOfDisplay(Display* a)
#else
Screen *XDefaultScreenOfDisplay(Display* a)
#endif
{}
#if defined(_VISUALC_)
extern MagickExport long XEventMaskOfScreen(
#else
long XEventMaskOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     a       /* screen */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XScreenNumberOfScreen(
#else
int XScreenNumberOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     a       /* screen */
#endif
){}

typedef int (*XErrorHandler) (      /* WARNING, this type not in Xlib spec */
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XErrorEvent* b      /* error_event */
#endif
);


#if defined(_VISUALC_)
extern MagickExport XErrorHandler XSetErrorHandler (
#else
XErrorHandler XSetErrorHandler (
#endif
#if NeedFunctionPrototypes
    XErrorHandler a     /* handler */
#endif
){}


typedef int (*XIOErrorHandler) (    /* WARNING, this type not in Xlib spec */
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
);

#if defined(_VISUALC_)
extern MagickExport XIOErrorHandler XSetIOErrorHandler (
#else
XIOErrorHandler XSetIOErrorHandler (
#endif
#if NeedFunctionPrototypes
    XIOErrorHandler     a/* handler */
#endif
){}


#if defined(_VISUALC_)
extern MagickExport XPixmapFormatValues *XListPixmapFormats(
#else
XPixmapFormatValues *XListPixmapFormats(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int* b              /* count_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport int *XListDepths(
#else
int *XListDepths(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */,
    int* c              /* count_return */
#endif
){}

/* ICCCM routines for things that don't require special include files; */
/* other declarations are given in Xutil.h                             */
#if defined(_VISUALC_)
extern MagickExport Status XReconfigureWMWindow(
#else
Status XReconfigureWMWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    int c               /* screen_number */,
    unsigned int d      /* mask */,
    XWindowChanges*     e /* changes */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetWMProtocols(
#else
Status XGetWMProtocols(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Atom** c            /* protocols_return */,
    int* d              /* count_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Status XSetWMProtocols(
#else
Status XSetWMProtocols(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Atom* c             /* protocols */,
    int d               /* count */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Status XIconifyWindow(
#else
Status XIconifyWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    int c               /* screen_number */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Status XWithdrawWindow(
#else
Status XWithdrawWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    int c               /* screen_number */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Status XGetCommand(
#else
Status XGetCommand(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    char***     c       /* argv_return */,
    int* d              /* argc_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Status XGetWMColormapWindows(
#else
Status XGetWMColormapWindows(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Window** c          /* windows_return */,
    int* d              /* count_return */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport Status XSetWMColormapWindows(
#else
Status XSetWMColormapWindows(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Window*     c       /* colormap_windows */,
    int d               /* count */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport void XFreeStringList(
#else
void XFreeStringList(
#endif
#if NeedFunctionPrototypes
    char** a            /* list */
#endif
){}
#if defined(_VISUALC_)
extern MagickExport XSetTransientForHint(
#else
XSetTransientForHint(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Window c            /* prop_window */
#endif
){}

/* The following are given in alphabetical order */

#if defined(_VISUALC_)
extern MagickExport XActivateScreenSaver(
#else
XActivateScreenSaver(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XAddHost(
#else
XAddHost(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XHostAddress* b     /* host */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XAddHosts(
#else
XAddHosts(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XHostAddress* b     /* hosts */,
    int c               /* num_hosts */    
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XAddToExtensionList(
#else
XAddToExtensionList(
#endif
#if NeedFunctionPrototypes
    struct _XExtData** a        /* structure */,
    XExtData* b         /* ext_data */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XAddToSaveSet(
#else
XAddToSaveSet(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XAllocColor(
#else
Status XAllocColor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    XColor*     c       /* screen_in_out */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XAllocColorCells(
#else
Status XAllocColorCells(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    Bool c              /* contig */,
    unsigned long* d    /* plane_masks_return */,
    unsigned int e      /* nplanes */,
    unsigned long* f    /* pixels_return */,
    unsigned int g      /* npixels */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XAllocColorPlanes(
#else
Status XAllocColorPlanes(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    Bool c              /* contig */,
    unsigned long* x    /* pixels_return */,
    int d               /* ncolors */,
    int e               /* nreds */,
    int f               /* ngreens */,
    int g               /* nblues */,
    unsigned long* h    /* rmask_return */,
    unsigned long* i    /* gmask_return */,
    unsigned long* j    /* bmask_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XAllocNamedColor(
#else
Status XAllocNamedColor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    _Xconst char* c     /* color_name */,
    XColor*     d       /* screen_def_return */,
    XColor*     e       /* exact_def_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XAllowEvents(
#else
XAllowEvents(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* event_mode */,
    Time c              /* time */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XAutoRepeatOff(
#else
XAutoRepeatOff(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XAutoRepeatOn(
#else
XAutoRepeatOn(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XBell(
#else
XBell(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* percent */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XBitmapBitOrder(
#else
int XBitmapBitOrder(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XBitmapPad(
#else
int XBitmapPad(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XBitmapUnit(
#else
int XBitmapUnit(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XCellsOfScreen(
#else
int XCellsOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     a       /* screen */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XChangeActivePointerGrab(
#else
XChangeActivePointerGrab(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    unsigned int b      /* event_mask */,
    Cursor c            /* cursor */,
    Time d              /* time */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XChangeGC(
#else
XChangeGC(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC b                        /* gc */,
    unsigned long c     /* valuemask */,
    XGCValues* d        /* values */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XChangeKeyboardControl(
#else
XChangeKeyboardControl(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    unsigned long b     /* value_mask */,
    XKeyboardControl* c /* values */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XChangeKeyboardMapping(
#else
XChangeKeyboardMapping(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* first_keycode */,
    int c               /* keysyms_per_keycode */,
    KeySym*     d       /* keysyms */,
    int e               /* num_codes */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XChangePointerControl(
#else
XChangePointerControl(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Bool b              /* do_accel */,
    Bool c              /* do_threshold */,
    int d               /* accel_numerator */,
    int e               /* accel_denominator */,
    int f               /* threshold */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XChangeProperty(
#else
XChangeProperty(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Atom c              /* property */,
    Atom d              /* type */,
    int e               /* format */,
    int f               /* mode */,
    _Xconst unsigned char* g    /* data */,
    int h               /* nelements */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XChangeSaveSet(
#else
XChangeSaveSet(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    int c               /* change_mode */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XChangeWindowAttributes(
#else
XChangeWindowAttributes(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    unsigned long c     /* valuemask */,
    XSetWindowAttributes* d /* attributes */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XCheckIfEvent(
#else
Bool XCheckIfEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XEvent* b           /* event_return */,
    Bool (* c) (
#if NeedNestedPrototypes
               Display* x               /* display */,
               XEvent* y                        /* event */,
               XPointer z               /* arg */
#endif
             )          /* predicate */,
    XPointer d          /* arg */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XCheckMaskEvent(
#else
Bool XCheckMaskEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    long b              /* event_mask */,
    XEvent*     c       /* event_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XCheckTypedEvent(
#else
Bool XCheckTypedEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* event_type */,
    XEvent*     c       /* event_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XCheckTypedWindowEvent(
#else
Bool XCheckTypedWindowEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    int c               /* event_type */,
    XEvent*     d       /* event_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XCheckWindowEvent(
#else
Bool XCheckWindowEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    long c              /* event_mask */,
    XEvent*     d       /* event_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XCirculateSubwindows(
#else
XCirculateSubwindows(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    int c               /* direction */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XCirculateSubwindowsDown(
#else
XCirculateSubwindowsDown(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XCirculateSubwindowsUp(
#else
XCirculateSubwindowsUp(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XClearArea(
#else
XClearArea(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    int c               /* x */,
    int d               /* y */,
    unsigned int e      /* width */,
    unsigned int f      /* height */,
    Bool g              /* exposures */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XClearWindow(
#else
XClearWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XCloseDisplay(
#else
XCloseDisplay(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XConfigureWindow(
#else
XConfigureWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    unsigned int c      /* value_mask */,
    XWindowChanges*     d /* values */           
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XConnectionNumber(
#else
int XConnectionNumber(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XConvertSelection(
#else
XConvertSelection(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Atom b              /* selection */,
    Atom c              /* target */,
    Atom d              /* property */,
    Window e    /* requestor */,
    Time f              /* time */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XCopyArea(
#else
XCopyArea(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* src */,
    Drawable c          /* dest */,
    GC  d               /* gc */,
    int  e              /* src_x */,
    int  f              /* src_y */,
    unsigned int g      /* width */,
    unsigned int h      /* height */,
    int i               /* dest_x */,
    int j               /* dest_y */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XCopyGC(
#else
XCopyGC(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC  b               /* src */,
    unsigned long c     /* valuemask */,
    GC  d               /* dest */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XCopyPlane(
#else
XCopyPlane(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* src */,
    Drawable c          /* dest */,
    GC  d               /* gc */,
    int e               /* src_x */,
    int f               /* src_y */,
    unsigned int g      /* width */,
    unsigned int h      /* height */,
    int i               /* dest_x */,
    int j               /* dest_y */,
    unsigned long k     /* plane */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XDefaultDepth(
#else
int XDefaultDepth(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XDefaultDepthOfScreen(
#else
int XDefaultDepthOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     a       /* screen */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XDefaultScreen(Display* a)
#else
int XDefaultScreen(Display* a)
#endif
{}

#if defined(_VISUALC_)
extern MagickExport XDefineCursor(
#else
XDefineCursor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Cursor c            /* cursor */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDeleteProperty(
#else
XDeleteProperty(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window      b       /* w */,
    Atom c              /* property */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDestroyWindow(
#else
XDestroyWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDestroySubwindows(
#else
XDestroySubwindows(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XDoesBackingStore(
#else
int XDoesBackingStore(
#endif
#if NeedFunctionPrototypes
    Screen* c           /* screen */    
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XDoesSaveUnders(
#else
Bool XDoesSaveUnders(
#endif
#if NeedFunctionPrototypes
    Screen* c           /* screen */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDisableAccessControl(
#else
XDisableAccessControl(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}


#if defined(_VISUALC_)
extern MagickExport int XDisplayCells(
#else
int XDisplayCells(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XDisplayHeight(
#else
int XDisplayHeight(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XDisplayHeightMM(
#else
int XDisplayHeightMM(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDisplayKeycodes(
#else
XDisplayKeycodes(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int* b              /* min_keycodes_return */,
    int* c              /* max_keycodes_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XDisplayPlanes(
#else
int XDisplayPlanes(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XDisplayWidth(
#else
int XDisplayWidth(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XDisplayWidthMM(
#else
int XDisplayWidthMM(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawArc(
#else
XDrawArc(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* d */,
    GC c                        /* gc */,
    int d               /* x */,
    int e               /* y */,
    unsigned int f      /* width */,
    unsigned int g      /* height */,
    int h               /* angle1 */,
    int i               /* angle2 */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawArcs(
#else
XDrawArcs(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* d */,
    GC c                        /* gc */,
    XArc* d             /* arcs */,
    int e               /* narcs */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawImageString(
#else
XDrawImageString(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* d */,
    GC c                        /* gc */,
    int d               /* x */,
    int e               /* y */,
    _Xconst char* f     /* string */,
    int g               /* length */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawImageString16(
#else
XDrawImageString16(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* d */,
    GC c                        /* gc */,
    int d               /* x */,
    int e               /* y */,
    _Xconst XChar2b* f  /* string */,
    int g               /* length */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawLine(
#else
XDrawLine(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable b          /* d */,
    GC c                        /* gc */,
    int d               /* x1 */,
    int e               /* x2 */,
    int f               /* y1 */,
    int g               /* y2 */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawLines(
#else
XDrawLines(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    XPoint*     f       /* points */,
    int g               /* npoints */,
    int h               /* mode */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawPoint(
#else
XDrawPoint(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    int f               /* x */,
    int g               /* y */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawPoints(
#else
XDrawPoints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    XPoint*     f       /* points */,
    int g               /* npoints */,
    int h               /* mode */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawRectangle(
#else
XDrawRectangle(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    int f               /* x */,
    int g               /* y */,
    unsigned int h      /* width */,
    unsigned int i      /* height */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawRectangles(
#else
XDrawRectangles(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    XRectangle* f       /* rectangles */,
    int g               /* nrectangles */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawSegments(
#else
XDrawSegments(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    XSegment* f         /* segments */,
    int g                       /* nsegments */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawString(
#else
XDrawString(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    int f               /* x */,
    int g               /* y */,
    _Xconst char* h     /* string */,
    int i               /* length */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawString16(
#else
XDrawString16(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    int f               /* x */,
    int g               /* y */,
    _Xconst XChar2b* h  /* string */,
    int i               /* length */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawText(
#else
XDrawText(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    int f               /* x */,
    int g               /* y */,
    XTextItem* h                /* items */,
    int i               /* nitems */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDrawText16(
#else
XDrawText16(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    int f                       /* x */,
    int g               /* y */,
    XTextItem16* h      /* items */,
    int i               /* nitems */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XEnableAccessControl(
#else
XEnableAccessControl(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XEventsQueued(
#else
int XEventsQueued(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* mode */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XFetchName(
#else
Status XFetchName(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    char** c            /* window_name_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFillArc(
#else
XFillArc(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    int f               /* x */,
    int g               /* y */,
    unsigned int h      /* width */,
    unsigned int i      /* height */,
    int j               /* angle1 */,
    int k               /* angle2 */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFillArcs(
#else
XFillArcs(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    XArc* f             /* arcs */,
    int g               /* narcs */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFillPolygon(
#else
XFillPolygon(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    XPoint*     f       /* points */,
    int g                       /* npoints */,
    int h               /* shape */,
    int i               /* mode */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFillRectangle(
#else
XFillRectangle(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    int f               /* x */,
    int g               /* y */,
    unsigned int h      /* width */,
    unsigned int i      /* height */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFillRectangles(
#else
XFillRectangles(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    XRectangle* f       /* rectangles */,
    int g               /* nrectangles */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFlush(
#else
XFlush(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XForceScreenSaver(
#else
XForceScreenSaver(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* mode */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFree(
#else
XFree(
#endif
#if NeedFunctionPrototypes
    void* a             /* data */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFreeColormap(
#else
XFreeColormap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFreeColors(
#else
XFreeColors(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    unsigned long* c    /* pixels */,
    int d               /* npixels */,
    unsigned long e     /* planes */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFreeCursor(
#else
XFreeCursor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Cursor b            /* cursor */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFreeExtensionList(
#else
XFreeExtensionList(
#endif
#if NeedFunctionPrototypes
    char** a            /* list */    
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFreeFont(
#else
XFreeFont(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XFontStruct* b      /* font_struct */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFreeFontInfo(
#else
XFreeFontInfo(
#endif
#if NeedFunctionPrototypes
    char** a            /* names */,
    XFontStruct* b      /* free_info */,
    int c               /* actual_count */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFreeFontNames(
#else
XFreeFontNames(
#endif
#if NeedFunctionPrototypes
    char** a            /* list */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFreeFontPath(
#else
XFreeFontPath(
#endif
#if NeedFunctionPrototypes
    char** a            /* list */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFreeGC(
#else
XFreeGC(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFreeModifiermap(
#else
XFreeModifiermap(
#endif
#if NeedFunctionPrototypes
    XModifierKeymap* a  /* modmap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFreePixmap(
#else
XFreePixmap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Pixmap b            /* pixmap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XGeometry(
#else
int XGeometry(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen */,
    _Xconst char* c     /* position */,
    _Xconst char* d     /* default_position */,
    unsigned int e      /* bwidth */,
    unsigned int f      /* fwidth */,
    unsigned int g      /* fheight */,
    int h               /* xadder */,
    int i               /* yadder */,
    int* j              /* x_return */,
    int* k              /* y_return */,
    int* l              /* width_return */,
    int* m              /* height_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XGetErrorDatabaseText(
#else
XGetErrorDatabaseText(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* name */,
    _Xconst char* c     /* message */,
    _Xconst char* d     /* default_string */,
    char* e             /* buffer_return */,
    int f               /* length */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XGetErrorText(
#else
XGetErrorText(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* code */,
    char* c             /* buffer_return */,
    int d               /* length */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XGetFontProperty(
#else
Bool XGetFontProperty(
#endif
#if NeedFunctionPrototypes
    XFontStruct* a      /* font_struct */,
    Atom b              /* atom */,
    unsigned long* c    /* value_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetGCValues(
#else
Status XGetGCValues(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    unsigned long f     /* valuemask */,
    XGCValues* g                /* values_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetGeometry(
#else
Status XGetGeometry(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    Window*     e       /* root_return */,
    int* f              /* x_return */,
    int* g              /* y_return */,
    unsigned int* h     /* width_return */,
    unsigned int* i     /* height_return */,
    unsigned int* j     /* border_width_return */,
    unsigned int* k     /* depth_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetIconName(
#else
Status XGetIconName(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    char** c            /* icon_name_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XGetInputFocus(
#else
XGetInputFocus(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window*     b       /* focus_return */,
    int* c              /* revert_to_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XGetKeyboardControl(
#else
XGetKeyboardControl(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XKeyboardState*     b /* values_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XGetPointerControl(
#else
XGetPointerControl(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int* b              /* accel_numerator_return */,
    int* c              /* accel_denominator_return */,
    int* d              /* threshold_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XGetPointerMapping(
#else
int XGetPointerMapping(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    unsigned char* b    /* map_return */,
    int c               /* nmap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XGetScreenSaver(
#else
XGetScreenSaver(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int* b              /* timeout_return */,
    int* c              /* interval_return */,
    int* d              /* prefer_blanking_return */,
    int* e              /* allow_exposures_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetTransientForHint(
#else
Status XGetTransientForHint(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window      b       /* w */,
    Window*      c      /* prop_window_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XGetWindowProperty(
#else
int XGetWindowProperty(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Atom c              /* property */,
    long d              /* long_offset */,
    long e              /* long_length */,
    Bool f              /* delete */,
    Atom g              /* req_type */,
    Atom* h             /* actual_type_return */,
    int* i              /* actual_format_return */,
    unsigned long* j    /* nitems_return */,
    unsigned long* k    /* bytes_after_return */,
    unsigned char**     l /* prop_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetWindowAttributes(
#else
Status XGetWindowAttributes(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XWindowAttributes* c        /* window_attributes_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XGrabButton(
#else
XGrabButton(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    unsigned int b      /* button */,
    unsigned int c      /* modifiers */,
    Window d            /* grab_window */,
    Bool e              /* owner_events */,
    unsigned int f      /* event_mask */,
    int g               /* pointer_mode */,
    int h               /* keyboard_mode */,
    Window      i       /* confine_to */,
    Cursor      j       /* cursor */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XGrabKey(
#else
XGrabKey(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* keycode */,
    unsigned int c      /* modifiers */,
    Window      d       /* grab_window */,
    Bool e              /* owner_events */,
    int f               /* pointer_mode */,
    int g               /* keyboard_mode */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XGrabKeyboard(
#else
int XGrabKeyboard(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* grab_window */,
    Bool c              /* owner_events */,
    int d               /* pointer_mode */,
    int e               /* keyboard_mode */,
    Time f              /* time */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XGrabPointer(
#else
int XGrabPointer(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* grab_window */,
    Bool c              /* owner_events */,
    unsigned int d      /* event_mask */,
    int e               /* pointer_mode */,
    int f               /* keyboard_mode */,
    Window g            /* confine_to */,
    Cursor h            /* cursor */,
    Time i              /* time */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XGrabServer(
#else
XGrabServer(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XHeightMMOfScreen(
#else
int XHeightMMOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen* c           /* screen */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XHeightOfScreen(
#else
int XHeightOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen* c           /* screen */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XIfEvent(
#else
XIfEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XEvent*     b       /* event_return */,
    Bool (* c) (
#if NeedNestedPrototypes
               Display* x                       /* display */,
               XEvent* y                        /* event */,
               XPointer z               /* arg */
#endif
             )          /* predicate */,
    XPointer d          /* arg */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XImageByteOrder(
#else
int XImageByteOrder(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XInstallColormap(
#else
XInstallColormap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport KeyCode XKeysymToKeycode(
#else
KeyCode XKeysymToKeycode(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    KeySym b            /* keysym */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XKillClient(
#else
XKillClient(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XID b               /* resource */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport unsigned long XXLastKnownRequestProcessed(
#else
unsigned long XXLastKnownRequestProcessed(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XLookupColor(
#else
Status XLookupColor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    _Xconst char* c     /* color_name */,
    XColor*     d       /* exact_def_return */,
    XColor*     e       /* screen_def_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XLowerWindow(
#else
XLowerWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window      b       /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XMapRaised(
#else
XMapRaised(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XMapSubwindows(
#else
XMapSubwindows(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XMapWindow(
#else
XMapWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XMaskEvent(
#else
XMaskEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    long b              /* event_mask */,
    XEvent*     c       /* event_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XMaxCmapsOfScreen(
#else
int XMaxCmapsOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen* c           /* screen */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XMinCmapsOfScreen(
#else
int XMinCmapsOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen* c           /* screen */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XMoveResizeWindow(
#else
XMoveResizeWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    int c                       /* x */,
    int d               /* y */,
    unsigned int e      /* width */,
    unsigned int f      /* height */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XMoveWindow(
#else
XMoveWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    int c               /* x */,
    int d               /* y */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XNextEvent(
#else
XNextEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XEvent*     b       /* event_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XNoOp(
#else
XNoOp(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XParseColor(
#else
Status XParseColor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    _Xconst char* c     /* spec */,
    XColor*     d       /* exact_def_return */
#endif
){}

static int
ReadInteger(string, NextString)
register char *string;
char **NextString;
{
    register int Result = 0;
    int Sign = 1;
    
    if (*string == '+')
        string++;
    else if (*string == '-')
    {
        string++;
        Sign = -1;
    }
    for (; (*string >= '0') && (*string <= '9'); string++)
    {
        Result = (Result * 10) + (*string - '0');
    }
    *NextString = string;
    if (Sign >= 0)
        return (Result);
    else
        return (-Result);
}

#define NoValue         0x0000
#define XValue          0x0001
#define YValue          0x0002
#define WidthValue      0x0004
#define HeightValue     0x0008
#define AllValues       0x000F
#define XNegative       0x0010
#define YNegative       0x0020


#if NeedFunctionPrototypes
#if defined(_VISUALC_)
extern MagickExport int XParseGeometry (
#else
int XParseGeometry (
#endif
_Xconst char *string,
int *x,
int *y,
unsigned int *width,    /* RETURN */
unsigned int *height)    /* RETURN */
#else
int XParseGeometry (string, x, y, width, height)
char *string;
int *x, *y;
unsigned int *width, *height;    /* RETURN */
#endif
{
        int mask = NoValue;
        register char *strind;
        unsigned int tempWidth, tempHeight;
        int tempX, tempY;
        char *nextCharacter;

        if ( (string == NULL) || (*string == '\0')) return(mask);
        if (*string == '=')
                string++;  /* ignore possible '=' at beg of geometry spec */

        strind = (char *)string;
        if (*strind != '+' && *strind != '-' && *strind != 'x') {
                tempWidth = ReadInteger(strind, &nextCharacter);
                if (strind == nextCharacter) 
                    return (0);
                strind = nextCharacter;
                mask |= WidthValue;
        }

        if (*strind == 'x' || *strind == 'X') { 
                strind++;
                tempHeight = ReadInteger(strind, &nextCharacter);
                if (strind == nextCharacter)
                    return (0);
                strind = nextCharacter;
                mask |= HeightValue;
        }

        if ((*strind == '+') || (*strind == '-')) {
                if (*strind == '-') {
                        strind++;
                        tempX = -ReadInteger(strind, &nextCharacter);
                        if (strind == nextCharacter)
                            return (0);
                        strind = nextCharacter;
                        mask |= XNegative;

                }
                else
                {       strind++;
                        tempX = ReadInteger(strind, &nextCharacter);
                        if (strind == nextCharacter)
                            return(0);
                        strind = nextCharacter;
                }
                mask |= XValue;
                if ((*strind == '+') || (*strind == '-')) {
                        if (*strind == '-') {
                                strind++;
                                tempY = -ReadInteger(strind, &nextCharacter);
                                if (strind == nextCharacter)
                                    return(0);
                                strind = nextCharacter;
                                mask |= YNegative;

                        }
                        else
                        {
                                strind++;
                                tempY = ReadInteger(strind, &nextCharacter);
                                if (strind == nextCharacter)
                                    return(0);
                                strind = nextCharacter;
                        }
                        mask |= YValue;
                }
        }
        
        /* If strind isn't at the end of the string the it's an invalid
                geometry specification. */

        if (*strind != '\0') return (0);

        if (mask & XValue)
            *x = tempX;
        if (mask & YValue)
            *y = tempY;
        if (mask & WidthValue)
            *width = tempWidth;
        if (mask & HeightValue)
            *height = tempHeight;
        return (mask);
}

#if defined(_VISUALC_)
extern MagickExport XPeekEvent(
#else
XPeekEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XEvent*     b       /* event_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XPeekIfEvent(
#else
XPeekIfEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XEvent*     b       /* event_return */,
    Bool (* c) (
#if NeedNestedPrototypes
               Display* x               /* display */,
               XEvent* y                /* event */,
               XPointer z       /* arg */
#endif
             )          /* predicate */,
    XPointer d          /* arg */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XPending(
#else
int XPending(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XPlanesOfScreen(
#else
int XPlanesOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen* c           /* screen */
    
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XProtocolRevision(
#else
int XProtocolRevision(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XProtocolVersion(
#else
int XProtocolVersion(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}


#if defined(_VISUALC_)
extern MagickExport XPutBackEvent(
#else
XPutBackEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XEvent*     b       /* event */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XPutImage(
#else
XPutImage(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    XImage*     f       /* image */,
    int g               /* src_x */,
    int h               /* src_y */,
    int i               /* dest_x */,
    int j               /* dest_y */,
    unsigned int k      /* width */,
    unsigned int l      /* height */      
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XQLength(
#else
int XQLength(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XQueryBestCursor(
#else
Status XQueryBestCursor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    unsigned int e       /* width */,
    unsigned int f      /* height */,
    unsigned int* g     /* width_return */,
    unsigned int* h     /* height_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XQueryBestSize(
#else
Status XQueryBestSize(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* class */,
    Drawable d          /* which_screen */,
    unsigned int e      /* width */,
    unsigned int f      /* height */,
    unsigned int* g     /* width_return */,
    unsigned int* h     /* height_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XQueryBestStipple(
#else
Status XQueryBestStipple(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* which_screen */,
    unsigned int e      /* width */,
    unsigned int f      /* height */,
    unsigned int* g     /* width_return */,
    unsigned int* h     /* height_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XQueryBestTile(
#else
Status XQueryBestTile(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* which_screen */,
    unsigned int e      /* width */,
    unsigned int f      /* height */,
    unsigned int* g     /* width_return */,
    unsigned int* h     /* height_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XQueryColor(
#else
XQueryColor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    XColor*     c       /* def_in_out */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XQueryColors(
#else
XQueryColors(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    XColor*     c       /* defs_in_out */,
    int d               /* ncolors */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XQueryExtension(
#else
Bool XQueryExtension(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* name */,
    int* c              /* major_opcode_return */,
    int* d              /* first_event_return */,
    int* e              /* first_error_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XQueryKeymap(
#else
XQueryKeymap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    char b[32]          /* keys_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XQueryPointer(
#else
Bool XQueryPointer(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Window*     c       /* root_return */,
    Window*     d       /* child_return */,
    int* e              /* root_x_return */,
    int* f              /* root_y_return */,
    int* g              /* win_x_return */,
    int* h              /* win_y_return */,
    unsigned int*  i     /* mask_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XQueryTextExtents(
#else
XQueryTextExtents(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XID b               /* font_ID */,
    _Xconst char* c     /* string */,
    int d               /* nchars */,
    int* e              /* direction_return */,
    int* f              /* font_ascent_return */,
    int* g              /* font_descent_return */,
    XCharStruct* h      /* overall_return */    
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XQueryTextExtents16(
#else
XQueryTextExtents16(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XID b               /* font_ID */,
    _Xconst XChar2b* c  /* string */,
    int d               /* nchars */,
    int* e              /* direction_return */,
    int* f              /* font_ascent_return */,
    int* g              /* font_descent_return */,
    XCharStruct* h      /* overall_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XQueryTree(
#else
Status XQueryTree(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Window*     c       /* root_return */,
    Window*     d       /* parent_return */,
    Window** e          /* children_return */,
    unsigned int* f     /* nchildren_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XRaiseWindow(
#else
XRaiseWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window      b       /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XReadBitmapFile(
#else
int XReadBitmapFile(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    _Xconst char* e     /* filename */,
    unsigned int* f     /* width_return */,
    unsigned int* g     /* height_return */,
    Pixmap*     h       /* bitmap_return */,
    int* i              /* x_hot_return */,
    int* j              /* y_hot_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XReadBitmapFileData(
#else
int XReadBitmapFileData(
#endif
#if NeedFunctionPrototypes
    _Xconst char* a     /* filename */,
    unsigned int* b     /* width_return */,
    unsigned int* c     /* height_return */,
    unsigned char**     d /* data_return */,
    int* e              /* x_hot_return */,
    int* f              /* y_hot_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XRebindKeysym(
#else
XRebindKeysym(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    KeySym b            /* keysym */,
    KeySym*     c       /* list */,
    int d               /* mod_count */,
    _Xconst unsigned char* e    /* string */,
    int f               /* bytes_string */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XRecolorCursor(
#else
XRecolorCursor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Cursor b            /* cursor */,
    XColor*     c       /* foreground_color */,
    XColor*     d       /* background_color */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XRefreshKeyboardMapping(
#else
XRefreshKeyboardMapping(
#endif
#if NeedFunctionPrototypes
    XMappingEvent* a    /* event_map */    
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XRemoveFromSaveSet(
#else
XRemoveFromSaveSet(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XRemoveHost(
#else
XRemoveHost(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XHostAddress* b     /* host */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XRemoveHosts(
#else
XRemoveHosts(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XHostAddress* b     /* hosts */,
    int c               /* num_hosts */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XReparentWindow(
#else
XReparentWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Window c            /* parent */,
    int d               /* x */,
    int e               /* y */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XResetScreenSaver(
#else
XResetScreenSaver(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XResizeWindow(
#else
XResizeWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window      b       /* w */,
    unsigned int c      /* width */,
    unsigned int d      /* height */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XRestackWindows(
#else
XRestackWindows(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window*     b       /* windows */,
    int c               /* nwindows */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XRotateBuffers(
#else
XRotateBuffers(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* rotate */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XRotateWindowProperties(
#else
XRotateWindowProperties(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Atom* c             /* properties */,
    int d               /* num_prop */,
    int e               /* npositions */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XScreenCount(
#else
int XScreenCount(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSelectInput(
#else
XSelectInput(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window      b       /* w */,
    long c              /* event_mask */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XSendEvent(
#else
Status XSendEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Bool c              /* propagate */,
    long d              /* event_mask */,
    XEvent*     e       /* event_send */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetAccessControl(
#else
XSetAccessControl(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* mode */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetArcMode(
#else
XSetArcMode(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    int f               /* arc_mode */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetBackground(
#else
XSetBackground(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    unsigned long g     /* background */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetClipMask(
#else
XSetClipMask(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    Pixmap f            /* pixmap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetClipOrigin(
#else
XSetClipOrigin(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    int f               /* clip_x_origin */,
    int g               /* clip_y_origin */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetClipRectangles(
#else
XSetClipRectangles(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    int b               /* clip_x_origin */,
    int c               /* clip_y_origin */,
    XRectangle* d       /* rectangles */,
    int x               /* n */,
    int f               /* ordering */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetCloseDownMode(
#else
XSetCloseDownMode(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* close_mode */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetCommand(
#else
XSetCommand(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window      b       /* w */,
    char**      c       /* argv */,
    int d               /* argc */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetDashes(
#else
XSetDashes(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    int b               /* dash_offset */,
    _Xconst char* c     /* dash_list */,
    int d               /* n */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetFillRule(
#else
XSetFillRule(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    int f               /* fill_rule */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetFillStyle(
#else
XSetFillStyle(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    int b               /* fill_style */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetFont(
#else
XSetFont(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    Font b              /* font */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetFontPath(
#else
XSetFontPath(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    char**      b       /* directories */,
    int c               /* ndirs */          
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetForeground(
#else
XSetForeground(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    unsigned long b     /* foreground */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetFunction(
#else
XSetFunction(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    int b               /* function */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetGraphicsExposures(
#else
XSetGraphicsExposures(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    Bool b              /* graphics_exposures */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetIconName(
#else
XSetIconName(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window      b       /* w */,
    _Xconst char* c     /* icon_name */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetInputFocus(
#else
XSetInputFocus(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* focus */,
    int c               /* revert_to */,
    Time d              /* time */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetLineAttributes(
#else
XSetLineAttributes(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    unsigned int b      /* line_width */,
    int c               /* line_style */,
    int d               /* cap_style */,
    int f               /* join_style */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XSetModifierMapping(
#else
int XSetModifierMapping(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XModifierKeymap* b  /* modmap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetPlaneMask(
#else
XSetPlaneMask(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    unsigned long b     /* plane_mask */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XSetPointerMapping(
#else
int XSetPointerMapping(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst unsigned char* b    /* map */,
    int  c              /* nmap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetScreenSaver(
#else
XSetScreenSaver(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* timeout */,
    int c               /* interval */,
    int d               /* prefer_blanking */,
    int e               /* allow_exposures */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetSelectionOwner(
#else
XSetSelectionOwner(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Atom b              /* selection */,
    Window c            /* owner */,
    Time d              /* time */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetState(
#else
XSetState(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC x                        /* gc */,
    unsigned long b     /* foreground */,
    unsigned long c     /* background */,
    int d               /* function */,
    unsigned long e     /* plane_mask */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetStipple(
#else
XSetStipple(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    Pixmap      b       /* stipple */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetSubwindowMode(
#else
XSetSubwindowMode(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    int b               /* subwindow_mode */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetTSOrigin(
#else
XSetTSOrigin(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    int b               /* ts_x_origin */,
    int c               /* ts_y_origin */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetTile(
#else
XSetTile(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    Pixmap      b       /* tile */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetWindowBackground(
#else
XSetWindowBackground(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window      b       /* w */,
    unsigned long c     /* background_pixel */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetWindowBackgroundPixmap(
#else
XSetWindowBackgroundPixmap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Pixmap c            /* background_pixmap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetWindowBorder(
#else
XSetWindowBorder(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window c            /* w */,
    unsigned long d     /* border_pixel */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetWindowBorderPixmap(
#else
XSetWindowBorderPixmap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Pixmap c            /* border_pixmap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetWindowBorderWidth(
#else
XSetWindowBorderWidth(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    unsigned int c      /* width */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetWindowColormap(
#else
XSetWindowColormap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    Colormap c          /* colormap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XStoreBuffer(
#else
XStoreBuffer(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* bytes */,
    int c               /* nbytes */,
    int d               /* buffer */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XStoreBytes(
#else
XStoreBytes(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* bytes */,
    int c               /* nbytes */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XStoreColor(
#else
XStoreColor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    XColor*     c       /* color */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XStoreColors(
#else
XStoreColors(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    XColor*     c       /* color */,
    int d               /* ncolors */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XStoreName(
#else
XStoreName(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    _Xconst char* c     /* window_name */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XStoreNamedColor(
#else
XStoreNamedColor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */,
    _Xconst char* c     /* color */,
    unsigned long d     /* pixel */,
    int e               /* flags */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSync(Display* a,Bool b)
#else
XSync(Display* a,Bool b)
#endif
{}

#if defined(_VISUALC_)
extern MagickExport XTextExtents(
#else
XTextExtents(
#endif
#if NeedFunctionPrototypes
    XFontStruct* a      /* font_struct */,
    _Xconst char* b     /* string */,
    int c               /* nchars */,
    int* d              /* direction_return */,
    int* e              /* font_ascent_return */,
    int* f              /* font_descent_return */,
    XCharStruct* g      /* overall_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XTextExtents16(
#else
XTextExtents16(
#endif
#if NeedFunctionPrototypes
    XFontStruct* a      /* font_struct */,
    _Xconst XChar2b* b  /* string */,
    int c               /* nchars */,
    int* d              /* direction_return */,
    int* e              /* font_ascent_return */,
    int* f              /* font_descent_return */,
    XCharStruct* g      /* overall_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XTextWidth(
#else
int XTextWidth(
#endif
#if NeedFunctionPrototypes
    XFontStruct* a      /* font_struct */,
    _Xconst char* b     /* string */,
    int c               /* count */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XTextWidth16(
#else
int XTextWidth16(
#endif
#if NeedFunctionPrototypes
    XFontStruct* a      /* font_struct */,
    _Xconst XChar2b* b  /* string */,
    int c               /* count */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XTranslateCoordinates(
#else
Bool XTranslateCoordinates(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* src_w */,
    Window c            /* dest_w */,
    int d               /* src_x */,
    int e               /* src_y */,
    int* f              /* dest_x_return */,
    int* g              /* dest_y_return */,
    Window*     h       /* child_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUndefineCursor(
#else
XUndefineCursor(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUngrabButton(
#else
XUngrabButton(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    unsigned int b      /* button */,
    unsigned int c      /* modifiers */,
    Window      d       /* grab_window */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUngrabKey(
#else
XUngrabKey(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* keycode */,
    unsigned int c      /* modifiers */,
    Window d            /* grab_window */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUngrabKeyboard(
#else
XUngrabKeyboard(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Time b              /* time */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUngrabPointer(
#else
XUngrabPointer(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Time b              /* time */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUngrabServer(
#else
XUngrabServer(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUninstallColormap(
#else
XUninstallColormap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Colormap b          /* colormap */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUnloadFont(
#else
XUnloadFont(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Font b              /* font */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUnmapSubwindows(
#else
XUnmapSubwindows(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUnmapWindow(
#else
XUnmapWindow(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XVendorRelease(
#else
int XVendorRelease(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XWarpPointer(
#else
XWarpPointer(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* src_w */,
    Window c            /* dest_w */,
    int d               /* src_x */,
    int e               /* src_y */,
    unsigned int f      /* src_width */,
    unsigned int g      /* src_height */,
    int h               /* dest_x */,
    int i               /* dest_y */         
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XWidthMMOfScreen(
#else
int XWidthMMOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen* c           /* screen */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XWidthOfScreen(
#else
int XWidthOfScreen(
#endif
#if NeedFunctionPrototypes
    Screen*     c       /* screen */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XWindowEvent(
#else
XWindowEvent(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    long c              /* event_mask */,
    XEvent*     d       /* event_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XWriteBitmapFile(
#else
int XWriteBitmapFile(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* filename */,
    Pixmap c            /* bitmap */,
    unsigned int d      /* width */,
    unsigned int e      /* height */,
    int f               /* x_hot */,
    int g               /* y_hot */                  
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XSupportsLocale(
#else
Bool XSupportsLocale(
#endif
#if NeedFunctionPrototypes
    void *a
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XSetLocaleModifiers(
#else
char *XSetLocaleModifiers(
#endif
#if NeedFunctionPrototypes
    _Xconst char* a     /* modifier_list */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XOM XOpenOM(
#else
XOM XOpenOM(
#endif
#if NeedFunctionPrototypes
    Display* a                  /* display */,
    struct _XrmHashBucketRec* b /* rdb */,
    _Xconst char* c             /* res_name */,
    _Xconst char* d             /* res_class */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XCloseOM(
#else
Status XCloseOM(
#endif
#if NeedFunctionPrototypes
    XOM a                       /* om */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XSetOMValues(
#else
char *XSetOMValues(
#endif
#if NeedVarargsPrototypes
    XOM a                       /* om */,
    ...
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XGetOMValues(
#else
char *XGetOMValues(
#endif
#if NeedVarargsPrototypes
    XOM a                       /* om */,
    ...
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Display *XDisplayOfOM(
#else
Display *XDisplayOfOM(
#endif
#if NeedFunctionPrototypes
    XOM a                       /* om */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XLocaleOfOM(
#else
char *XLocaleOfOM(
#endif
#if NeedFunctionPrototypes
    XOM a                       /* om */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XOC XCreateOC(
#else
XOC XCreateOC(
#endif
#if NeedVarargsPrototypes
    XOM a                       /* om */,
    ...
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XDestroyOC(
#else
void XDestroyOC(
#endif
#if NeedFunctionPrototypes
    XOC a                       /* oc */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XOM XOMOfOC(
#else
XOM XOMOfOC(
#endif
#if NeedFunctionPrototypes
    XOC a                       /* oc */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XSetOCValues(
#else
char *XSetOCValues(
#endif
#if NeedVarargsPrototypes
    XOC a                       /* oc */,
    ...
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XGetOCValues(
#else
char *XGetOCValues(
#endif
#if NeedVarargsPrototypes
    XOC a                       /* oc */,
    ...
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFontSet XCreateFontSet(
#else
XFontSet XCreateFontSet(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    _Xconst char* b     /* base_font_name_list */,
    char***     c       /* missing_charset_list */,
    int* d              /* missing_charset_count */,
    char** e            /* def_string */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XFreeFontSet(
#else
void XFreeFontSet(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XFontSet b          /* font_set */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XFontsOfFontSet(
#else
int XFontsOfFontSet(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */,
    XFontStruct*** b    /* font_struct_list */,
    char***     c       /* font_name_list */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XBaseFontNameListOfFontSet(
#else
char *XBaseFontNameListOfFontSet(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XLocaleOfFontSet(
#else
char *XLocaleOfFontSet(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XContextDependentDrawing(
#else
Bool XContextDependentDrawing(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XDirectionalDependentDrawing(
#else
Bool XDirectionalDependentDrawing(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XContextualDrawing(
#else
Bool XContextualDrawing(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XFontSetExtents *XExtentsOfFontSet(
#else
XFontSetExtents *XExtentsOfFontSet(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XmbTextEscapement(
#else
int XmbTextEscapement(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */,
    _Xconst char* b     /* text */,
    int c               /* bytes_text */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XwcTextEscapement(
#else
int XwcTextEscapement(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */,
    _Xconst wchar_t* b  /* text */,
    int c               /* num_wchars */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XmbTextExtents(
#else
int XmbTextExtents(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */,
    _Xconst char* b     /* text */,
    int c               /* bytes_text */,
    XRectangle* d       /* overall_ink_return */,
    XRectangle* e       /* overall_logical_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XwcTextExtents(
#else
int XwcTextExtents(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */,
    _Xconst wchar_t* b  /* text */,
    int c               /* num_wchars */,
    XRectangle* d       /* overall_ink_return */,
    XRectangle* e       /* overall_logical_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XmbTextPerCharExtents(
#else
Status XmbTextPerCharExtents(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */,
    _Xconst char* b     /* text */,
    int c               /* bytes_text */,
    XRectangle* d       /* ink_extents_buffer */,
    XRectangle* e       /* logical_extents_buffer */,
    int f               /* buffer_size */,
    int* g              /* num_chars */,
    XRectangle* h       /* overall_ink_return */,
    XRectangle* i       /* overall_logical_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XwcTextPerCharExtents(
#else
Status XwcTextPerCharExtents(
#endif
#if NeedFunctionPrototypes
    XFontSet a          /* font_set */,
    _Xconst wchar_t* b  /* text */,
    int c               /* num_wchars */,
    XRectangle* d       /* ink_extents_buffer */,
    XRectangle* e       /* logical_extents_buffer */,
    int f               /* buffer_size */,
    int* g              /* num_chars */,
    XRectangle* h       /* overall_ink_return */,
    XRectangle* i       /* overall_logical_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XmbDrawText(
#else
void XmbDrawText(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    int x               /* x */,
    int y               /* y */,
    XmbTextItem* z      /* text_items */,
    int w               /* nitems */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XwcDrawText(
#else
void XwcDrawText(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    GC e                        /* gc */,
    int x               /* x */,
    int y               /* y */,
    XwcTextItem* z      /* text_items */,
    int w               /* nitems */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XmbDrawString(
#else
void XmbDrawString(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    XFontSet f          /* font_set */,
    GC e                        /* gc */,
    int x               /* x */,
    int y               /* y */,
    _Xconst char* z     /* text */,
    int w               /* bytes_text */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XwcDrawString(
#else
void XwcDrawString(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    XFontSet f          /* font_set */,
    GC e                        /* gc */,
    int x               /* x */,
    int y               /* y */,
    _Xconst wchar_t* z  /* text */,
    int w               /* num_wchars */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XmbDrawImageString(
#else
void XmbDrawImageString(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    XFontSet f          /* font_set */,
    GC e                        /* gc */,
    int x               /* x */,
    int y               /* y */,
    _Xconst char* z     /* text */,
    int w               /* bytes_text */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XwcDrawImageString(
#else
void XwcDrawImageString(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Drawable d          /* d */,
    XFontSet f          /* font_set */,
    GC e                        /* gc */,
    int x               /* x */,
    int y               /* y */,
    _Xconst wchar_t* z  /* text */,
    int w               /* num_wchars */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XIM XOpenIM(
#else
XIM XOpenIM(
#endif
#if NeedFunctionPrototypes
    Display* a                  /* dpy */,
    struct _XrmHashBucketRec* b /* rdb */,
    char* c                     /* res_name */,
    char* d                     /* res_class */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XCloseIM(
#else
Status XCloseIM(
#endif
#if NeedFunctionPrototypes
    XIM a /* im */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XGetIMValues(
#else
char *XGetIMValues(
#endif
#if NeedVarargsPrototypes
    XIM a /* im */, ...
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Display *XDisplayOfIM(
#else
Display *XDisplayOfIM(
#endif
#if NeedFunctionPrototypes
    XIM a /* im */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XLocaleOfIM(
#else
char *XLocaleOfIM(
#endif
#if NeedFunctionPrototypes
    XIM a /* im*/
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XIC XCreateIC(
#else
XIC XCreateIC(
#endif
#if NeedVarargsPrototypes
    XIM a /* im */, ...
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XDestroyIC(
#else
void XDestroyIC(
#endif
#if NeedFunctionPrototypes
    XIC a /* ic */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XSetICFocus(
#else
void XSetICFocus(
#endif
#if NeedFunctionPrototypes
    XIC a /* ic */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XUnsetICFocus(
#else
void XUnsetICFocus(
#endif
#if NeedFunctionPrototypes
    XIC a /* ic */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport wchar_t *XwcResetIC(
#else
wchar_t *XwcResetIC(
#endif
#if NeedFunctionPrototypes
    XIC a /* ic */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XmbResetIC(
#else
char *XmbResetIC(
#endif
#if NeedFunctionPrototypes
    XIC a /* ic */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XSetICValues(
#else
char *XSetICValues(
#endif
#if NeedVarargsPrototypes
    XIC a /* ic */, ...
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XGetICValues(
#else
char *XGetICValues(
#endif
#if NeedVarargsPrototypes
    XIC a /* ic */, ...
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XIM XIMOfIC(
#else
XIM XIMOfIC(
#endif
#if NeedFunctionPrototypes
    XIC a /* ic */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XFilterEvent(
#else
Bool XFilterEvent(
#endif
#if NeedFunctionPrototypes
    XEvent*     a /* event */,
    Window b    /* window */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XmbLookupString(
#else
int XmbLookupString(
#endif
#if NeedFunctionPrototypes
    XIC a                       /* ic */,
    XKeyPressedEvent* b /* event */,
    char* c             /* buffer_return */,
    int d               /* bytes_buffer */,
    KeySym*     e       /* keysym_return */,
    Status*     f       /* status_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XwcLookupString(
#else
int XwcLookupString(
#endif
#if NeedFunctionPrototypes
    XIC a                       /* ic */,
    XKeyPressedEvent* b /* event */,
    wchar_t* c          /* buffer_return */,
    int d               /* wchars_buffer */,
    KeySym*     e       /* keysym_return */,
    Status*     f       /* status_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XVaNestedList XVaCreateNestedList(
#else
XVaNestedList XVaCreateNestedList(
#endif
#if NeedVarargsPrototypes
    int a /*unused*/, ...
#endif
){}

/* internal connections for IMs */

#if defined(_VISUALC_)
extern MagickExport Bool XRegisterIMInstantiateCallback(
#else
Bool XRegisterIMInstantiateCallback(
#endif
#if NeedFunctionPrototypes
    Display* a                  /* dpy */,
    struct _XrmHashBucketRec* b /* rdb */,
    char* c                     /* res_name */,
    char* d                     /* res_class */,
    XIMProc     e               /* callback */,
    XPointer* f                 /* client_data */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XUnregisterIMInstantiateCallback(
#else
Bool XUnregisterIMInstantiateCallback(
#endif
#if NeedFunctionPrototypes
    Display* a                  /* dpy */,
    struct _XrmHashBucketRec* x /* rdb */,
    char* b                     /* res_name */,
    char* c                     /* res_class */,
    XIMProc     d               /* callback */,
    XPointer* e                 /* client_data */
#endif
){}

typedef void (*XConnectionWatchProc)(
#if NeedFunctionPrototypes
    Display* a                  /* dpy */,
    XPointer b                  /* client_data */,
    int c                       /* fd */,
    Bool d                      /* opening */,   /* open or close flag */
    XPointer* e                 /* watch_data */ /* open sets, close uses */
#endif
);
    

#if defined(_VISUALC_)
extern MagickExport Status XInternalConnectionNumbers(
#else
Status XInternalConnectionNumbers(
#endif
#if NeedFunctionPrototypes
    Display* a                  /* dpy */,
    int** b                     /* fd_return */,
    int* c                      /* count_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XProcessInternalConnection(
#else
void XProcessInternalConnection(
#endif
#if NeedFunctionPrototypes
    Display* a                  /* dpy */,
    int b                       /* fd */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XAddConnectionWatch(
#else
Status XAddConnectionWatch(
#endif
#if NeedFunctionPrototypes
    Display* a                  /* dpy */,
    XConnectionWatchProc b      /* callback */,
    XPointer c                  /* client_data */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XRemoveConnectionWatch(
#else
void XRemoveConnectionWatch(
#endif
#if NeedFunctionPrototypes
    Display* a                  /* dpy */,
    XConnectionWatchProc b      /* callback */,
    XPointer c                  /* client_data */
#endif
){}

/* * * * * * */


/* 
 * Bitmask returned by XParseGeometry().  Each bit tells if the corresponding
 * value (x, y, width, height) was found in the parsed string.
 */
#define NoValue         0x0000
#define XValue          0x0001
#define YValue          0x0002
#define WidthValue      0x0004
#define HeightValue     0x0008
#define AllValues       0x000F
#define XNegative       0x0010
#define YNegative       0x0020

/*
 * new version containing base_width, base_height, and win_gravity fields;
 * used with WM_NORMAL_HINTS.
 */
typedef struct {
        long flags;     /* marks which fields in this structure are defined */
        int x, y;               /* obsolete for new window mgrs, but clients */
        int width, height;      /* should set so old wm's don't mess up */
        int min_width, min_height;
        int max_width, max_height;
        int width_inc, height_inc;
        struct {
                int x;  /* numerator */
                int y;  /* denominator */
        } min_aspect, max_aspect;
        int base_width, base_height;            /* added by ICCCM version 1 */
        int win_gravity;                        /* added by ICCCM version 1 */
} XSizeHints;

/*
 * The next block of definitions are for window manager properties that
 * clients and applications use for communication.
 */

/* flags argument in size hints */
#define USPosition      (1L << 0) /* user specified x, y */
#define USSize          (1L << 1) /* user specified width, height */

#define PPosition       (1L << 2) /* program specified position */
#define PSize           (1L << 3) /* program specified size */
#define PMinSize        (1L << 4) /* program specified minimum size */
#define PMaxSize        (1L << 5) /* program specified maximum size */
#define PResizeInc      (1L << 6) /* program specified resize increments */
#define PAspect         (1L << 7) /* program specified min and max aspect ratios */
#define PBaseSize       (1L << 8) /* program specified base for incrementing */
#define PWinGravity     (1L << 9) /* program specified window gravity */

/* obsolete */
#define PAllHints (PPosition|PSize|PMinSize|PMaxSize|PResizeInc|PAspect)



typedef struct {
        long flags;     /* marks which fields in this structure are defined */
        Bool input;     /* does this application rely on the window manager to
                        get keyboard input? */
        int initial_state;      /* see below */
        Pixmap icon_pixmap;     /* pixmap to be used as icon */
        Window icon_window;     /* window to be used as icon */
        int icon_x, icon_y;     /* initial position of icon */
        Pixmap icon_mask;       /* icon mask bitmap */
        XID window_group;       /* id of related window group */
        /* this structure may be extended in the future */
} XWMHints;

/* definition for flags of XWMHints */

#define InputHint               (1L << 0)
#define StateHint               (1L << 1)
#define IconPixmapHint          (1L << 2)
#define IconWindowHint          (1L << 3)
#define IconPositionHint        (1L << 4)
#define IconMaskHint            (1L << 5)
#define WindowGroupHint         (1L << 6)
#define AllHints (InputHint|StateHint|IconPixmapHint|IconWindowHint| \
IconPositionHint|IconMaskHint|WindowGroupHint)
#define XUrgencyHint            (1L << 8)

/* definitions for initial window state */
#define WithdrawnState 0        /* for windows that are not mapped */
#define NormalState 1   /* most applications want to start this way */
#define IconicState 3   /* application wants to start as an icon */

/*
 * Obsolete states no longer defined by ICCCM
 */
#define DontCareState 0 /* don't know or care */
#define ZoomState 2     /* application wants to start zoomed */
#define InactiveState 4 /* application believes it is seldom used; */
                        /* some wm's may put it on inactive menu */


/*
 * new structure for manipulating TEXT properties; used with WM_NAME, 
 * WM_ICON_NAME, WM_CLIENT_MACHINE, and WM_COMMAND.
 */
typedef struct {
    unsigned char *value;               /* same as Property routines */
    Atom encoding;                      /* prop type */
    int format;                         /* prop data format: 8, 16, or 32 */
    unsigned long nitems;               /* number of data items in value */
} XTextProperty;

#define XNoMemory -1
#define XLocaleNotSupported -2
#define XConverterNotFound -3

typedef enum {
    XStringStyle,               /* STRING */
    XCompoundTextStyle,         /* COMPOUND_TEXT */
    XTextStyle,                 /* text in owner's encoding (current locale)*/
    XStdICCTextStyle            /* STRING, else COMPOUND_TEXT */
} XICCEncodingStyle;

typedef struct {
        int min_width, min_height;
        int max_width, max_height;
        int width_inc, height_inc;
} XIconSize;

typedef struct {
        char *res_name;
        char *res_class;
} XClassHint;

/*
 * These macros are used to give some sugar to the image routines so that
 * naive people are more comfortable with them.
 */
#define XDestroyImage(ximage) \
        ((*((ximage)->f.destroy_image))((ximage)))
#define XGetPixel(ximage, x, y) \
        ((*((ximage)->f.get_pixel))((ximage), (x), (y)))
#define XPutPixel(ximage, x, y, pixel) \
        ((*((ximage)->f.put_pixel))((ximage), (x), (y), (pixel)))
#define XSubImage(ximage, x, y, width, height)  \
        ((*((ximage)->f.sub_image))((ximage), (x), (y), (width), (height)))
#define XAddPixel(ximage, value) \
        ((*((ximage)->f.add_pixel))((ximage), (value)))

/*
 * Compose sequence status structure, used in calling XLookupString.
 */
typedef struct _XComposeStatus {
    XPointer compose_ptr;       /* state table pointer */
    int chars_matched;          /* match state */
} XComposeStatus;

/*
 * Keysym macros, used on Keysyms to test for classes of symbols
 */
#define IsKeypadKey(keysym) \
  (((KeySym)(keysym) >= XK_KP_Space) && ((KeySym)(keysym) <= XK_KP_Equal))

#define IsPrivateKeypadKey(keysym) \
  (((KeySym)(keysym) >= 0x11000000) && ((KeySym)(keysym) <= 0x1100FFFF))

#define IsCursorKey(keysym) \
  (((KeySym)(keysym) >= XK_Home)     && ((KeySym)(keysym) <  XK_Select))

#define IsPFKey(keysym) \
  (((KeySym)(keysym) >= XK_KP_F1)     && ((KeySym)(keysym) <= XK_KP_F4))

#define IsFunctionKey(keysym) \
  (((KeySym)(keysym) >= XK_F1)       && ((KeySym)(keysym) <= XK_F35))

#define IsMiscFunctionKey(keysym) \
  (((KeySym)(keysym) >= XK_Select)   && ((KeySym)(keysym) <= XK_Break))

#define IsModifierKey(keysym) \
  ((((KeySym)(keysym) >= XK_Shift_L) && ((KeySym)(keysym) <= XK_Hyper_R)) \
   || ((KeySym)(keysym) == XK_Mode_switch) \
   || ((KeySym)(keysym) == XK_Num_Lock))
/*
 * opaque reference to Region data type 
 */
typedef struct _XRegion *Region; 

/* Return values from XRectInRegion() */
 
#define RectangleOut 0
#define RectangleIn  1
#define RectanglePart 2
 

/*
 * Information used by the visual utility routines to find desired visual
 * type from the many visuals a display may support.
 */

typedef struct {
  Visual *visual;
  VisualID visualid;
  int screen;
  int depth;
#if defined(__cplusplus) || defined(c_plusplus)
  int c_class;                                  /* C++ */
#else
  int class;
#endif
  unsigned long red_mask;
  unsigned long green_mask;
  unsigned long blue_mask;
  int colormap_size;
  int bits_per_rgb;
} XVisualInfo;

#define VisualNoMask            0x0
#define VisualIDMask            0x1
#define VisualScreenMask        0x2
#define VisualDepthMask         0x4
#define VisualClassMask         0x8
#define VisualRedMaskMask       0x10
#define VisualGreenMaskMask     0x20
#define VisualBlueMaskMask      0x40
#define VisualColormapSizeMask  0x80
#define VisualBitsPerRGBMask    0x100
#define VisualAllMask           0x1FF

/*
 * This defines a window manager property that clients may use to
 * share standard color maps of type RGB_COLOR_MAP:
 */
typedef struct {
        Colormap colormap;
        unsigned long red_max;
        unsigned long red_mult;
        unsigned long green_max;
        unsigned long green_mult;
        unsigned long blue_max;
        unsigned long blue_mult;
        unsigned long base_pixel;
        VisualID visualid;              /* added by ICCCM version 1 */
        XID killid;                     /* added by ICCCM version 1 */
} XStandardColormap;

#define ReleaseByFreeingColormap ((XID) 1L)  /* for killid field above */


/*
 * return codes for XReadBitmapFile and XWriteBitmapFile
 */
#define BitmapSuccess           0
#define BitmapOpenFailed        1
#define BitmapFileInvalid       2
#define BitmapNoMemory          3

/****************************************************************
 *
 * Context Management
 *
 ****************************************************************/


/* Associative lookup table return codes */

#define XCSUCCESS 0     /* No error. */
#define XCNOMEM   1    /* Out of memory */
#define XCNOENT   2    /* No entry in table */

typedef int XContext;

#define XUniqueContext()       ((XContext) XrmUniqueQuark())
#define XStringToContext(string)   ((XContext) XrmStringToQuark(string))

/* The following declarations are alphabetized. */

#if defined(_VISUALC_)
extern MagickExport XClassHint *XAllocClassHint (
#else
XClassHint *XAllocClassHint (
#endif
#if NeedFunctionPrototypes
    void
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XIconSize *XAllocIconSize (
#else
XIconSize *XAllocIconSize (
#endif
#if NeedFunctionPrototypes
    void
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSizeHints *XAllocSizeHints (
#else
XSizeHints *XAllocSizeHints (
#endif
#if NeedFunctionPrototypes
    void
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XStandardColormap *XAllocStandardColormap (
#else
XStandardColormap *XAllocStandardColormap (
#endif
#if NeedFunctionPrototypes
    void
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XWMHints *XAllocWMHints (
#else
XWMHints *XAllocWMHints (
#endif
#if NeedFunctionPrototypes
    void
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XClipBox(
#else
XClipBox(
#endif
#if NeedFunctionPrototypes
    Region a            /* r */,
    XRectangle* b       /* rect_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Region XCreateRegion(
#else
Region XCreateRegion(
#endif
#if NeedFunctionPrototypes
    void *a
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XDefaultString(
#else
char *XDefaultString(
#endif
#if NeedFunctionPrototypes
    void *a
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XDeleteContext(
#else
int XDeleteContext(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XID  b              /* rid */,
    XContext c          /* context */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XDestroyRegion(
#else
XDestroyRegion(
#endif
#if NeedFunctionPrototypes
    Region a            /* r */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XEmptyRegion(
#else
XEmptyRegion(
#endif
#if NeedFunctionPrototypes
    Region a            /* r */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XEqualRegion(
#else
XEqualRegion(
#endif
#if NeedFunctionPrototypes
    Region a            /* r1 */,
    Region b            /* r2 */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XFindContext(
#else
int XFindContext(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XID b               /* rid */,
    XContext c          /* context */,
    XPointer* d         /* data_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetClassHint(
#else
Status XGetClassHint(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XClassHint* c       /* class_hints_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetIconSizes(
#else
Status XGetIconSizes(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XIconSize** c       /* size_list_return */,
    int* d              /* count_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetNormalHints(
#else
Status XGetNormalHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XSizeHints* c       /* hints_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetRGBColormaps(
#else
Status XGetRGBColormaps(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XStandardColormap** d /* stdcmap_return */,
    int* e              /* count_return */,
    Atom f              /* property */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetSizeHints(
#else
Status XGetSizeHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XSizeHints* c       /* hints_return */,
    Atom d              /* property */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetStandardColormap(
#else
Status XGetStandardColormap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XStandardColormap* c        /* colormap_return */,
    Atom d              /* property */                      
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetTextProperty(
#else
Status XGetTextProperty(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* window */,
    XTextProperty* c    /* text_prop_return */,
    Atom d              /* property */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XVisualInfo *XGetVisualInfo(
#else
XVisualInfo *XGetVisualInfo(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    long b              /* vinfo_mask */,
    XVisualInfo* c      /* vinfo_template */,
    int* d              /* nitems_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetWMClientMachine(
#else
Status XGetWMClientMachine(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XTextProperty* c    /* text_prop_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XWMHints *XGetWMHints(
#else
XWMHints *XGetWMHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */               
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetWMIconName(
#else
Status XGetWMIconName(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XTextProperty* c    /* text_prop_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetWMName(
#else
Status XGetWMName(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XTextProperty* c    /* text_prop_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetWMNormalHints(
#else
Status XGetWMNormalHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XSizeHints* c       /* hints_return */,
    long* d             /* supplied_return */ 
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetWMSizeHints(
#else
Status XGetWMSizeHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XSizeHints* c       /* hints_return */,
    long* d             /* supplied_return */,
    Atom e              /* property */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XGetZoomHints(
#else
Status XGetZoomHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XSizeHints* c       /* zhints_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XIntersectRegion(
#else
XIntersectRegion(
#endif
#if NeedFunctionPrototypes
    Region a            /* sra */,
    Region b            /* srb */,
    Region c            /* dr_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XConvertCase(
#else
void XConvertCase(
#endif
#if NeedFunctionPrototypes
    KeySym      a       /* sym */,
    KeySym*     b       /* lower */,
    KeySym*     c       /* upper */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XLookupString(
#else
int XLookupString(
#endif
#if NeedFunctionPrototypes
    XKeyEvent* a                /* event_struct */,
    char* b             /* buffer_return */,
    int c               /* bytes_buffer */,
    KeySym*     d       /* keysym_return */,
    XComposeStatus*     e /* status_in_out */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XMatchVisualInfo(
#else
Status XMatchVisualInfo(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen */,
    int c               /* depth */,
    int d               /* class */,
    XVisualInfo* e      /* vinfo_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XOffsetRegion(
#else
XOffsetRegion(
#endif
#if NeedFunctionPrototypes
    Region a            /* r */,
    int b               /* dx */,
    int c               /* dy */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XPointInRegion(
#else
Bool XPointInRegion(
#endif
#if NeedFunctionPrototypes
    Region a            /* r */,
    int b               /* x */,
    int c               /* y */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Region XPolygonRegion(
#else
Region XPolygonRegion(
#endif
#if NeedFunctionPrototypes
    XPoint*     a       /* points */,
    int b               /* n */,
    int c               /* fill_rule */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XRectInRegion(
#else
int XRectInRegion(
#endif
#if NeedFunctionPrototypes
    Region a            /* r */,
    int b               /* x */,
    int c               /* y */,
    unsigned int d      /* width */,
    unsigned int e      /* height */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XSaveContext(
#else
int XSaveContext(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XID b               /* rid */,
    XContext c          /* context */,
    _Xconst char* d     /* data */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetClassHint(
#else
XSetClassHint(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XClassHint* c       /* class_hints */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetIconSizes(
#else
XSetIconSizes(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XIconSize* c                /* size_list */,
    int d               /* count */    
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetNormalHints(
#else
XSetNormalHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XSizeHints* c       /* hints */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XSetRGBColormaps(
#else
void XSetRGBColormaps(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XStandardColormap* c        /* stdcmaps */,
    int d               /* count */,
    Atom e              /* property */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetSizeHints(
#else
XSetSizeHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XSizeHints* c       /* hints */,
    Atom d              /* property */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetStandardProperties(
#else
XSetStandardProperties(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    _Xconst char* c     /* window_name */,
    _Xconst char* d     /* icon_name */,
    Pixmap e            /* icon_pixmap */,
    char** f            /* argv */,
    int g               /* argc */,
    XSizeHints* h       /* hints */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XSetTextProperty(
#else
void XSetTextProperty(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XTextProperty* c    /* text_prop */,
    Atom d              /* property */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XSetWMClientMachine(
#else
void XSetWMClientMachine(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XTextProperty* c    /* text_prop */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetWMHints(
#else
XSetWMHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XWMHints* c         /* wm_hints */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XSetWMIconName(
#else
void XSetWMIconName(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XTextProperty* c    /* text_prop */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XSetWMName(
#else
void XSetWMName(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XTextProperty* c    /* text_prop */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XSetWMNormalHints(
#else
void XSetWMNormalHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XSizeHints* c       /* hints */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XSetWMProperties(
#else
void XSetWMProperties(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XTextProperty* c    /* window_name */,
    XTextProperty* d    /* icon_name */,
    char** e            /* argv */,
    int f               /* argc */,
    XSizeHints* g       /* normal_hints */,
    XWMHints* h         /* wm_hints */,
    XClassHint* i       /* class_hints */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XmbSetWMProperties(
#else
void XmbSetWMProperties(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    _Xconst char* c     /* window_name */,
    _Xconst char* d     /* icon_name */,
    char** e            /* argv */,
    int f               /* argc */,
    XSizeHints* g       /* normal_hints */,
    XWMHints* h         /* wm_hints */,
    XClassHint* i       /* class_hints */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XSetWMSizeHints(
#else
void XSetWMSizeHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XSizeHints* c       /* hints */,
    Atom d              /* property */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetRegion(
#else
XSetRegion(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    GC e                        /* gc */,
    Region x            /* r */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XSetStandardColormap(
#else
void XSetStandardColormap(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XStandardColormap* c        /* colormap */,
    Atom d              /* property */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSetZoomHints(
#else
XSetZoomHints(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    Window b            /* w */,
    XSizeHints* c       /* zhints */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XShrinkRegion(
#else
XShrinkRegion(
#endif
#if NeedFunctionPrototypes
    Region x            /* r */,
    int y               /* dx */,
    int z               /* dy */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XStringListToTextProperty(
#else
Status XStringListToTextProperty(
#endif
#if NeedFunctionPrototypes
    char** a            /* list */,
    int b               /* count */,
    XTextProperty* c    /* text_prop_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XSubtractRegion(
#else
XSubtractRegion(
#endif
#if NeedFunctionPrototypes
    Region x            /* sra */,
    Region y            /* srb */,
    Region z            /* dr_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XmbTextListToTextProperty(
#else
int XmbTextListToTextProperty(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    char** b            /* list */,
    int c               /* count */,
    XICCEncodingStyle d /* style */,
    XTextProperty* e    /* text_prop_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XwcTextListToTextProperty(
#else
int XwcTextListToTextProperty(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    wchar_t** b         /* list */,
    int c               /* count */,
    XICCEncodingStyle d /* style */,
    XTextProperty* e    /* text_prop_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XwcFreeStringList(
#else
void XwcFreeStringList(
#endif
#if NeedFunctionPrototypes
    wchar_t** a         /* list */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XTextPropertyToStringList(
#else
Status XTextPropertyToStringList(
#endif
#if NeedFunctionPrototypes
    XTextProperty* a    /* text_prop */,
    char***     b       /* list_return */,
    int* c              /* count_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XmbTextPropertyToTextList(
#else
int XmbTextPropertyToTextList(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XTextProperty* b    /* text_prop */,
    char***     c       /* list_return */,
    int* d              /* count_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XwcTextPropertyToTextList(
#else
int XwcTextPropertyToTextList(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XTextProperty* b    /* text_prop */,
    wchar_t*** c                /* list_return */,
    int* d              /* count_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUnionRectWithRegion(
#else
XUnionRectWithRegion(
#endif
#if NeedFunctionPrototypes
    XRectangle* a       /* rectangle */,
    Region b            /* src_region */,
    Region c            /* dest_region_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XUnionRegion(
#else
XUnionRegion(
#endif
#if NeedFunctionPrototypes
    Region c            /* sra */,
    Region d            /* srb */,
    Region e            /* dr_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport int XWMGeometry(
#else
int XWMGeometry(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    int b               /* screen_number */,
    _Xconst char* c     /* user_geometry */,
    _Xconst char* d     /* default_geometry */,
    unsigned int e      /* border_width */,
    XSizeHints* f       /* hints */,
    int* g              /* x_return */,
    int* h              /* y_return */,
    int* i              /* width_return */,
    int* j              /* height_return */,
    int* k              /* gravity_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XXorRegion(
#else
XXorRegion(
#endif
#if NeedFunctionPrototypes
    Region a            /* sra */,
    Region b            /* srb */,
    Region c            /* dr_return */
#endif
){}

/* * * * * * */

/****************************************************************
 ****************************************************************
 ***                                                          ***
 ***                                                          ***
 ***          X Resource Manager Intrinsics                   ***
 ***                                                          ***
 ***                                                          ***
 ****************************************************************
 ****************************************************************/

_XFUNCPROTOBEGIN

/****************************************************************
 *
 * Memory Management
 *
 ****************************************************************/

#if defined(_VISUALC_)
extern MagickExport char *Xpermalloc(
#else
char *Xpermalloc(
#endif
#if NeedFunctionPrototypes
    unsigned int a      /* size */
#endif
){}

/****************************************************************
 *
 * Quark Management
 *
 ****************************************************************/

typedef int     XrmQuark, *XrmQuarkList;
#define NULLQUARK ((XrmQuark) 0)

typedef char *XrmString;
#define NULLSTRING ((XrmString) 0)

/* find quark for string, create new quark if none already exists */
#if defined(_VISUALC_)
extern MagickExport XrmQuark XrmStringToQuark(
#else
XrmQuark XrmStringToQuark(
#endif
#if NeedFunctionPrototypes
    _Xconst char* a     /* string */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XrmQuark XrmPermStringToQuark(
#else
XrmQuark XrmPermStringToQuark(
#endif
#if NeedFunctionPrototypes
    _Xconst char* a     /* string */
#endif
){}

/* find string for quark */
#if defined(_VISUALC_)
extern MagickExport XrmString XrmQuarkToString(
#else
XrmString XrmQuarkToString(
#endif
#if NeedFunctionPrototypes
    XrmQuark a          /* quark */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XrmQuark XrmUniqueQuark(
#else
XrmQuark XrmUniqueQuark(
#endif
#if NeedFunctionPrototypes
    void *a
#endif
){}

#define XrmStringsEqual(a1, a2) (strcmp(a1, a2) == 0)


/****************************************************************
 *
 * Conversion of Strings to Lists
 *
 ****************************************************************/

typedef enum {XrmBindTightly, XrmBindLoosely} XrmBinding, *XrmBindingList;

#if defined(_VISUALC_)
extern MagickExport void XrmStringToQuarkList(
#else
void XrmStringToQuarkList(
#endif
#if NeedFunctionPrototypes
    _Xconst char* a     /* string */,
    XrmQuarkList b      /* quarks_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XrmStringToBindingQuarkList(
#else
void XrmStringToBindingQuarkList(
#endif
#if NeedFunctionPrototypes
    _Xconst char* a     /* string */,
    XrmBindingList b    /* bindings_return */,
    XrmQuarkList c      /* quarks_return */
#endif
){}

/****************************************************************
 *
 * Name and Class lists.
 *
 ****************************************************************/

typedef XrmQuark     XrmName;
typedef XrmQuarkList XrmNameList;
#define XrmNameToString(name)           XrmQuarkToString(name)
#define XrmStringToName(string)         XrmStringToQuark(string)
#define XrmStringToNameList(str, name)  XrmStringToQuarkList(str, name)

typedef XrmQuark     XrmClass;
typedef XrmQuarkList XrmClassList;
#define XrmClassToString(c_class)       XrmQuarkToString(c_class)
#define XrmStringToClass(c_class)       XrmStringToQuark(c_class)
#define XrmStringToClassList(str,c_class) XrmStringToQuarkList(str, c_class)



/****************************************************************
 *
 * Resource Representation Types and Values
 *
 ****************************************************************/

typedef XrmQuark     XrmRepresentation;
#define XrmStringToRepresentation(string)   XrmStringToQuark(string)
#define XrmRepresentationToString(type)   XrmQuarkToString(type)

typedef struct {
    unsigned int    size;
    XPointer        addr;
} XrmValue, *XrmValuePtr;


/****************************************************************
 *
 * Resource Manager Functions
 *
 ****************************************************************/

typedef struct _XrmHashBucketRec *XrmHashBucket;
typedef XrmHashBucket *XrmHashTable;
typedef XrmHashTable XrmSearchList[];
typedef struct _XrmHashBucketRec *XrmDatabase;


#if defined(_VISUALC_)
extern MagickExport void XrmDestroyDatabase(
#else
void XrmDestroyDatabase(
#endif
#if NeedFunctionPrototypes
    XrmDatabase a               /* database */    
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XrmQPutResource(
#else
void XrmQPutResource(
#endif
#if NeedFunctionPrototypes
    XrmDatabase* a      /* database */,
    XrmBindingList b    /* bindings */,
    XrmQuarkList c      /* quarks */,
    XrmRepresentation d /* type */,
    XrmValue* e         /* value */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XrmPutResource(
#else
void XrmPutResource(
#endif
#if NeedFunctionPrototypes
    XrmDatabase* a      /* database */,
    _Xconst char* b     /* specifier */,
    _Xconst char* c     /* type */,
    XrmValue* d         /* value */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XrmQPutStringResource(
#else
void XrmQPutStringResource(
#endif
#if NeedFunctionPrototypes
    XrmDatabase* a      /* database */,
    XrmBindingList b     /* bindings */,
    XrmQuarkList c      /* quarks */,
    _Xconst char* d     /* value */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XrmPutStringResource(
#else
void XrmPutStringResource(
#endif
#if NeedFunctionPrototypes
    XrmDatabase* a      /* database */,
    _Xconst char* b     /* specifier */,
    _Xconst char* c     /* value */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XrmPutLineResource(
#else
void XrmPutLineResource(
#endif
#if NeedFunctionPrototypes
    XrmDatabase* a      /* database */,
    _Xconst char* b     /* line */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XrmQGetResource(
#else
XrmQGetResource(
#endif
#if NeedFunctionPrototypes
    XrmDatabase a       /* database */,
    XrmNameList b       /* quark_name */,
    XrmClassList c      /* quark_class */,
    XrmRepresentation* d        /* quark_type_return */,
    XrmValue* e         /* value_return */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XrmGetResource(
#else
Bool XrmGetResource(
#endif
#if NeedFunctionPrototypes
    XrmDatabase a       /* database */,
    _Xconst char* b     /* str_name */,
    _Xconst char* c     /* str_class */,
    char**      d       /* str_type_return */,
    XrmValue*   e       /* value_return */
#endif
)
{
  return(0);
}

#if defined(_VISUALC_)
extern MagickExport Bool XrmQGetSearchList(
#else
Bool XrmQGetSearchList(
#endif
#if NeedFunctionPrototypes
    XrmDatabase a       /* database */,
    XrmNameList b       /* names */,
    XrmClassList c      /* classes */,
    XrmSearchList d     /* list_return */,
    int e               /* list_length */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Bool XrmQGetSearchResource(
#else
Bool XrmQGetSearchResource(
#endif
#if NeedFunctionPrototypes
    XrmSearchList a     /* list */,
    XrmName     b       /* name */,
    XrmClass c          /* class */,
    XrmRepresentation* d        /* type_return */,
    XrmValue* e         /* value_return */
#endif
){}

/****************************************************************
 *
 * Resource Database Management
 *
 ****************************************************************/

#if defined(_VISUALC_)
extern MagickExport void XrmSetDatabase(
#else
void XrmSetDatabase(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */,
    XrmDatabase b       /* database */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XrmDatabase XrmGetDatabase(
#else
XrmDatabase XrmGetDatabase(
#endif
#if NeedFunctionPrototypes
    Display* a          /* display */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XrmDatabase XrmGetFileDatabase(
#else
XrmDatabase XrmGetFileDatabase(
#endif
#if NeedFunctionPrototypes
    _Xconst char* a     /* filename */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport Status XrmCombineFileDatabase(
#else
Status XrmCombineFileDatabase(
#endif
#if NeedFunctionPrototypes
    _Xconst char* b     /* filename */,
    XrmDatabase* a      /* target */,
    Bool c              /* override */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport XrmDatabase XrmGetStringDatabase(
#else
XrmDatabase XrmGetStringDatabase(
#endif
#if NeedFunctionPrototypes
    _Xconst char* a     /* data */  /*  null terminated string */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XrmPutFileDatabase(
#else
void XrmPutFileDatabase(
#endif
#if NeedFunctionPrototypes
    XrmDatabase a       /* database */,
    _Xconst char* b     /* filename */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XrmMergeDatabases(
#else
void XrmMergeDatabases(
#endif
#if NeedFunctionPrototypes
    XrmDatabase b       /* source_db */,
    XrmDatabase* a      /* target_db */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport void XrmCombineDatabase(
#else
void XrmCombineDatabase(
#endif
#if NeedFunctionPrototypes
    XrmDatabase b       /* source_db */,
    XrmDatabase* a      /* target_db */,
    Bool c              /* override */
#endif
){}

#define XrmEnumAllLevels 0
#define XrmEnumOneLevel  1

#if defined(_VISUALC_)
extern MagickExport Bool XrmEnumerateDatabase(
#else
Bool XrmEnumerateDatabase(
#endif
#if NeedFunctionPrototypes
    XrmDatabase a       /* db */,
    XrmNameList b       /* name_prefix */,
    XrmClassList c      /* class_prefix */,
    int d               /* mode */,
    Bool (* e)(
#if NeedNestedPrototypes
             XrmDatabase* q     /* db */,
             XrmBindingList r   /* bindings */,
             XrmQuarkList s     /* quarks */,
             XrmRepresentation* t /* type */,
             XrmValue* u                /* value */,
             XPointer v         /* closure */
#endif
             )          /* proc */,
    XPointer x          /* closure */
#endif
){}

#if defined(_VISUALC_)
extern MagickExport char *XrmLocaleOfDatabase(
#else
char *XrmLocaleOfDatabase(
#endif
#if NeedFunctionPrototypes
    XrmDatabase a       /* database */
#endif
){}


/****************************************************************
 *
 * Command line option mapping to resource entries
 *
 ****************************************************************/

typedef enum {
    XrmoptionNoArg,     /* Value is specified in OptionDescRec.value        */
    XrmoptionIsArg,     /* Value is the option string itself                */
    XrmoptionStickyArg, /* Value is characters immediately following option */
    XrmoptionSepArg,    /* Value is next argument in argv                   */
    XrmoptionResArg,    /* Resource and value in next argument in argv      */
    XrmoptionSkipArg,   /* Ignore this option and the next argument in argv */
    XrmoptionSkipLine,  /* Ignore this option and the rest of argv          */
    XrmoptionSkipNArgs  /* Ignore this option and the next 
                           OptionDescRes.value arguments in argv */
} XrmOptionKind;

typedef struct {
    char            *option;        /* Option abbreviation in argv          */
    char            *specifier;     /* Resource specifier                   */
    XrmOptionKind   argKind;        /* Which style of option it is          */
    XPointer        value;          /* Value to provide if XrmoptionNoArg   */
} XrmOptionDescRec, *XrmOptionDescList;


#if defined(_VISUALC_)
extern MagickExport void XrmParseCommand(
#else
void XrmParseCommand(
#endif
#if NeedFunctionPrototypes
    XrmDatabase* a      /* database */,
    XrmOptionDescList b /* table */,
    int c               /* table_count */,
    _Xconst char* d     /* name */,
    int* e              /* argc_in_out */,
    char**      f       /* argv_in_out */                    
#endif
){}
