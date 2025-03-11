#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "sound.h"
#include "lfo.h"
#include "timer_555.h"
#include "pitch.h"
#include "fire_sound.h"
//#include "lfsr.h"

#define DEVICE_FORMAT ma_format_f32
#define DEVICE_CHANNELS 2
#define DEVICE_SAMPLE_RATE 48000

// Background sounds
lfo_t lfo = {};
timer_555_astable_t t1 = {};
timer_555_astable_t t2 = {};
timer_555_astable_t t3 = {};

// Pitch sounds
pitch_t p = {};

// Fire sound
fire_t fire = {};
timer_555_astable_t t4 = {};
lfsr_t lfsr = {};

ma_device device = {};


void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{

    MA_ASSERT(pDevice->playback.channels == DEVICE_CHANNELS);

    ma_uint64 iFrame;
    ma_uint64 iChannel;
    float* pFramesOutF32 = (float*)pOutput;

    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        float signal = 0;
        signal =  pitch_wavefunc(&p);
        signal += timer_555_wavefunc(&t1);
        signal += timer_555_wavefunc(&t2);
        signal += timer_555_wavefunc(&t3);
        // Fire
        signal += fire_wavefunc(&fire);

        for (iChannel = 0; iChannel < pDevice->playback.channels; iChannel += 1) {
            pFramesOutF32[iFrame * pDevice->playback.channels + iChannel] = signal;
        }
    }
}




// TODO: Should be set within sound_init
/*const int FPS = 60;*/
/*const double PERIOD = 1.0 / FPS;*/
/*double timer = 0;*/
int sound_init()
{
    // The background sound
    lfo_init(&lfo);
    timer_555_init(&t1, DEVICE_SAMPLE_RATE, R22, R23, C17);
    timer_555_init(&t2, DEVICE_SAMPLE_RATE, R25, R26, C18);
    timer_555_init(&t3, DEVICE_SAMPLE_RATE, R28, R29, C19);
    
    // Coin, Start, Extra life, Attack, Alien & Flagship Hit sounds
    pitch_init(&p, DEVICE_SAMPLE_RATE);

    // Fire & Player Hit sounds
    lfsr_init(&lfsr);
    fire_init(&fire, DEVICE_SAMPLE_RATE);

    // PLAYBACK DEVICE
    // (The miniaudio-way: Always initialize ma-objects by providing a "params-struct")
    ma_device_config deviceConfig;
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = DEVICE_FORMAT;
    deviceConfig.playback.channels = DEVICE_CHANNELS;
    deviceConfig.sampleRate = DEVICE_SAMPLE_RATE;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = NULL;

    // Init the device by giving it parames, gathered in a struct
    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return -4;
    }
    printf("Device Name: %s\n", device.playback.name);

    // Start the playback/output device
    // (The miniaudio-way: Check for errors and return a specific negative number of choice, to identify the error)
    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return -5;
    }

    return 0;
}

void sound_close() 
{
    ma_device_uninit(&device);
}

// Called 60 time a second from the gameloop
// Used to update stuff that changes over time...that affects the waveforms
//  - Background:
//      - The lfo needs to updated, and that is used to control the 3 timers
//      - The lfsr needs to updated, by filling a buffer with MANY samples, to control "FIRE"
//      - The fire needs to be updated, envelope and timer
void sound_update()
{
    // Background
    double cv = lfo_update(&lfo);
    timer_555_update(&t1, cv);
    timer_555_update(&t2, cv);
    timer_555_update(&t3, cv);
    // TODO: Add mixer??

    // Fire
    fire_update(&fire);
    

}

void sound_pitch_set(uint8_t pitch)    // 0..255	7800:		Controls pitch (41C1),		HANDLE_SOUND:
{
    pitch_set_pitch(&p, pitch);
}

void sound_background_freq_set(int8_t freqBits) 
{
    lfo.nextFreqBits =  freqBits; 
}

void sound_vol_set(uint8_t vol1, uint8_t vol2) // 0/1	6806, 6807:	Selects "instrument" (41C0)	HANDLE_SOUND:
{
    p.vol1 = vol1;
    p.vol2 = vol2;
}

void sound_background_enable(uint8_t fs1, uint8_t fs2, uint8_t fs3) // 0/1 for the oscillators	HANDLE_ALIEN_AGGRESSIVENESS:
{
    t1.enabled = fs1;
    t2.enabled = fs2;
    t3.enabled = fs3;
}

void sound_fire_enable(uint8_t enable) // 0/1		6805:		Controls the shoot-sound	HANDLE_PLAYER_SHOOTING_SOUND:
{
    fire_set_enable(&fire, enable);
}
// vim: ts=4
