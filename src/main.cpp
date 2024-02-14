#include <iostream>

extern "C" {
#include <xcb/xcb.h>
#include <xcb/shape.h>

#include <unistd.h>
}

const uint16_t GMASK = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;

struct winInfo {
    uint16_t width;
    uint16_t height;
    int8_t maxzoom;

    winInfo():width(148), height(148), maxzoom(4) {} /* Default */

    winInfo(uint16_t width, uint16_t height, int8_t maxzoom) {
        this->width = width;
        this->height = height;
        this->maxzoom = maxzoom;
    }
};

xcb_window_t create_new_window(xcb_connection_t *conn, xcb_screen_t *screen, winInfo &lens) {
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
    const uint32_t mask_values[] = {
        screen->white_pixel,
        1
    };

    xcb_window_t window = xcb_generate_id(conn);
    xcb_query_pointer_reply_t *query_reply = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, screen->root), 0); 

    xcb_create_window(
        conn,
        screen->root_depth,
        window,
        screen->root,
        query_reply->root_x - lens.width/2,
        query_reply->root_y - lens.width/2,
        lens.width,
        lens.height,
        0, 
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        mask,
        mask_values
    );

    return window;
}

void grab_pointer(xcb_connection_t *conn, xcb_screen_t *screen) {
    xcb_grab_pointer_cookie_t grab_cookie = xcb_grab_pointer(
        conn,
        false,
        screen->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_POINTER_MOTION,
        XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC,
        XCB_NONE,
        XCB_NONE,
        XCB_CURRENT_TIME 
    );

    xcb_grab_pointer_reply_t *grab_reply = xcb_grab_pointer_reply(conn, grab_cookie, nullptr);

    if(grab_reply) {
        if(grab_reply->status == XCB_GRAB_STATUS_SUCCESS) {
                std::cout << "Grab Status: OK\n";
        } else {
            std::cerr << "Grab Status: FAILED\n";
            exit(EXIT_FAILURE);
        }
        delete grab_reply;
    } else {
        std::cerr << "xcb_grab returned NULL\n";
        exit(EXIT_FAILURE);
    }
}

void listen_for_pointer(xcb_connection_t *conn, xcb_window_t win_id) {
    bool run = true;
    xcb_generic_event_t *event;
    while(run) {
        event = xcb_wait_for_event(conn);

        switch(event->response_type & ~0x80) {
            case XCB_BUTTON_PRESS: {
                xcb_button_press_event_t *e = (xcb_button_press_event_t*)event;
                if(e->detail == XCB_BUTTON_INDEX_1) {
                    std::cout << "Killing window...\n";
                    xcb_destroy_window(conn, win_id);
                    run = false;
                }
                break;
            }
            case XCB_MOTION_NOTIFY: {
                xcb_motion_notify_event_t *e = (xcb_motion_notify_event_t*)event;
                uint32_t coords[] = {static_cast<uint32_t>(e->root_x) - 74, static_cast<uint32_t>(e->root_y) - 74};

                xcb_configure_window(
                    conn,
                    win_id,
                    GMASK,
                    coords
                );
                xcb_flush(conn);
                break; 
            }
            default:
                std::cout << "Unknown event: " << (event->response_type & ~0x80) << "\n";
                break;
        }
        delete event;
    }
    xcb_ungrab_pointer(conn, XCB_CURRENT_TIME);
}

int main() {
    winInfo lens; 

    xcb_connection_t *conn; 
    xcb_screen_t *screen;
    xcb_window_t win_id;

    conn = xcb_connect(nullptr, nullptr);

    if(!xcb_connection_has_error(conn)) {
        std::cout << "X server connect: OK\n";
    } else {
        std::cerr << "X server connect: FAILED\n";
        return 1;
    }

    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    win_id = create_new_window(conn, screen, lens);

    xcb_map_window(conn, win_id);
    xcb_flush(conn);

    grab_pointer(conn, screen);
    listen_for_pointer(conn, win_id);

    std::cout << "Closing connection\n";
    xcb_disconnect(conn);

    return EXIT_SUCCESS;
}
