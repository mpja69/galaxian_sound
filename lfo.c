#include "galaxian_board.h"
#include "miniaudio.h"
#include "lfo.h"
#include <stdint.h>
#include <stdio.h>



void lfo_init(lfo_t* lfo) {
    lfo->resetSwarmSoundTemp = 1;
    lfo->lfoFreqBits = 15;
    lfo->nextFreqBits = 15;

}

double lfo_update(lfo_t* lfo) {
    // Create a lfo sawtooth, wich controls the timers 
    lfo->time +=  0.167;				// HACK: The time updates ever 1/60 s.
    double adv = 1 - lfo->lfoFreqBits / 16.0;		// HACK: The advance of the saw wave inversely proportional to the freqBits
    double sawtooth = lfo->time * adv;			// HACK: The sawtooth depends on time and advance rate
    lfo->cv = sawtooth * 3 + 0.664;			// HACK: The cv amplitude:  0.664..3.664

    if (sawtooth >= 1) {
	lfo->time = 0;
    }

    // To make sure we only change frequency when a new puls starts
    if (lfo->nextFreqBits != lfo->lfoFreqBits && lfo->time == 0) {
	lfo->lfoFreqBits = lfo->nextFreqBits;
    }
    return lfo->cv;
}

/*#define R15 RES_K(100) // These 4 are connected to 9M*/
/*#define R16 RES_K(220)*/
/*#define R17 RES_K(470)*/
/*#define R18 RES_K(1000)*/
/*#define R19 RES_K(330) // Called R16 in schematics. For division*/
/*#define R20 RES_K(15) // These 2 for transistor Q1*/
/*#define R21 RES_K(100) // ...and Q2*/
/*#define C15 CAP_U(1)	// The draining cap for Q2*/

// Chip 9M: 74259, 8-bit addressable latch
// Output pins: 9..12 -> Q4..Q7
// Input:
//	Data: bit 0 on the data bus
//	Addr: bit 0..3 on the address bus + DRIVER
//
// Q7	Q6  Q4	Q4
// R15	R15 R17	R18
//
// Voltage division with: 330K (R16?? again in schematics, called R_DIV here) ->
// styr en 555...som sedan kopplas till 3 andra 555 Calculate audio voltages


// LFO_FREQ_BITS: $421F, Value used to set !DRIVER Background lfo frequency
// ports (0-3) for the "swarm" noise
// -------------- All this isn't really necessary...now when I work with a normalized value -------------------
/*float lfo_freqBits_norm(lfo* lfo)*/
/*{*/
/**/
/*    int q4 = lfo->lfoFreqBits >> 0 & 1; // $6004 lfo freq bit 0*/
/*    int q5 = lfo->lfoFreqBits >> 1 & 1; // §6005 lfo freq bit 1*/
/*    int q6 = lfo->lfoFreqBits >> 2 & 1; // $6006 lfo freq bit 2*/
/*    int q7 = lfo->lfoFreqBits >> 3 & 1; // $6007 lfo freq bit 3*/
/**/
/**/
/*    double v4 = q4 / R18;*/
/*    double v5 = q5 / R17;*/
/*    double v6 = q6 / R16;*/
/*    double v7 = q7 / R15;*/
/*    double vi = 1 / R20;*/
/**/
/*    // The voltage of the base of the transistors Q1 and Q2 (simplified)*/
/*    double base = 5 * R19 / (1/(v4+v5+v6+v7+vi) + R19)-0.6; // 5V Vcc - 0.6 (Transistor as diod)*/
/*    // bits: 15 -> 4.227*/
/*    // bits: 14 -> 4.225*/
/*    // ...*/
/*    // bits: 1  -> 4.186*/
/*    // bits: 0: -> 4.183*/
/*    //*/
/*    // The Monostable 555: Maps this to:    1.212...3.333 (the input to the positive OP)*/
/*    // The OP amp part: Maps this to:	    0.664...3.688 (the CV of the 3 astable 555:s)*/
/*    double b_max = 5 * R19 / (1/(1/R18 + 1/R17 + 1/R16 +  1/R15 + 1/R20) + R19)-0.6;*/
/*    double b_min = 5 * R19 / (1/(0/R18 + 0/R17 + 0/R16 +  0/R15 + 1/R20) + R19)-0.6;*/
/*    double b_norm = (base-b_min) / (b_max-b_min); */
/*    return b_norm;*/
/*}*/

