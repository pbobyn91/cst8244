/******************************************************************
 * FILE:des.h
 * AUTHOR: JOHNATHON CAMERON - PATRICK BOBYN
 * VERSION: 1.0
 *
 * Header file, defining the different states
 * encountered in the DES program.
 *
 * Input possibilities mapped to input messages
 * Output possibilities mapped to output messages
 *
 *
 * File also has defined structures for the incoming and outgoing
 * messages
 *
 ******************************************************************/
#ifndef DES_H_
#define DES_H_

#define RUNNING  1 /** RUNNING DEFINE FOR INFINITE LOOPS */

#define LEFT 0 /* Going down the LEFT path */
#define RIGHT 1 /* Going down the RIGHT path */
#define DEFAULT 2 /* Neither LEFT nor RIGHT selected */

/* Person structure ( SEND MSG) */
struct person { /* SENDING person structure to server to process */
	int id; /* person card ID */
	int weight; /* persons weight if scaled */
	int state; /* current state the person is in */
} typedef person_t;

/* Controller response structure (REPLY MSG) */
struct ctrl_response{ /* response structure from server, person included in response */
	person_t person; /* person structure being sent through IPC */
	int statusCode; /* status of response */
	char errMsg[128]; /*error message */
}typedef ctrl_resp_t;

typedef void *(*FState)();

#define NUM_STATES 13
typedef enum /* all states possible in progra, */
{
	ST_START = 0, 		/* 0: Start state                */
	ST_LS = 1,		/* 2: Left_Scan state            */
	ST_RS = 2,		/* 3: Right_Scan state           */
	ST_WS = 3,     	/* 4: Weighted state              */
	ST_LO = 4,		/* 5: Left_Open state            */
	ST_RO = 5,		/* 6: Right_Open state           */
	ST_LC = 6,		/* 7: Left_Close state           */
	ST_RC = 7,		/* 8: Right_Close state          */
	ST_GRL = 8,		/* 9: Guard_Right_Lock state     */
	ST_GRU = 9,		/* 10: Guard_Right_Unlock state  */
	ST_GLL = 10,		/* 11: Guard_Left_Lock state     */
	ST_GLU = 11,		/* 12: Guard_Left_Unlock state   */
	ST_EXIT = 12,		/* 13: Exit State				 */
} State;

#define NUM_INPUTS 12
typedef enum /* ALL Input enum possibilities mapped to input msgs */
{
	IN_LS = 0, 		/* LEFT SCAN 			*/
	IN_RS = 1, 		/* RIGHT SCAN 			*/
	IN_WS = 2, 		/* WEIGHT SCALE 		*/
	IN_LO = 3,		/* LEFT OPEN 			*/
	IN_RO = 4,		/* RIGHT OPEN 			*/
	IN_LC = 5,		/* LEFT CLOSE 			*/
	IN_RC = 6,		/* RIGHT CLOSE 			*/
	IN_GRL = 7,		/* GUARD RIGHT LOCKED 	*/
	IN_GRU = 8,		/* GUARD RIGHT UNLOCKED */
	IN_GLL = 9,		/* GUARD LEFT LOCKED 	*/
	IN_GLU = 10, 	/* GUARD LEFT UNLOCKED 	*/
	IN_EXIT = 11 	/* EXIT 				*/
} Input;

/* INPUTS ENUMS MAPPED TO INPUT MESSAGES */
const char *inMessage[NUM_INPUTS] = {
	"ls",  /* IN_LS    */
	"rs",  /* IN_RS    */
	"ws",  /* IN_WS    */
	"lo",  /* IN_LO    */
	"ro",  /* IN_RO    */
	"lc",  /* IN_LC    */
	"rc",  /* IN_RC    */
	"grl", /* IN_GRL   */
	"gru", /* IN_GRU   */
	"gll", /* IN_GLL   */
	"glu", /* IN_GLU   */
	"exit" /* IN_EXIT  */
};

