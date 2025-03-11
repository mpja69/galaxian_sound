// =========== Comoponents =============

// ----------- Background ------------
// Timer 8R
#define R22	  RES_K(100)	    // R1
#define R23	  RES_K(470)	    // R2
#define R24	  RES_K(10)
#define C17	  CAP_U(0.01)	    

// Timer 8S
#define R25	  RES_K(100)	    // R1
#define R26	  RES_K(330)	    // R2
#define R27	  RES_K(10)
#define C18	  CAP_U(0.01)

// Timer 8T
#define R28	  RES_K(100)	    // R1
#define R29	  RES_K(220)	    // R2
#define R30	  RES_K(10)
#define C19	  CAP_U(0.01)



// ------------- Fire circuit --------------
// Timer 7S
#define R44	  RES_K(10)	    // R1
#define R45	  RES_K(22)	    // R2
//#define RXX	  RES_K(10)	
#define C27	  CAP_U(0.01)

// The RC discharge for controlling the fire amplitude decay 
#define R41	RES_K(100)
#define C25	CAP_U(1)	    

// The RC charge and discharge. (Controlling CV)
#define R47	RES_K(2.2)
#define R48	RES_K(2.2)
#define C28	CAP_U(47)	    

// For the voltage divider between NOISE and C28 -> 
#define R46	RES_K(10)

// ----------- Vcc on the board ------------
#define V_CC (5.0)

// shared definitions for discrete components
// From mame/src/lib/netlist/devices/net_lib.h
#define RES_R(res)	(res)
#define RES_K(res)	((res) * 1e3)
#define RES_M(res)	((res) * 1e6)
// Capacitors
#define CAP_U(cap)	((cap) * 1e-6)
#define CAP_N(cap)	((cap) * 1e-9)
#define CAP_P(cap)	((cap) * 1e-12)

// Helper functions
// From mame/src/devices/machine/rescap.h
//  Vin --/\r1/\-- Out --/\r2/\-- Gnd
#define RES_VOLTAGE_DIVIDER(r1, r2) ( (r2) / ((r1) + (r2)) )

#define RES_2_PARALLEL(r1, r2)          (((r1) * (r2)) / ((r1) + (r2)))
#define RES_3_PARALLEL(r1, r2, r3)      (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3)))
#define RES_4_PARALLEL(r1, r2, r3, r4)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4)))
#define RES_5_PARALLEL(r1, r2, r3, r4, r5)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4) + 1.0 / (r5)))
#define RES_6_PARALLEL(r1, r2, r3, r4, r5, r6)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4) + 1.0 / (r5) + 1.0 / (r6)))

#define RES_2_SERIAL(r1,r2)             ((r1)+(r2))

