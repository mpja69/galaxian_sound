#include <stdint.h>

int sound_init();    // Initializes the device and all oscillators
void sound_close();   // Close the device

void sound_update();  // Updates the lfo and and all oscillators, (based on the internal variables)

// Background
void sound_background_freq_set(int8_t freqBits);   // 0..15	6004..6007:	Frequency of the lfo		HANDLE_SWARM_SOUND:
void sound_background_enable(uint8_t fs1, uint8_t fs2, uint8_t fs3); // 0/1 6800: for the oscillator	HANDLE_ALIEN_AGGRESSIVENESS:

// Noise related
void sound_hit(uint8_t);   // 0/1		6803:		Controls the "noise" explosion	HANDLE_PLAYER_HIT:
void sound_fire(uint8_t); // 0/1		6805:		Controls the shoot-sound	HANDLE_PLAYER_SHOOTING_SOUND:

// Melodies and complex sounds
void sound_vol_set(uint8_t vol1, uint8_t vol2); // 0/1, 0/1	6806, 6807:	Selects "instrument" (41C0)	HANDLE_SOUND:
void sound_pitch_set(uint8_t pitch);    // 0..255	7800:		Controls pitch (41C1),		HANDLE_SOUND:


//vim: ts=4
