#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_timer.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <cmath>

#include <emscripten.h>
class Sphere
{
private:
    int m_r2, m_x, m_y, l_x, l_y, l_dist;
public:
    Sphere(){}
    Sphere(int r, int x = 0, int y = 0):m_r2(r*r), m_x(x), m_y(y){
        genLight(m_x, m_y);
    }

    void genLight(int x, int y){
        l_x = x - m_x;
        l_y = y - m_y;
        int l = l_x*l_x+l_y*l_y;
        if (l > m_r2) {
            l_x = l_x*sqrt(m_r2)/sqrt(l);
            l_y = l_y*sqrt(m_r2)/sqrt(l);
        }
        l_dist = m_r2-l_x*l_x-l_y*l_y;
    }

    void incSize(){
        m_r2 *= 1.2;
        l_dist *= 1.2;
    }

    void deSize(){
        m_r2 *= 0.8;
        l_dist *= 0.8;
    }

    void move(int mov_x, int mov_y){
        m_x += mov_x;
        m_y += mov_y;
    }

    void renderLight(SDL_Renderer * renderer, unsigned char * bg_color, int width, int height){
        long y2, x2;
        SDL_SetRenderDrawColor(renderer,
            bg_color[0], bg_color[1], bg_color[2], 255);
        SDL_RenderClear(renderer);
        for (unsigned y = 0; y < height; y++) {
            int y_ = y - m_y;
            y2 = y_*y_;
            for (unsigned x = 0; x < width; x++) {
                int x_ = x - m_x;
                x2 = x_*x_;
                if (x2 + y2 > m_r2) {
                    long pq = l_x*x_ + l_y*y_;
                    long sqrt_term = ((pq*pq)/m_r2 - x2 - y2 + m_r2);
                    if (sqrt_term > 0 && (sqrt_term > pq*pq || pq < 0)) {
                        SDL_SetRenderDrawColor(renderer,
                            bg_color[0]/2, bg_color[1]/2, bg_color[2]/2, 255);
                        SDL_RenderDrawPoint(renderer, x, y);
                    }
                    else {
                    }
                }
                else {
                    int color = (l_x * x_ + l_y * y_ + sqrt(l_dist*(m_r2 - x2 - y2)))*255/m_r2;
                    color = (color >  0) ? color : 0;
                    SDL_SetRenderDrawColor(renderer,
                        color, color, color, 255);
                    SDL_RenderDrawPoint(renderer, x, y);
                }
            }
        }
        SDL_RenderPresent(renderer);
    }
};

Sphere s;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
unsigned char bg_color[] = {60, 100, 150};
int width = 300;
int height = 300;
bool quit = false;

void main_loop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit = true;
            emscripten_cancel_main_loop();
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                s.genLight(event.button.x, event.button.y);
            }
        }
        else if (event.type == SDL_KEYDOWN) {
            SDL_Keycode key = event.key.keysym.sym;
            if (key == SDLK_PLUS) s.incSize();
            else if (key == SDLK_MINUS) s.deSize();
            else if (key == SDLK_UP || key == SDLK_k) s.move(0, -8);
            else if (key == SDLK_DOWN || key == SDLK_j) s.move(0, 8);
            else if (key == SDLK_LEFT || key == SDLK_h) s.move(-8, 0);
            else if (key == SDLK_RIGHT || key == SDLK_l) s.move(8, 0);
            else if (key == SDLK_r) bg_color[0] += 10;
            else if (key == SDLK_g) bg_color[1] += 10;
            else if (key == SDLK_b) bg_color[2] += 10;
        }
    }

    // Always redraw
    s.renderLight(renderer, bg_color, width, height);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_SHOWN, &window, &renderer);
    SDL_RenderSetScale(renderer, 1, 1);

    s = Sphere(150, width / 2, height / 2);

    // Let Emscripten call main_loop at ~60fps
    emscripten_set_main_loop(main_loop, 0, 1);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