#define NUM_OUTPUTS 12
typedef enum /* OUTPUT possibilities, mapped to messages */
{
	OUT_START = 0,  /* START                */
	OUT_LS_RS = 1,  /* LEFT-RIGHT SCAN 		*/
	OUT_WS = 2,     /* WEIGHT SCALE 		*/
	OUT_LO = 3,     /* LEFT OPEN 			*/
	OUT_RO = 4,     /* RIGHT OPEN 			*/
	OUT_LC = 5,     /* LEFT CLOSED 			*/
	OUT_RC = 6,     /* RIGHT CLOSED 		*/
	OUT_GRL = 7,    /* GUARD RIGHT LOCKED 	*/
	OUT_GRU = 8,    /* GUARD RIGHT UNLOCK 	*/
	OUT_GLL = 9,   /* GUARD LEFT LOCKED 	*/
	OUT_GLU = 10,   /* GUARD LEFT UNLOCK 	*/
	OUT_EXIT = 11,  /* EXIT 				*/
} Output;

/* OUTPUT ENUMS MAPPED TO MESSAGES */
const char *outMessage[NUM_OUTPUTS] = {
	"Controller PID: ", 					/* OUT_START - REQUIREMENT 	*/
	"Person scanned ID. ID = ", 			/* OUT_LS_RS - REQUIREMENT	*/
	"Person weighed. Weight = ",			/* OUT_WS 	 - REQUIREMENT	*/
	"Left door Open.",						/* OUT_LO 	 - DEBUG		*/
	"Right door Open.",						/* OUT_RO 	 - DEBUG		*/
	"Left door Closed.",					/* OUT_LC 	 - DEBUG		*/
	"Right door Closed.",					/* OUT_RC 	 - DEBUG		*/
	"Right door locked by Guard. ",			/* OUT_GRL 	 - DEBUG		*/
	"Right door unlocked by Guard. ",		/* OUT_GRU 	 - DEBUG		*/
	"Left door locked by Guard. ",			/* OUT_GLL 	 - DEBUG		*/
	"Left door unlocked by Guard. ",		/* OUT_GLU 	 - DEBUG		*/
	"Exiting Door Entry System..." 			/* OUT_EXIT  - REQUIREMENT	*/

};

typedef enum {
	IN_ERR_CONN = 0,
	IN_ERR_SND = 1,
	IN_ERR_RSP = 2,
	IN_ERR_USG = 3,
	ERR_SRVR_MSG = 4,
	DP_ERR_CHANNEL_CREATE = 5,
	DP_ERR_RCV = 6,
	DP_ERR_RPLY = 7,
	CTRL_ERR_USG = 8,
	CTRL_ERR_CHANNEL_CREATE = 9,
	CTRL_ERR_CONN = 10,
	CTRL_ERR_RCV = 11,
	CTRL_ERR_SND = 12

} Error;

#define NUM_ERROR 13
const char *errorMessages[NUM_ERROR] = {
	"ERROR: Could not Connect to Controller ./des-input ", /* 0 */
	"ERROR: Sending Message ./des-input", /* 1 */
	"ERROR: NULL Response from server ./des-input", /*2*/
	"ERROR: USAGE - ./des_input <controller-pid>",/*3*/
	"GENERAL-SERVER-ERROR: ",/*4*/
	"ERROR: Unable to create Channel ./des-display",/*5*/
	"ERROR: Message not Received ./des-display",/*6*/
	"ERROR: Replying to Message ./des-display",/*7*/
	"ERROR: USAGE - ./des-controller <display pid>",
	"ERROR: Unable to Create Channel ./des-controller",/*9*/
	"ERROR: Could not connect to des-display ./des-controller",/*10*/
	"ERROR: Message not received ./des-controller",/*11*/
	"ERROR: Could not send Message ./des-controller STATE: ",/*12*/



};


#endif /* DES_H_ */
