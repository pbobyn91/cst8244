#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <unistd.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <ctype.h>
#include <errno.h>
#include "des.h"
/****************************************
 * Function Declaration
 ****************************************/
void *st_ls(); 	 /*	 LEFT SCAN 			*/
void *st_lo();	 /*  LEFT OPEN  		*/
void *st_lc();	 /*  LEFT CLOSE 		*/
void *st_rs();	 /*  RIGHT SCAN 		*/
void *st_ro();	 /*  RIGHT OPEN 		*/
void *st_rc();	 /*  RIGHT CLOSE 		*/
void *st_grl();	 /*  GUARD RIGHT LOCK 	*/
void *st_gru();	 /*  GUARD RIGHT UNLOCK */
void *st_gll();	 /*  GUARD LEFT LOCK 	*/
void *st_glu();	 /*  GUARD LEFT UNLOCK	*/
void *st_ws();	 /*  WEIGHT SCALE 		*/
void *st_ready();/*  READY 				*/
void *st_exit(); /*  EXIT 				*/

/****************************************
 * GLOBAL VARIABLES
 ****************************************/
ctrl_resp_t controller_response; /* response structure */
person_t person; /* person structure */
FState f_state = st_ready; /* Initially start at ready state  TODO pointer to function*/


int main(int argc, char* argv[]) {

	pid_t dpid;
	int coid,chid,rcvid; /* connection id,Channel,return from MsgReceive message */

	if(argc!=2){/* Validate correct amount of arguments */
		printf("USAGE: ./des_controller <display-pid>\n");
		exit(EXIT_FAILURE);
	}

	dpid = atoi(argv[1]); /* Get display pid from cmd line arguments */

	/* PHASE I: Create Channel */
	if((chid = ChannelCreate(0)) == -1){
		printf("ERROR: Could not create Channel");
		exit(EXIT_FAILURE);
	}

	/* Connect to display */
	if((coid = ConnectAttach(ND_LOCAL_NODE,dpid,1,_NTO_SIDE_CHANNEL,0)) == -1){
		printf("ERROR: Could not connect to des_display");
		exit(EXIT_FAILURE);
	}

	while(RUNNING){

		if((rcvid = MsgReceive(chid,&person,sizeof(person),NULL)) == -1){ /* receive message from client */
				printf("ERROR : Message not received\n"); /* ON FAIL */
				exit(EXIT_FAILURE);
		}

		if(person.state == ST_EXIT)
			st_exit();
			
		
		/* which function to run? */
		f_state = (FState)(*f_state)();
		
		controller_response.statusCode = EOK;
		MsgReply(rcvid, EOK, &controller_response, sizeof(controller_response));
		
		if (person.state == ST_STOP)
			break;
	}
	
	ChannelDestroy(chid);
	ConnectDetach(coid);
	return EXIT_SUCCESS;
}

void *st_ready() {		/* READY STATE */
	if (person.state == ST_LS) {		/* LEFT STATE */
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_ls;
	} else if (person.state == ST_RS) {	/* RIGHT STATE */
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_rs;
	}
	return st_ready;
}

void *st_ls() {		/* LEFT SCAN */
	if (person.state == ST_GLU) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_glu;
	}
	return st_ls;
}

void *st_glu(){	 /*  GUARD LEFT UNLOCK	*/
	if (person.state == ST_LO) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_lo;
	}
	return st_glu;
}

void *st_lo() {	 /*  LEFT OPEN  		*/
	if (person.state == ST_WS) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_ws;
	}
	return st_lo;
}

void *st_rs() {	 	/* RIGHT SCAN */
	if (person.state == ST_GRU) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
	return st_gru;
	}
	return st_rs;
}

void *st_gru(){	 /*  GUARD RIGHT UNLOCK */
	if (person.state == ST_RO) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_ro;
	}
	return st_gru;
}

void *st_ro() {	 /*  RIGHT OPEN 		*/
	if (person.state == ST_WS){
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_ws;
	}
	return st_ro;
}

void *st_ws(){	 /*  WEIGHT SCALE 		*/
	if (person.state == ST_LC) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_lc;
	}
	if (person.state == ST_RC) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_rc;
	}
	return st_ws;
}

void *st_lc() {	 /*  LEFT CLOSE 		*/
	if (person.state == ST_GLL){
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_gll;
	}
	return st_lc;
}

void *st_rc() {	 /*  RIGHT CLOSE 		*/
	if (person.state == ST_GRL){
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_grl;
	}
	return st_rc;
}

void *st_grl(){	 /*  GUARD RIGHT LOCK 	*/
	if (person.state == ST_READY){
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_ready;
	}
	return st_grl;
}

void *st_gll(){	 /*  GUARD LEFT LOCK 	*/
	if (person.state == ST_READY){
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("ERROR: Could not send message");
			exit(EXIT_FAILURE);
		}
		return st_ready;
	}
	return st_gll;
}

void *st_exit(){  /*  EXIT */
	person.state = ST_EXIT;
	if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
		printf("ERROR: Could not send message");
		exit(EXIT_FAILURE);
	}
	person.state = ST_END;
	return st_exit;
}


