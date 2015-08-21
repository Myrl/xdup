/* Compile with -lrt and -lxcb */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#include <xcb/xcb.h>

struct window_size
{
    uint16_t width;
    uint16_t height;
};

int get_window_size(xcb_connection_t *connection, 
                    int window_id, 
                    struct window_size *dest)
{
    xcb_get_geometry_cookie_t cookie;
    cookie = xcb_get_geometry(connection, window_id);

    xcb_get_geometry_reply_t *reply;
    reply = xcb_get_geometry_reply(connection, // Get window
                                   cookie,     // geometry to get 
                                   NULL);      // width and height.
   
    dest->width  = reply->width;
    dest->height = reply->height;

    free(reply);
    
    return 0;
}

int setup_timer()
{
    struct sigevent sevp;
    sevp.sigev_notify = SIGEV_NONE;

    struct itimerspec its = {{0, 16666666}, //60 Hz.
                             {0, 16666666}};
    int timerfd;
    if ((timerfd = timerfd_create(CLOCK_REALTIME, 0)) > 0)
        timerfd_settime(timerfd, 0, &its, NULL);

    return timerfd;
}


int main (int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s window-id\n", argv[0]);
        exit(1);
    }

    xcb_connection_t  *connection = xcb_connect(NULL, NULL);
    const xcb_setup_t *setup      = xcb_get_setup(connection);
    xcb_screen_t      *screen     = xcb_setup_roots_iterator(setup).data;

    xcb_drawable_t window = xcb_generate_id(connection);
    xcb_drawable_t source = strtol(argv[1], NULL, 0);

    struct window_size size;
    get_window_size(connection, source, &size);

    xcb_create_window(connection,
                      XCB_COPY_FROM_PARENT,
                      window,
                      screen->root,
                      0, 0,
                      size.width, size.height,
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual,
                      0, NULL);

    // Create dummy graphics context.
    xcb_gcontext_t graphics = xcb_generate_id(connection);
    xcb_create_gc(connection, graphics, screen->root, 0, NULL);

    xcb_map_window(connection, window);
    xcb_flush(connection);

    int timerfd = setup_timer();

    while (1) {
        int64_t _;                    // Dummy buffer
        read(timerfd, &_, sizeof(_)); // Block until timer expiry

        get_window_size(connection, window, &size);

        xcb_copy_area(connection,
                source,
                window,
                graphics,
                0, 0,
                0, 0,
                size.width, size.height);

        xcb_flush(connection);
    }

    return 0;
}
