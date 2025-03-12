#include "sound.h"
#include <stdio.h>
#include <unistd.h>
#import <sys/time.h>


typedef struct 
{
    // Gaming variables
    int TIMING_VARIABLE;
    int HAVE_NO_INFLIGHT_ALIENS;
    int IS_GAME_IN_PLAY;
    int IS_GAME_OVER;

    // Data for the Background circuit
    int resetSwarmSoundTemp;	// $41D0,   Set to 1 when level completed. (Will start at slow tempo again.)
    int lfoFreqBits;		// The value set in HANDLE_SWARM_SOUND

    // Data for the PITCH circuit
    int pitch;			// $41C1
    int vol1;			// $41C0
    int vol2;
    
    // Which melody/notes to play
    int playGameStartMelody;	// $41D1
    int playAlienDeathSound;	// ($41DF) 
    int playFlagshipDeathSound;	// ($41CF)

    // Complex sound
    int melodyIndex;
    int delayBeforeNextSound;	// $41D6
    int isComplexSoundPlaying;	// $41CD
    int *complexSoundPointer;	// $41D3
    int complexPitch;		// $41D5

    int creditSoundCounter;	// $41CA
    int creditPitch;		// $41CB
    int playPlayerCreditSound;	// $41C9

    int extraLifeSoundCounter;	// $41C8
    int playExtraLifeSound;	// $41C7
    
    int enableAlienAttackSound;	// $41C2
    int sirenCounter;
    int wheeePitch;

    int playPlayerShootSound;	// $41CC
    int playerShootSoundCounter;// $41CE

    int isPlayerDying;
    int playerExplosionCounter;
} game_t;

// ============================ Data ====================================
// GAME START melody. $1E68
const int GAME_START[] = {
	0x11, 0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x41, 0x42, 0x41, 0x42, 0x45,
	0x42, 0x45, 0x47, 0x45, 0x47, 0x6A, 0x60, 0x41, 0x42, 0x41, 0x42, 0x45, 0x42, 0x45, 0x47, 0x45,
	0x47, 0x6A, 0x60, 0x45, 0x23, 0x24, 0x23, 0x24, 0x23, 0x24, 0x23, 0x24, 0x23, 0x24, 0x23, 0x24,
	0x23, 0x24, 0x23, 0x24, 0x02, 0x03, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x02, 0x03, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x0A, 0x02, 0x03, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x02, 0x03, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x0A, 0xE0,
};

// ALIEN DEATH sound effect. $1EBD  
const int ALIEN_DEATH[] = {
	0x08, 0x07, 0x06, 0x05, 0x03, 0x02, 0x08, 0x07, 0x06, 0x05, 0x03, 0x02, 0x02, 0x03, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0C,
	0x0D, 0xE0
};

// FLAGSHIP DEATH sound effect. $1EDF
const int FLAGSHIP_DEATH[] = {
	0x02, 0x17, 0x16, 0x01, 0x16, 0x02, 0x03, 0x05, 0x06, 0x07, 0x18, 0x20, 0x07, 0x06, 0x05, 0x03,
	0x02, 0x03, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x19, 0x20, 0x0A, 0x09, 0x08, 0x07, 0x08, 0x0A, 0x0B,
	0x0C, 0x0D, 0x0E, 0x1A, 0x20, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
	0x1B, 0x3C, 0xE0,
};

// Pitch value is the the start value of counter
// 2 cascade connected 74161-counters, counts towards 255, 1 536 000 times a second
// Every time we get to 255, the binary counter 74393, creates 1 sample of the wave
// The 74393n binary counter, counts to 16, (so 16 samples per wave)

const int PITCH[] = {
	0xFF, 0x00, 0x40, 0x55, 0x5F, 0x68, 0x70, 0x80, 0x8E, 0x9A, 0xA0, 0xAA, 0xB4, 0xB8, 0xC0, 0xC7,
	0xCD, 0xD0, 0xD5, 0xDA, 0xDC, 0xE0, 0x1C, 0x35, 0x87, 0xA5, 0xC4, 0xD3, 0xCA, 0xE3, 0xE6,
}; // $17A9:

// getPitch:  (lowest 5 bits of "Note" is index into table) Write to $41D5
static int getPitch(int note) {
	int idx = note & 0x1F;
	int pitch = PITCH[idx];
	return pitch;
}

const int TONE[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00}; // $17C8:

