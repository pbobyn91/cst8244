/*
 * metronome.h
 *
 */

#ifndef SRC_METRONOME_H_
#define SRC_METRONOME_H_


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
#include <sys/iofunc.h>
#include <sys/dispatch.h>



#define METRO_PULSE_CODE _PULSE_CODE_MINAVAIL //0
#define PAUSE (_PULSE_CODE_MINAVAIL +1) //1
#define QUIT (_PULSE_CODE_MINAVAIL +2) // 2
#define METRO_ATTACH  "metronome"
#define METRO_PATH "/dev/local/metronome"


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

struct Properties {
	int bpm;
	int tst;
	int tsb;
}typedef metro_props_t;



#endif /* SRC_METRONOME_H_ */
