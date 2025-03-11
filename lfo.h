#include <stdint.h>
#include "miniaudio.h"

typedef struct {
    double time;		// Used by the RC 
    int resetSwarmSoundTemp;	// Set to 1 when level completed. (Will start at slow tempo again.)
    int nextFreqBits;	
    int lfoFreqBits;		// The value set in HANDLE_SWARM_SOUND
    double cv;			// The control voltage based on the frequency bits, that controls the 3 555-timers
} lfo_t;

void lfo_init(lfo_t* lfo);
double lfo_update(lfo_t* lfo);


