#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

#define PI 3.1459265

typedef void (*periodic_function)(double time, float frequency, Sint16 *sample, int max);
typedef struct callback_struct {
    int *sample_nr;
    periodic_function periodic_function;

} callback_struct;

/*
    Sinewave function 
*/    
void sinewave(double time, float frequency, Sint16 *sample, int max) {
    double val = sin(2 * PI * frequency * time)*max;
    *sample = (Sint16)(val);
}

/*
    Squarewave function
*/
void squarewave(double time, float frequency, Sint16 *sample, int max) {
    double val = sin(2 * PI * frequency * time)*max;
    *sample = (Sint16)(val >= 0 ? max : -max);

}

/*
    Sawtooth function
*/

void sawtooth(double time, float frequency, Sint16 *sample, int max) {
    float p = 1.0/frequency;
    double val = 2 * ((time/p) - floor(0.5 + (time/p))) * max;
    *sample = (Sint16)val;
}

void audio_callback(void *userdata, Uint8 *stream_, int len) {

    Sint16 *stream = (Sint16*)stream_;
    callback_struct *user_data = userdata;

    float freq = 220.0;
    float duration = 3;
    float sample_len = len / sizeof(Sint16);
    int *sample_nr = (*user_data).sample_nr;
    float phase = 0.0;

    Sint16 sample;
    for(int i = 0; i < sample_len;i++, (*sample_nr)++) {
        double time = (double) *sample_nr / 44100;
        Sint16 sample;
        (*user_data).periodic_function(time, freq, &sample, 28000);
        *stream++ = sample;
    }

}

int main(int argc, char* argv[])
{


    SDL_Window *window;
    SDL_Renderer *renderer;


    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
    {
        printf("Error initializing sdl\n");
        return -1;
    }

    SDL_CreateWindowAndRenderer(680,440, 0, &window, &renderer);

    if(!window)
    {
        printf("Failed to create window\n");
        return -1;
    }

    struct callback_struct user_data;
    int sample_nr = 12;
    user_data.sample_nr = &sample_nr;
    user_data.periodic_function = sawtooth;

    SDL_AudioSpec want,have;
    SDL_AudioDeviceID dev;

    SDL_memset(&want, 0, sizeof(want));
    // Samples per second
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = audio_callback;
    want.userdata = &user_data;
    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);


    // Play audio
    SDL_PauseAudioDevice(dev, 0);

    // Render wave function
    SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255,0,0,255);

    // draw wave-function
    double draw_freq = 5/680.0;
    int prev_sample = 0;
    for(int i=1; i < 680; i++) {
        Sint16 sample;
        user_data.periodic_function((double) i, draw_freq, &sample, 100);
        SDL_RenderDrawLine(renderer,i-1,prev_sample + 170,i,(int) sample + 170);
        prev_sample = (int) sample;
    }

    SDL_RenderPresent(renderer);

    // Main loop
    int run_program = 1;
    while(run_program) {
        SDL_Event e;
        while(SDL_PollEvent(&e) > 0)
        {
            // Handle events 
            switch(e.type) {
                case SDL_QUIT:
                    run_program = 0;
                    break;

            }
            SDL_Delay(33);
            SDL_RenderPresent(renderer);
        }   
    }

    // Stop playing audio
    SDL_PauseAudioDevice(dev, 1);
    
    // Clean up and quit
    SDL_CloseAudioDevice(dev);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}