/*
 *  main.cpp
 *  project
 *
 *  Created by Yuhei Akamine on 08/10/31.
 *  Copyright 2008 Yuhei Akamine. All rights reserved.
 *
 */

#include <SDL2/SDL.h>

#include "App.h"
#include "getPosition.h"

static App app;
static getPosition getPosition;

bool handleEvent(const SDL_Event& e)
{
    switch (e.type) {
        case SDL_MOUSEBUTTONDOWN:
            break;
            
        case SDL_MOUSEBUTTONUP:
            break;
            
        case SDL_MOUSEMOTION:
            break;
            
        case SDL_QUIT:
            return true;
            
    
        default:
            break;
    }
    

    return false;
}

void processMouseEvent()
{
    int mx, my;
    bool left_state = SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT);
    static bool prev_left_state = false;
    
    if(prev_left_state != left_state) {
        if( left_state ) {
            app.mouseLeftPressed(mx, my);
        }else {
            app.mouseLeftReleased(mx,my);
        }
    }
    if(left_state) {
        app.mouseLeftDragged(mx, my);
    }
    
    prev_left_state = left_state;
}


int main(int argc, const char * argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    
    
    int width = 640;
    int height = 480;
    
    SDL_Window* window = SDL_CreateWindow("rge sample", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI /*  | SDL_WINDOW_FULLSCREEN*/);
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    //SDL_GL_SetSwapInterval(1);
    
    
    app.init();
    //float rot = 0;
    
    bool exit_loop = false;
    while(!exit_loop) {
        
        SDL_GL_GetDrawableSize(window, &width, &height);
        
        app.applyWindowSize(width, height);
        
        SDL_Event event;
        if(SDL_PollEvent(&event)) {
            exit_loop = handleEvent(event);
        }

        processMouseEvent();
        
        app.update();
        app.display();
        
        //妖精を回転させる
//        app.dummyData(0,0,rot);
//        rot += 0.8;
//        if (rot > 360){
//            rot = 0;
//        }
        
        
        SDL_GL_SwapWindow(window);
    }
    
    
    SDL_GL_DeleteContext(gl);
    
    
    return 0;
}


