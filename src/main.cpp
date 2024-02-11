#include <iostream>

extern "C" {
    #include <xcb/xcb.h>
    #include <xcb/shape.h>
    #include <unistd.h>
}

int main() {
    xcb_connection_t *conn;
    xcb_screen_t *screen;

    conn = xcb_connect(NULL, NULL);

    if(!xcb_connection_has_error(conn)) {
        std::cout << "X server connect: OK\n";
    } else {
        std::cerr << "X server connect: Failed\n";
        return 1;
    }

    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

    xcb_window_t winId = xcb_generate_id(conn);

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
    uint32_t mask_values[] = {
        screen->white_pixel,
        1, 
        XCB_EVENT_MASK_EXPOSURE
    };

    xcb_create_window(
        conn,
        screen->root_depth,
        winId,
        screen->root,
        0,
        0,
        128,
        128,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        mask,
        mask_values
    );

    xcb_map_window(conn, winId);
    xcb_flush(conn);

    xcb_get_geometry_reply_t *wininfo;
    xcb_query_pointer_reply_t *pointer;
    int16_t winoffset[2];

    while(true) {
        wininfo = xcb_get_geometry_reply(conn, xcb_get_geometry(conn, winId), NULL);
        pointer = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, screen->root), NULL);

        /* making sure window doesn't half in size */
        winoffset[0] = (pointer->root_x + wininfo->width) > screen->width_in_pixels ? 
                        (screen->width_in_pixels - wininfo->width):
                        pointer->root_x;
        winoffset[1] = (pointer->root_y + wininfo->height) > screen->height_in_pixels? 
                        (screen->height_in_pixels- wininfo->height):
                        pointer->root_y;

        std::cout << "wininfo: " << wininfo->x << ", " << wininfo->y << "\n";
        std::cout << "calc winoffset: " << winoffset[0] << ", " << winoffset[1] << "\n\n";

        xcb_configure_window(
            conn,
            winId,
            XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
            winoffset
        );

        xcb_flush(conn);
    }

    xcb_disconnect(conn);

    return 0;
}
