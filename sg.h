// Simple graphics
// Handles graphical/physical events

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <sys/utsname.h>

class Graphics
{
public:
    short unsigned int WINDOW_WIDTH = 800;
    short unsigned int WINDOW_HEIGHT = 600;
    std::string WINDOW_TITLE = "Window";

    short int R_WINDOW_WIDTH = WINDOW_WIDTH;
    short int R_WINDOW_HEIGHT = WINDOW_HEIGHT;

    Display* main_display;
    int main_screen;
    Window main_window;
    bool uname_ok;
    Atom WM_DELETE_WINDOW;
    GC default_gc;
    KeySym keysym;
    int keysym_len;

    int windowX, windowY, mouseX, mouseY;

    bool keydown;

    Graphics(short unsigned int w_width, short unsigned int w_height, std::string w_title)
    {
        WINDOW_WIDTH = w_width;
        WINDOW_HEIGHT = w_height;
        WINDOW_TITLE = w_title;
    }

    unsigned long int _RGB(int r,int g, int b)
    {
        return b + (g<<8) + (r<<16);
    }

    void _DrawLine(int x1, int y1, int x2, int y2, unsigned long rgb)
    {
        XSetForeground(main_display, default_gc, rgb);
        XDrawLine(main_display, main_window, default_gc, x1, y1, x2, y2);
    }

    void _DrawRect(int x, int y, int w, int h, bool fill, unsigned long rgb)
    {
        XSetForeground(main_display, default_gc, rgb);
        if (!fill)
            XDrawRectangle(main_display, main_window, default_gc, x, y, w, h);
        else
            XFillRectangle(main_display, main_window, default_gc, x, y, w, h);
    }

    void _DrawPoly(XPoint *point, size_t psize, unsigned int rgb)
    {
        XSetForeground(main_display, default_gc, rgb);
        XFillPolygon(main_display, main_window, default_gc, point, psize, Convex, CoordModeOrigin);
    }
    void _DrawPolyCheck(XPoint *point, size_t psize, unsigned int rgb)
    {
        XPoint point2[psize];
        for (size_t i = 0; i < psize; ++i)
        {
            point2[i].x = point[i].x;
            if (point[i].x > R_WINDOW_WIDTH)
            {
                point2[i].x = R_WINDOW_WIDTH;
            }
            if (point[i].x < 0)
            {
                point2[i].x = 0;
            }
            point2[i].y = point[i].y;
            if (point[i].y > R_WINDOW_HEIGHT)
            {
                point2[i].y = R_WINDOW_HEIGHT;
            }
            if (point[i].y < 0)
            {
                point2[i].y = 0;
            }
        }
        XSetForeground(main_display, default_gc, rgb);
        XFillPolygon(main_display, main_window, default_gc, point2, psize, Convex, CoordModeOrigin);
    }

    void _InitWindow()
    {
        main_display = XOpenDisplay(NULL);
        if (main_display == NULL)
        {
            fprintf(stderr, "Cannot open display. XOpenDisplay error.\n");
            exit(1);
        }

        int main_screen = DefaultScreen(main_display);
        main_window = XCreateSimpleWindow(main_display, RootWindow(main_display, main_screen), 10, 10, WINDOW_WIDTH, WINDOW_HEIGHT, 1, BlackPixel(main_display, main_screen), BlackPixel(main_display, main_screen));
        XSelectInput(main_display, main_window, ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask);
        XMapWindow(main_display, main_window);

        XStoreName(main_display, main_window, WINDOW_TITLE.c_str());

        WM_DELETE_WINDOW = XInternAtom(main_display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(main_display, main_window, &WM_DELETE_WINDOW, 1);

        uname_ok = false;
        struct utsname sname;
        int ret = uname(&sname);
        if (ret != -1)
        {
            uname_ok = true;
        }

        default_gc = DefaultGC(main_display, main_screen);
    }

    void _WindowLoop(void Render(), void Keyboard(KeySym ks))
    {
        XEvent main_event;
        while (1)
        {
            while(XPending(main_display))
                XNextEvent(main_display, &main_event);

            if (main_event.type == Expose)
            {
                XWindowAttributes wa;

                int x, y;
                Window child;
                XTranslateCoordinates( main_display, main_window, RootWindow(main_display, main_screen), 0, 0, &x, &y, &child );

                XGetWindowAttributes(main_display, main_window, &wa);

                //x - wa.x, y - wa.y
                windowX = x - wa.x;
                windowY = y - wa.y;

                R_WINDOW_WIDTH = wa.width;
                R_WINDOW_HEIGHT = wa.height;

                if (uname_ok)
                {
                    XClearWindow(main_display, main_window);
                    Render();
                }
            }

            // Keyboard
            if (main_event.type == KeyPress)
            {
                XAutoRepeatOff(main_display);
                keydown = true;
                char buf[128] = {0};
                keysym_len = XLookupString(&main_event.xkey, buf, sizeof buf, &keysym, NULL);
                Keyboard(keysym);
                if (keysym == XK_Escape)
                {
                    XAutoRepeatOn(main_display);
                    break;
                }
                main_event.type = Expose;
            }
            if (main_event.type == KeyRelease)
            {
                XAutoRepeatOff(main_display);
                keydown = false;
                char buf[128] = {0};
                keysym_len = XLookupString(&main_event.xkey, buf, sizeof buf, &keysym, NULL);
                Keyboard(keysym);
                main_event.type = Expose;
            }

            XQueryPointer(main_display, RootWindow(main_display, main_screen),
                        &main_event.xbutton.root, &main_event.xbutton.window,
                        &main_event.xbutton.x_root, &main_event.xbutton.y_root,
                        &main_event.xbutton.x, &main_event.xbutton.y,
                        &main_event.xbutton.state
            );
            //printf("Mouse Coordinates: %d %d\n", main_event.xbutton.x, main_event.xbutton.y);
            mouseX = main_event.xbutton.x - windowX;
            mouseY = main_event.xbutton.y - windowY;

            if ((main_event.type == ClientMessage) && (static_cast<unsigned int>(main_event.xclient.data.l[0]) == WM_DELETE_WINDOW))
            {
                break;
            }

            XFlush(main_display);
        }
        XAutoRepeatOn(main_display);
    }

    void _CloseWindow()
    {
        XAutoRepeatOn(main_display);
        XDestroyWindow(main_display, main_window);
        XCloseDisplay(main_display);
    }

};
