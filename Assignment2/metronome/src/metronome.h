/*
 * metronome.h
 *
 */

#ifndef SRC_METRONOME_H_
#define SRC_METRONOME_H_

/*****************
 * LIBS
 *****************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <math.h>
#include <sys/types.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

/*****************
 * PULSE CODES MIN 0 MAX 127
 *****************/
#define METRO_PULSE_CODE _PULSE_CODE_MINAVAIL         //0
#define PAUSE_PULSE_CODE (_PULSE_CODE_MINAVAIL +1) 	  //1
#define START_PULSE_CODE (_PULSE_CODE_MINAVAIL +2)    //2
#define STOP_PULSE_CODE  (_PULSE_CODE_MINAVAIL +3)    //3
#define QUIT_PULSE_CODE  (_PULSE_CODE_MINAVAIL +4)    //4
#define SET_PULSE_CODE   (_PULSE_CODE_MINAVAIL +5)    //5

/*********************
 * OVERRIDE iofunc_att_t
 *********************/
struct ioattr_t;
#define IOFUNC_ATTR_T struct ioattr_t
#include <sys/iofunc.h>
#include <sys/dispatch.h>
typedef struct ioattr_t {
	iofunc_attr_t attr;
	int device;
} ioattr_t;

/************************
 * CONNECTIONS | PATH
 ************************/
#define METRO_ATTACH  "metronome"
#define DEVICES 2 /* Device amount */

char *devnames[DEVICES] = {
		"/dev/local/metronome",
		"/dev/local/metronome-help"
};

/***********************
 * Timer Status
 **********************/
#define START 0
#define STOPPED 1
#define PAUSED 2

/************************
 * Additional DEFINES
 ***********************/
#define PULSE 0
#define ERROR -1

/***********************
 * STRUCTS | UNIONS
 **********************/
typedef union {
	struct _pulse pulse;
	char msg[255];
}my_message_t;

struct DataTableRow{
	int tst;
	int tsb;
	int intervals;
	char pattern[16];
};

struct DataTableRow t[] = {
		{2, 4, 4, "|1&2&"},
		{3, 4, 6, "|1&2&3&"},
		{4, 4, 8, "|1&2&3&4&"},
		{5, 4, 10, "|1&2&3&4-5-"},
		{3, 8, 6, "|1-2-3-"},
		{6, 8, 6, "|1&a2&a"},
		{9, 8, 9, "|1&a2&a3&a"},
		{12, 8, 12, "|1&a2&a3&a4&a"}
};

struct Timer_Properties{
	double bps; /* Beats per second */
	double measure; /* Beat per measure */
	double interval; /* Sec per interval */
	double nano_sec; /* nano seconds value */

}typedef timer_props_t;

struct Metro_Properties {
	timer_props_t timer_props;
	int bpm;
	int tst;
	int tsb;
}typedef metro_props_t;



#endif /* SRC_METRONOME_H_ */
