package main

// #cgo LDFLAGS: -lm -pthread
//
// #include "sound.h"
import "C"
import "time"

// import "runtime"
import "fmt"

var TIMING_VARIABLE = 0

func main() {

	// The lfo is strictly handled in Go
	lfo := lfo{resetSwarmSoundTemp: 1, lfoFreqBits: 15, nextFreqBits: 15}

	// Allocate memory for the structs (owned by Go)
	var pp C.pitchplayer
	var tp1 C.timer_555_astable
	var tp2 C.timer_555_astable
	var tp3 C.timer_555_astable

	// Initialize the device and the players, (the device is owned by C)
	err := C.sound_init(&tp1, &tp2, &tp3, &pp)
	defer C.sound_uninit()
	if err > 0 {
		return
	}

	fmt.Println(tp1)
	fmt.Println(tp2)
	fmt.Println(tp3)

	// "Game loop" - 60 fps
	for true {
		handle_swarm_sound(&lfo) // calls C.sound_background_set_freqbits(int) IF it changes frequency
		//	C.sound_update()
		//		-> cv = lfo_update(&lfo) NO need for an extra "sound-loop" in C
		//		-> timer_555_update(&t1, cv)
		//		-> timer_555_update(&t2, cv)
		//		-> timer_555_update(&t3, cv)

		C.pitchplayer_handle_attack_sound(&pp, (C.int)(TIMING_VARIABLE))

		if lfo.nextFreqBits != lfo.lfoFreqBits {
			C.pitchplayer_attack_init(&pp)
		}

		C.timer_555_update(&tp1, (C.double)(lfo.cv))
		C.timer_555_update(&tp2, (C.double)(lfo.cv))
		C.timer_555_update(&tp3, (C.double)(lfo.cv))
		time.Sleep(16 * time.Millisecond)
		TIMING_VARIABLE++
	}
}

////////////////////////	API split (like in MAME)	/////////////////////////////////
//
//	sound_init()
//	sound_close()
//
//	sound_background_set_freqbits(int) // 0..15		6004..6007:	Frequency of the lfo			HANDLE_SWARM_SOUND()
//	sound_background_set_f1(int); // 0/1			6800:		On/Off for the low oscillator	HANDLE_ALIEN_AGGRESSIVENESS()
//	sound_background_set_f2(int); // 0/1			6801:		...mid...						HANDLE_ALIEN_AGGRESSIVENESS()
//	sound_background_set_f3(int); // 0/1			6802:		...high...						HANDLE_ALIEN_AGGRESSIVENESS()
//
//	sound_player_hit(int); // 0/1					6803:		Controls the "noise" explosion	HANDLE_PLAYER_HIT()
//	sound_player_shoot(int); // 0/1					6805:		Controls the shoot-sound		HANDLE_PLAYER_SHOOTING_SOUND()
//
//	sound_complex_set_vol(int, int); // 0/1, 0/1	6806, 6807:	Selects "instrument" (41C0)		HANDLE_SOUND()
//	sound_complex_set_pitch(int); // 0..255			7800:		Controls pitch (41C1),			HANDLE_SOUND()
//
// ----- Set a variable in "some other Go-code"...which will be picked up in the "handle-sound-Go-code":
// RESET_SWARM_SOUND_TEMPO = 1		// or "true"							HANDLE_LEVEL_COMPLETE() + PLAYER_ONE_INIT()
// ENABLE_ALIEN_ATTACK_SOUND = 1	// or "true"							INFLIGHT_ALIEN_PACKS_BAGS()
// PLAY_EXTRA_LIFE_SOUND = 1		// or "true"							AWARD_EXTRA_LIFE()
// PLAY_PLAYER_CREDIT_SOUND = 1		// or "true"							CHECK_IF_COIN_INSERTED()
// PLAY_GAME_START_MELODY = 1		// or "true"							HANDLE_START_BUTTONS()
// ALIEN_DEATH_SOUND = 0x06 or 0x16 // alien death or flagship death		INFLIGHT_ALIEN_DYING_SETUP_ANIM_AND_SOUND()
// PLAY_PLAYER_SHOOT_SOUND = 1		// or "true"							HANDLE_PLAYER_SHOOT()
//
//	========= PORTS: =========
//
//	Background sound
// 6004  !DRIVER Background lfo freq bit0
// 6005  !DRIVER Background lfo freq bit1
// 6006  !DRIVER Background lfo freq bit2
// 6007  !DRIVER Background lfo freq bit3
// 6800  !SOUND  reset background F1
// 6801  !SOUND  reset background F2
// 6802  !SOUND  reset background F3
//
//	Noise sounds: Hits
// 6803  !SOUND  player hit
//
//	Enable/Disable
// 6805  !SOUND  shoot on/off
//
//	Melodies: Start, Death,...
// 6806  !SOUND  Vol of f1
// 6807  !SOUND  Vol of f2
// 7800  !pitch  Sound Fx base frequency

