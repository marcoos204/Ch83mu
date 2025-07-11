#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

class Platform 
{
    public:

        Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
        {
            SDL_Init(SDL_INIT_VIDEO);
            window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN); //window opened by the program

            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); //2D rendering context for the window

            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight); //texture for the rendering context
        }

        ~Platform()
        {
            SDL_DestroyTexture(texture);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();

        }

        void Update(void const* buffer, int pitch)
        {
            SDL_UpdateTexture(texture, nullptr, buffer, pitch); //copies video buffer into texture (canvas to write). Need to specify pixel row size (pitch)
            SDL_RenderClear(renderer); //clears renderer info (previous frame)
            SDL_RenderCopy(renderer, texture, nullptr, nullptr); //copies texture info into renderer (info to write on window)
            SDL_RenderPresent(renderer); //presents the info into the window
        }

        bool ProcessInput(uint8_t* keys) //same with buffer, its better to pass as a parameter a pointer for the first position of the static array
        {
            bool quit = false;

            SDL_Event event;

            while (SDL_PollEvent(&event)) //continuous poll checking for events (in this case, keyboard presses)
            {
                switch(event.type)
                {
                    case SDL_QUIT:
                        quit = true;
                        break;
                    
                    case SDL_KEYDOWN:
                    {
                        switch (event.key.keysym.sym)
                        {
                            case SDLK_ESCAPE:
                                quit = true;
                                break;
                            
                            case SDLK_x:
                                keys[0] = 1;
                                break;

                            case SDLK_1:
                                keys[1] = 1;
                                break;

                            case SDLK_2:
                                keys[2] = 1;
                                break;

                            case SDLK_3:
                                keys[3] = 1;
                                break;

                            case SDLK_q:
                                keys[4] = 1;
                                break;

                            case SDLK_w:
                                keys[5] = 1;
                                break;

                            case SDLK_e:
                                keys[6] = 1;
                                break;

                            case SDLK_a:
                                keys[7] = 1;
                                break;

                            case SDLK_s:
                                keys[8] = 1;
                                break;

                            case SDLK_d:
                                keys[9] = 1;
                                break;
                            
                            case SDLK_z:
                                keys[0xA] = 1;
                                break;

                            case SDLK_c:
                                keys[0xB] = 1;
                                break;

                            case SDLK_4:
                                keys[0xC] = 1;
                                break;

                            case SDLK_r:
                                keys[0xD] = 1;
                                break;

                            case SDLK_f:
                                keys[0xE] = 1;
                                break;

                            case SDLK_v:
                                keys[0xF] = 1;
                                break;
                        }
                        break;
                    }
                    break;

                    case(SDL_KEYUP):
                    {
                        switch (event.key.keysym.sym)
                        {
                            case SDLK_x:
                                keys[0] = 0;
                                break;

                            case SDLK_1:
                                keys[1] = 0;
                                break;

                            case SDLK_2:
                                keys[2] = 0;
                                break;

                            case SDLK_3:
                                keys[3] = 0;
                                break;

                            case SDLK_q:
                                keys[4] = 0;
                                break;

                            case SDLK_w:
                                keys[5] = 0;
                                break;

                            case SDLK_e:
                                keys[6] = 0;
                                break;

                            case SDLK_a:
                                keys[7] = 0;
                                break;

                            case SDLK_s:
                                keys[8] = 0;
                                break;

                            case SDLK_d:
                                keys[9] = 0;
                                break;
                            
                            case SDLK_z:
                                keys[0xA] = 0;
                                break;

                            case SDLK_c:
                                keys[0xB] = 0;
                                break;

                            case SDLK_4:
                                keys[0xC] = 0;
                                break;

                            case SDLK_r:
                                keys[0xD] = 0;
                                break;

                            case SDLK_f:
                                keys[0xE] = 0;
                                break;

                            case SDLK_v:
                                keys[0xF] = 0;
                                break;
                                    
                        }
                        break;
                                
                    }
                    break;

                 }
                
            }
            return quit;
        }

    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* texture;

};