// getLength: (Highest 3 bits, but shifted to bit 2,1,0 -> index into table) Write to $41D6
static int getLength(int note) {
	int idx = (note & 0xE0) >> 5;
	return TONE[idx];
}

// ============================== Functions for playing different sounds =============================

void handle_game_start_melody(game_t* g) {
    if (g->playGameStartMelody == 0) { return; }

    g->playGameStartMelody = 0;
    g->melodyIndex = 0;
    g->isComplexSoundPlaying = 1;
    g->delayBeforeNextSound = 1;
    g->complexSoundPointer = (int*)GAME_START;
}

void handle_alien_death_sound(game_t* g) {
    if (g->IS_GAME_IN_PLAY == 0) { return; }	// No sound in demo mode
    if (g->playAlienDeathSound == 0) { return; }

    g->playAlienDeathSound = 0;
    g->melodyIndex = 0;
    g->isComplexSoundPlaying = 1;
    g->delayBeforeNextSound = 1;
    g->complexSoundPointer = (int*)ALIEN_DEATH;
}

// Added this func to separate the 2 different death sounds. And chnaged variables to simplify logic.
void handle_flagship_death_sound(game_t* g) {
    if (g->IS_GAME_IN_PLAY == 0) { return; }	// No sound in demo mode
    if (g->playFlagshipDeathSound == 0) { return; }

    g->playFlagshipDeathSound = 0;
    g->melodyIndex = 0;
    g->isComplexSoundPlaying = 1;
    g->delayBeforeNextSound = 1;
    g->complexSoundPointer = (int*)FLAGSHIP_DEATH;
}

// Handling which note to play according to the coded melody, (Both length of the note and the pitch of the note)
void handle_complex_sound(game_t* g) {
    if (g->isComplexSoundPlaying == 0 ) { return; }
    if (g->complexSoundPointer == NULL) { return; } // For good measure / safety

    g->vol1 = 0;
    g->vol2 = 1;
    g->pitch = g->complexPitch;

    // Are we still playing a note?
    g->delayBeforeNextSound -= 1;
    if (g->delayBeforeNextSound > 0) { return; }

    int note = g->complexSoundPointer[g->melodyIndex];
    if (note == 0xE0) {
	// End of sound marker (E0)
	g->isComplexSoundPlaying = 0;
	return;
    }

    // ...otherwise get the pitch and length of next note
    g->delayBeforeNextSound = getLength(note);
    g->complexPitch = getPitch(note);
    g->melodyIndex += 1;
    g->pitch = g->complexPitch;
}

void handle_coin_insert_sound(game_t* g) {
    if (g->playPlayerCreditSound == 1) {
	g->playPlayerCreditSound = 0;
	g->creditSoundCounter = 0x20; // 32 dec
	g->vol1 = 0;
	g->vol2 = 0;
	g->creditPitch = 0;
	return;
    }

    // Are we done playing?
    if (g->creditSoundCounter == 0) { return; }

    g->creditSoundCounter -= 1;
    g->creditPitch += 4;

    g->pitch = g->creditPitch;
}

// Extra life: counter: 0x80, pitch: oscillating 0x80 / 0x00, vol1: 1, vol2: 0 
void handle_extralife_sound(game_t* g) {
    if (g->playExtraLifeSound == 1) {
	g->playExtraLifeSound = 0;
	g->extraLifeSoundCounter = 0x80; // 128 dec
	return;
    }

    // Are we done playing?
    if (g->extraLifeSoundCounter == 0) { return; }

    g->vol1 = 1;
    g->vol2 = 0;
    
    g->extraLifeSoundCounter -= 1;
    if (g->extraLifeSoundCounter & 4) {
	g->pitch = 0x80;
	return;
    }
    g->pitch = 0xFF;
}

void handle_attack_sound(game_t* g) {
    if (!g->IS_GAME_IN_PLAY) { return; } // No sound in demo mode

    if (g->enableAlienAttackSound == 1) {
	g->enableAlienAttackSound = 0;	// $41C2
	g->sirenCounter = 0x02;		// $41C3
	g->wheeePitch = 0xA0;		// $41C4
	return; // NOTE: Onödig return?!
    }

    if (g->HAVE_NO_INFLIGHT_ALIENS) { return; }

    // $17EB
    if ((g->TIMING_VARIABLE & 1) == 0) {
	// $17F1
	if (g->wheeePitch < 0x60)	{ g->sirenCounter += 1; }	// Start siren when alien dived for 0x40, (64) cycles
	if (g->wheeePitch > 0)		{ g->wheeePitch -= 1; }		// Wheeee sound for 0xA0, (160) cycles
    }

    // $1801
    if ( (g->sirenCounter & 3) == 0){
	// $1807
	g->pitch = 0x60; 
	// jp $1815
    } else {
	// $180C
	if ((g->sirenCounter & 2) == 0) { 
	    // $1812
	    g->pitch = (0x60 + g->wheeePitch) / 2;
	    // jp $1815
	} else {
	    //$1807
	    g->pitch = g->wheeePitch;
	    // jp $1815
	}
    }
    // $1815
}