// ----- Complex sounds -----		   === Alla dessa interagerar man med i spelkoden.
// SOUND_VOL                           EQU $41C0         ; Bit 0 and 1 are written to !SOUND Vol of F1 and !SOUND Vol of F2
// PITCH_SOUND_FX_BASE_FREQ            EQU $41C1         ; used to write to !pitch  Sound Fx base frequency. See $171F
//                                     EQU $41D5         ; LOCAL: Store the pitch for the next "note", ...Write to above
// ENABLE_ALIEN_ATTACK_SOUND           EQU $41C2         ; When set to 1, turns on alien attack noise, see $17D0
// UNKNOWN_SOUND_41C3                  EQU $41C3           ...Local variables in "attack sound"
// UNKNOWN_SOUND_41C4                  EQU $41C4         ; Seems to affect the pitch of the alien attack noise.
//
// -----  Complex - Melodies -------   === Dessa finns i HANDLE_SOUND - funktionerna
// PLAY_EXTRA_LIFE_SOUND               EQU $41C7         ; when set to 1, play the sound of an extra life being awarded. See $184F
// EXTRA_LIFE_SOUND_COUNTER            EQU $41C8
// PLAY_PLAYER_CREDIT_SOUND            EQU $41C9         ; when set to 1, play the sound of player credits being added. See $1876
// PLAYER_CREDIT_SOUND_COUNTER         EQU $41CA         ; The higher the value, the longer the player credit sound plays.
//                                     EQU $41CB
// ----- Control playing a melody ----
// IS_COMPLEX_SOUND_PLAYING            EQU $41CD         ; When set to 1, a sequence of sounds, or a melody, is playing.
// PLAY_GAME_START_MELODY              EQU $41D1         ; When set to 1, plays the game start tune.
//                                     EQU $41D2         ; sound related
// COMPLEX_SOUND_POINTER               EQU $41D3         ; If music or complex sound effect is playing, this points to the "notes"
// DELAY_BEFORE_NEXT_SOUND             EQU $41D6         ; counter. When counts to zero the next sound/musical note is played.
// ALIEN_DEATH_SOUND                   EQU $41DF         ; $06: plays alien death sound. $16, plays flagship death sound.
//
// ----- Noice sounds -----
// PLAY_PLAYER_SHOOT_SOUND             EQU $41CC         ; When set to 1, play the sound of the player's bullet. See $1723
// PLAYER_SHOOT_SOUND_COUNTER          EQU $41CE         ; The higher the value, the longer the player spaceship bullet sound plays.
//
// ----- Swarm sound -----
// RESET_SWARM_SOUND_TEMPO             EQU $41D0         ; When set to 1, resets the tempo of the "swarm" sound to slow again. See $1898
//

// ---- "previous state" används bla för knapptryckningar och coins -----
// PORT_STATE_6000                     EQU $4010         ; copy of state for memory address 6000  - LFO Freq Bits
// PORT_STATE_6800                     EQU $4011         ; copy of state for memory address 6800  - Swarm oscillators FS1-FS3
// PREV_PORT_STATE_6000                EQU $4013         ; holds the previous state of memory address 6000
// PREV_PORT_STATE_6800                EQU $4014         ; holds the previous state of memory address 6800
// PREV_PREV_PORT_STATE_6000           EQU $4015         ; holds the previous, previous (!) state of memory address 6000 - LFO
// PREV_PREV_PREV_STATE_6000           EQU $4016         ; holds the previous, previous, previous state of memory address 6000 - LFO
//

type lfo struct {
	resetSwarmSoundTemp int
	freqBits            int
	time                float64
	cv                  float64
}

// Denna ska vara i C
// och anropas från en generell C.sound_update()
//
// func lfo_update(lfo *lfo) {
// 	// Create a lfo sawtooth, wich controls the timers
// 	lfo.time += 0.167							// HACK: The time updates ever 1/60 s.
// 	adv := 1.0 - float64(lfo.freqBits)/16.0		// HACK: The saw is advancing inv of freqBits
// 	sawtooth := lfo.time * adv					// HACK: The sawtooth depends on time and advance rate
// 	lfo.cv = sawtooth*3 + 0.664					// HACK: The cv amplitude:  0.664..3.664
//
// 	if sawtooth >= 1 {
// 		lfo.time = 0
// 	}
//
// 	// To make sure we only change frequency when a new puls starts
// 	if lfo.nextFreqBits != lfo.freqBits && lfo.time == 0 {
// 		lfo.freqBits = lfo.nextFreqBits
// 	}
// }

func handle_swarm_sound(lfo *lfo) {

	if lfo.resetSwarmSoundTemp > 0 {
		// Run this only once
		lfo.resetSwarmSoundTemp = 0
		lfo.freqBits = 0x0F // Maximum value => slowest tempo
		return
	}

	if TIMING_VARIABLE&0xFF != 0 {
		return
	}

	if lfo.freqBits == 0 {
		return
	}

	lfo.freqBits -= 1
	//	C.sound_background_set_freqbits((C.int8_t)lfo.freqBits)
}