// 3 lowest bits in $4011 controls F1..F3, 1 resets, (only set in HANDLE_ALIEN_AGGRESSIVENESS and )
// $6800 SOUND F1 (FS1) 
// $6801 SOUND F1 (FS2)
// $6802 SOUND F1 (FS3)
// $6803 SOUND Hit
// $6805 SOUND Fire
// $6806 SOUND Vol1 (see pitch_player)
// $6807 SOUND Vol2 (see pitch_player)





/************************************** MAME ************************************
 *
 * Background - The component-stack:
 *    0. 74ls259, Q4..Q7,   GAL_INP_BG_DAC  (NODE_10)
 *    1. voltage division - resistor ladder: BUG: Kolla hur den är uppsat och dess 15 olika outputs
 *    2. first 555
 *    3. op-amp
 *    4. "clamp"
 *    5. the 3 astable timer 555 with different frequencies
 *    6. mixing the the 3 outputs together
 *
//INFO:  src/mame/galaxian/galaxian_a.cpp: 377

		DISCRETE_DAC_R1(NODE_100, GAL_INP_BG_DAC, TTL_OUT, &galaxian_bck_dac)
				    |
				    +-------------+	
						  |
						  V
		DISCRETE_555_CC(NODE_105, 1, NODE_100, GAL_R21, GAL_C15, 0, 0, 0, &galaxian_bck_vco)
			          |
			          +------------+	
					       |
					       V	
mult/add opamp: DISCRETE_MULTADD(NODE_110, NODE_105, GAL_R33/RES_3_PARALLEL(GAL_R31,GAL_R32,GAL_R33),-5.0*GAL_R33/GAL_R31)
				    |
				    +-------+	
					    |
					    V
		DISCRETE_CLAMP(NODE_111,NODE_110,0.0,5.0)
				    |
				    +--------------------------------------------------------+	
											     |
											     V
The three 555:	DISCRETE_555_ASTABLE_CV(NODE_115, GAL_INP_FS1, GAL_R22, GAL_R23, GAL_C17, NODE_111, &galaxian_555_vco_desc)
		DISCRETE_555_ASTABLE_CV(NODE_116, GAL_INP_FS2, GAL_R25, GAL_R26, GAL_C18, NODE_111, &galaxian_555_vco_desc)
		DISCRETE_555_ASTABLE_CV(NODE_117, GAL_INP_FS3, GAL_R28, GAL_R29, GAL_C19, NODE_111, &galaxian_555_vco_desc)
					  |
					  +-----+---------+---------+	
						|         |         |
						V         V         V
		DISCRETE_MIXER3(NODE_120, 1, NODE_115, NODE_116, NODE_117, &galaxian_bck_mixer_desc)

 *INFO: src/devices/sound/discrete.h: ...
 *
 ***********************************************************************
 *
 * DISCRETE_DAC_R1 - R1 ladder DAC with cap smoothing and external bias
 *
 *                             rBias
 * data&0x01 >--/\R[0]/\--+-----/\/\----< vBias
 * data&0x02 >--/\R[1]/\--|
 * data&0x04 >--/\R[2]/\--|
 * data&0x08 >--/\R[3]/\--|
 * data&0x10 >--/\R[4]/\--|
 * data&0x20 >--/\R[5]/\--|
 * data&0x40 >--/\R[6]/\--|
 * data&0x80 >--/\R[7]/\--+-------------+-----> Netlist node
 *                        |             |
 *                        Z            ---
 *                        Z rGnd       --- cFilter
 *                        |             |
 *                       gnd           gnd
 *
 * NOTES: rBias and vBias are used together.  If not needed they should
 *        be set to 0.  If used, they should both have valid values.
 *        rGnd and cFilter should be 0 if not needed.
 *        A resistor value should be properly set for each resistor
 *        up to the ladder length.  Remember 0 is a short circuit.
 *        The data node is bit mapped to the ladder. valid int 0-255.
 *        TTL logic 0 is actually 0.2V but 0V is used.  The other parts
 *        have a tolerance that more then makes up for this.
 *
 *  Declaration syntax
 *
 *     DISCRETE_DAC_R1(name of node,
 *                     data node (static value is useless),
 *                     vData static value (voltage when a bit is on ),
 *                     address of discrete_dac_r1_ladder structure)
 *
 *     discrete_dac_r1_ladder = {ladderLength, r{}, vBias, rBias, rGnd, cFilter}
 *
 *  Note: Resistors in the ladder that are set to 0, will be handled like they
 *        are out of circuit.  So the bit selecting them will have no effect
 *        on the DAC output voltage.
 *

//INFO: src/devices/sound/discrete.h: 3820

struct discrete_dac_r1_ladder
{
	int     ladderLength;       // 2 to DISC_LADDER_MAXRES.  1 would be useless.
	double  r[DISC_LADDER_MAXRES];  // Don't use 0 for valid resistors.  That is a short.
	double  vBias;          // Voltage Bias resistor is tied to (0 = not used)
	double  rBias;          // Additional resistor tied to vBias (0 = not used)
	double  rGnd;           // Resistor tied to ground (0 = not used)
	double  cFilter;        // Filtering cap (0 = not used)
};

//INFO:  src/mame/galaxian/galaxian_a.cpp: 178

static const discrete_dac_r1_ladder galaxian_bck_dac = 
{
	4,          // size of ladder
	{GAL_R18, GAL_R17, GAL_R16, GAL_R15, 0,0,0,0},
	4.4,        // 5V - diode junction (0.6V)
	GAL_R20,    // rBIAS
	GAL_R19,    // rGnd
	0           // no C
};

//INFO: src/mame/galaxian/galaxian_a.cpp: 381

DISCRETE_DAC_R1(NODE_100, GAL_INP_BG_DAC, TTL_OUT, &galaxian_bck_dac)
		NODE,     DATA,           VDATA,   LADDER

//INFO: src/devices/sound/discrete.h: 4534
  #define DISCRETE_DAC_R1(NODE,DATA,VDATA,LADDER)                         
	  DSC_SND_ENTRY( 
	      NODE, 
	      dst_dac_r1      , 
	      DSS_NODE        , 
	      2, 
	      DSE( static_cast<int>(DATA),NODE_NC ), 
	      DSE( DATA,VDATA ), 
	      LADDER, 
	      "DISCRETE_DAC_R1" 
	  ),

#define	  DSC_SND_ENTRY(
	      _nod, 
	      _class, 
	      _dss, 
	      _num, 
	      _iact, 
	      _iinit, 
	      _custom, _name) 
	      
	      { _nod,  
	      &discrete_node_factory< DISCRETE_CLASS_NAME(_class) >::create, 
	      _dss, 
	      _num, 
	      _iact, 
	      _iinit, 
	      _custom, 
	      _name, 
	      # _class }

------------------------------------------ FIRST 555 ----------------------------------------------------
DISCRETE_555_CC(NODE_105, 1, NODE_100, GAL_R21, GAL_C15, 0, 0, 0, &galaxian_bck_vco)

static const discrete_555_cc_desc galaxian_bck_vco =
{
	DISC_555_OUT_DC | DISC_555_OUT_CAP,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.7     // Q2 junction voltage
};

struct discrete_555_cc_desc
{
	int     options;         bit mapped options 
	double  v_pos;           B+ voltage of 555 
	double  v_cc_source;     Voltage of the Constant Current source 
	double  v_out_high;      High output voltage of 555 (Defaults to v_pos - 1.2V) 
	double  v_cc_junction;   The voltage drop of the Constant Current source transistor (0 if Op Amp) 
};
----------------------------------------------------------------------------------------------------------

More files:
src/devices/machine/netlist.h

src/devices/sound/...
"disc_sys.hxx"        discrete core modules and support functions 
"disc_wav.hxx"        Wave sources   - SINE/SQUARE/NOISE/etc 
"disc_mth.hxx"        Math Devices   - ADD/GAIN/etc               :170       <--- DST_DAC_R1  + disc_mth.h: 149
"disc_inp.hxx"        Input Devices  - INPUT/CONST/etc 
"disc_flt.hxx"        Filter Devices - RCF/HPF/LPF 
"disc_dev.hxx"        Popular Devices - NE555/etc
*
*/

