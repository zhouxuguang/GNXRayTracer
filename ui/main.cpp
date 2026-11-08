#include "MiniFB.h"

int main(int argc, char *argv[])
{
    struct mfb_window *window = mfb_open_ex("my display", 800, 600, MFB_WF_RESIZABLE);
    if (window == NULL)
        return 0;

    uint32_t *buffer = (uint32_t*)malloc(800 * 600 * 4);

    mfb_update_state state;
    do 
    {
        state = mfb_update_ex(window, buffer, 800, 600);

        if (state != MFB_STATE_OK)
            break;

    } while(mfb_wait_sync(window));

    free(buffer);
    buffer = NULL;
    window = NULL;

    return 0;
}




