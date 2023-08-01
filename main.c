#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>


#include "wavetables/AKWF_saw8bit.h"
#include "wavetables/AKWF_tri8bit.h"
#include "wavetables/AKWF_fmsynth_0004.h"
#include "wavetables/AKWF_fmsynth_0081.h"
#include "wavetables/AKWF_fmsynth_0096.h"


#define PI 3.14159265
#define WIDTH 512
#define HEIGHT 512


#define NUM_SAMPLES 256
#define SAMPLE_RATE 48000

// Float arrays for the normalized wavetables.
// Must be initialized in the main function.
float saw8bit[NUM_SAMPLES];
float tri8bit[NUM_SAMPLES];
float fmsynth_0004[NUM_SAMPLES];
float fmsynth_0081[NUM_SAMPLES];
float fmsynth_0096[NUM_SAMPLES];



/*
    normalize_uint16_array() converts uint16 array to range [-1,1]
*/

void normalize_uint16_array(const uint16_t input_array[],float output_array[],  int length) {
    // Find the minimum and maximum value in list.
    float min = (float) UINT16_MAX; 
    float max = 0.0f;

    for(int i = 0; i < length; i++) {
        min = input_array[i] < min ? (float) input_array[i] : min;
        max = input_array[i] > max ? (float) input_array[i] : max;
    }
    float_t range = max - min;

    printf("MIN: %f MAX: %f RANGE: %f\n", min,max,range);

    // Convert numbers to range -1,1
    for(int i = 0; i < length; i++) { 
        output_array[i] = 2.0 * (input_array[i] - min) / range - 1;
    }

}

/*
    Struct that will be used by the callback_audio function
*/
typedef struct callback_struct {
    int *sample_nr;
    double *frequency;
    double *new_frequency;
    double *phase;
    double *LFO_phase;
    double *morph_val;
} callback_struct;

/*
    audio_callbak is used by SDL2.
*/
void audio_callback(void *userdata, Uint8 *stream_, int len) {
    Sint16 *stream = (Sint16*)stream_;
    callback_struct *user_data = userdata;

    int sample_len = len / sizeof(Sint16);
    int *sample_nr = (*user_data).sample_nr;
    double *frequency = (*user_data).frequency;
    double *phase = (*user_data).phase;
    double *LFO_phase = (*user_data).LFO_phase;
    double *morph_value = (*user_data).morph_val;
    double phase_incr = 2 * PI * *frequency / SAMPLE_RATE;

    double sample;

    for (int i = 0; i < sample_len; i++, (*sample_nr)++) {
        int k = (int) (*phase * NUM_SAMPLES / (2*PI));
        /* morph two wavetables together. morph_value is controlled by mouse motion */
        sample = (saw8bit[k] * (1- *morph_value) + fmsynth_0096[k] * *morph_value);
        //sample = fmsynth_0096[k];
        *stream++ = (Sint16) (sample * 20000);

        *phase = fmod(*phase + phase_incr, 2*PI);
        // *LFO_phase = fmod(*LFO_phase + 2 *PI*0.3 / SAMPLE_RATE, 2*PI);

    }

}

int main(int argc, char* argv[])
{
    
    
    // Normalize wavetable arrays. 
    normalize_uint16_array(AKWF_saw8bit, saw8bit, NUM_SAMPLES);
    normalize_uint16_array(AKWF_tri8bit, tri8bit, NUM_SAMPLES);
    normalize_uint16_array(AKWF_fmsynth_0004, fmsynth_0004, NUM_SAMPLES);
    normalize_uint16_array(AKWF_fmsynth_0081, fmsynth_0081, NUM_SAMPLES);
    normalize_uint16_array(AKWF_fmsynth_0096, fmsynth_0096, NUM_SAMPLES);




    // Set up SDL
    SDL_Window *window;
    SDL_Renderer *renderer;

    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
    {
        printf("Error initializing sdl\n");
        return -1;
    }

    SDL_CreateWindowAndRenderer(WIDTH,HEIGHT, 0, &window, &renderer);
    if(!window)
    {
        printf("Failed to create window\n");
        return -1;
    }

    // Set up user_data struct
    struct callback_struct user_data;
    int sample_nr = 0;
    double frequency = 150.0;
    double phase = 0.0;
    double LFO_phase = 0.0;
    double morph_val = 0.0;
    user_data.sample_nr = &sample_nr;
    user_data.frequency = &frequency;
    user_data.phase = &phase;
    user_data.LFO_phase = &LFO_phase;
    user_data.morph_val = &morph_val;

    SDL_AudioSpec want,have;
    SDL_AudioDeviceID dev;

    SDL_memset(&want, 0, sizeof(want));
    want.freq = 48000;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audio_callback;
    want.userdata = &user_data;
    if((dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE)) < 0) {
        printf("Error open audio device\n");
        return -1;
    }

    SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Play audio
    SDL_PauseAudioDevice(dev, 0);

    // Main loop
    int mousex,mousey;
    int run_program = 1;
    while(run_program) {
        SDL_Event e;
        while(SDL_PollEvent(&e) > 0)
        {
            switch(e.type) {
                case SDL_QUIT:
                    run_program = 0;
                    break;

                case SDL_MOUSEMOTION:
                     SDL_GetGlobalMouseState(&mousex,&mousey);
                     float x_normalized = (float) (mousex);
                     float y_normalized = (float) mousey / HEIGHT;

                     morph_val = y_normalized;
                     frequency = x_normalized;

                     continue;

                default:
                    break;
            }       
        }   
        SDL_RenderPresent(renderer);

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