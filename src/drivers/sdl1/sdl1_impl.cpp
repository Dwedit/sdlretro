#include "sdl1_impl.h"

#include "sdl1_video.h"
#include "sdl1_audio.h"
#include "sdl1_input.h"
#include "sdl1_font.h"
#include "throttle.h"

#include <core.h>

#include <SDL.h>

#include <cstdint>
#include <unistd.h>

//TODO: Fix usleep
#if _WIN32
void win32_wait_vblank();
#endif




namespace drivers {

sdl1_impl::sdl1_impl() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) != 0) {
        return;
    }
    SDL_WM_SetCaption("SDLRetro", nullptr);
    video = std::make_shared<sdl1_video>();
    input = std::make_shared<sdl1_input>();
}

sdl1_impl::~sdl1_impl() {
    video.reset();
    input.reset();
    audio.reset();
    SDL_Quit();
}

bool sdl1_impl::process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return true;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym ==
#ifdef GCW_ZERO
                    SDLK_HOME
#else
                    SDLK_ESCAPE
#endif
                    ) {
                    if (!menu_button_pressed)
                        menu_button_pressed = true;
                    else
                        return true;
                }
                break;
            default: break;
        }
    }
    return false;
}

bool sdl1_impl::init() {
    audio = std::make_shared<sdl1_audio>();
    static_cast<sdl1_video*>(video.get())->set_force_scale(0);
    return true;
}

void sdl1_impl::deinit() {
    static_cast<sdl1_video*>(video.get())->set_force_scale(1);
    audio.reset();
}

void sdl1_impl::unload() {
}

bool sdl1_impl::run_frame(std::function<void()> &in_game_menu_cb, bool check) {
    if (process_events()) return false;
    if (menu_button_pressed) {
        audio->pause(true);
        in_game_menu_cb();
        audio->pause(false);
        frame_throttle->reset(fps);
        menu_button_pressed = false;
    }
//#if _WIN32
//    win32_wait_vblank();
//    return true;
//#endif

    if (check) {
        int64_t usecs = 0;
        usecs = frame_throttle->check_wait();
        if (usecs > 0) {
            do {
                usleep(usecs);
                usecs = frame_throttle->check_wait();
            } while (usecs > 0);
        } else {
            video->set_skip_frame();
        }
    } else
        frame_throttle->skip_check();
    return true;
}

}