void handle_player_shooting_sound(game_t* g) {
    if (g->IS_GAME_OVER) { return; }

    if (g->playPlayerShootSound == 1) {
	g->playPlayerShootSound = 0;
	g->playerShootSoundCounter = 0x08;
	return;
    }

    // Are we done playing?
    if (g->playerShootSoundCounter == 0) { 
	sound_fire(0);
	return; 
    }
    g->playerShootSoundCounter -= 1;

    sound_fire(1);
}

void handle_player_dying(game_t* g) {
    if (g->IS_GAME_OVER) { return; }

    if (g->isPlayerDying == 1) {
	g->isPlayerDying = 0;
	g->playerExplosionCounter = 0x10;
	return;
    }

    // Are we done playing?
    if (g->playerExplosionCounter == 0) { 
	sound_hit(0);
	return; 
    }
    g->playerExplosionCounter -= 1;

    sound_hit(1);
}


void handle_sound(game_t* g) {
    g->vol1 = 0;
    g->vol2 = 0;
    g->pitch = 0xFF;

    handle_game_start_melody(g);
    handle_attack_sound(g);
    handle_alien_death_sound(g);
    handle_flagship_death_sound(g); // Added extra function for simplicity and concistancy
    handle_complex_sound(g);
    handle_extralife_sound(g);
    handle_coin_insert_sound(g);
    handle_player_shooting_sound(g);
    handle_player_dying(g);

    sound_vol_set(g->vol1, g->vol2);
    sound_pitch_set(g->pitch);
}

void handle_swarm_sound(game_t* g) {
    if (g->resetSwarmSoundTemp > 0) {
        g->resetSwarmSoundTemp = 0;
	g->lfoFreqBits = 0x0F; // Maximum value => slowest tempo
        return;
    }

    if (g->TIMING_VARIABLE & 0xFF) { return; }
    if (g->lfoFreqBits == 0) { return; }

    g->lfoFreqBits -= 1;

    sound_background_freq_set((uint8_t) g->lfoFreqBits);
}


int main(int argc, char** argv) {
    game_t g = {.resetSwarmSoundTemp = 1, .HAVE_NO_INFLIGHT_ALIENS = 1, .IS_GAME_IN_PLAY = 1, .IS_GAME_OVER = 0};

    int err = sound_init();
    printf("Sound started: %d\n", err);

    //sound_background_enable(1, 1, 1);
    g.enableAlienAttackSound = 1;	    // + HAVE_NO_INFLIGHT_ALIENS = 0
    //g.playPlayerCreditSound = 1;
    //g.playExtraLifeSound = 1;
    //g.playGameStartMelody = 1;
    //g.playAlienDeathSound = 1;	    // + IS_GAME_IN_PLAY = 1
    //g.playFlagshipDeathSound = 1;	    // + IS_GAME_IN_PLAY = 1
    //g.playPlayerShootSound = 1;		    // + IS_GAME_OVER = 0
    g.isPlayerDying = 1;		    // + IS_GAME_OVER = 0

    // "Game loop" - 60 fps
    struct timeval t0, t1;
    int sleepTime = 1e6 / 60;
    int dt = 0;
    int shiftreg = 0;
    while (1) {
	
	gettimeofday(&t0, NULL);
	sound_update();
	handle_sound(&g);
	handle_swarm_sound(&g);
   	gettimeofday(&t1, NULL);
	dt = t1.tv_sec*1e6 + t1.tv_usec  - t0.tv_sec*1e6 - t0.tv_usec;

        g.TIMING_VARIABLE++;

	//printf("main: Shooting: %d, Counter: %d\n", g.playPlayerShootSound, g.playerShootSoundCounter);

        usleep(sleepTime - dt);
    }

    sound_close(); 
    return 0;
}

// vim: ts=4


