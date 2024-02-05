#include <iostream>

#include <unistd.h>
#include <xcb/xcb.h>

int main() {
    //making X server connection
    xcb_connection_t *conn = xcb_connect(NULL, NULL);

    if(!xcb_connection_has_error(conn)) {
        std::cout << "No error detected" << std::endl;
    } else {
        std::cout << "Error occured" << std::endl;
        exit(1);
    }

    //screen setup
    const xcb_setup_t *setup = xcb_get_setup(conn);
    xcb_screen_t *screen = xcb_setup_roots_iterator(setup).data;

    std::cout << "Screen Dimensions: " << screen->width_in_pixels << "x" << screen->height_in_pixels << "\n"; 

    //create the window
    xcb_window_t win_id = xcb_generate_id(conn);
    uint32_t prop_name = XCB_CW_BACK_PIXEL;
    uint32_t prop_val = screen->white_pixel;

    xcb_create_window(conn, screen->root_depth, win_id, screen->root,
                    0, 0, 200, 100, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT, 
                    screen->root_visual, prop_name, &prop_val);
    
    //display the window
    xcb_map_window(conn, win_id);
    xcb_flush(conn); 

    sleep(10);

    xcb_disconnect(conn);
}
