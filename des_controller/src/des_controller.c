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
void *st_start();/*  Start 				*/
void *st_exit(); /*  EXIT 				*/

void reset();

/****************************************
 * GLOBAL VARIABLES
 ****************************************/
ctrl_resp_t controller_response; /* response structure */
person_t person; /* person structure */
int coid,chid,rcvid; /* connection id,Channel,return from MsgReceive message */
FState f_state = st_start; /* Initially start at ready state  TODO pointer to function*/


int main(int argc, char* argv[]) {

	pid_t dpid;


	if(argc!=2){/* Validate correct amount of arguments */
		printf("%s\n", errorMessages[CTRL_ERR_USG]);
		exit(EXIT_FAILURE);
	}

	dpid = atoi(argv[1]); /* Get display pid from cmd line arguments */

	/* PHASE I: Create Channel */
	if((chid = ChannelCreate(0)) == -1){
		printf("%s\n",errorMessages[CTRL_ERR_CHANNEL_CREATE]);
		exit(EXIT_FAILURE);
	}

	/* Connect to display */
	if((coid = ConnectAttach(ND_LOCAL_NODE,dpid,1,_NTO_SIDE_CHANNEL,0)) == -1){
		printf("%s\n",errorMessages[CTRL_ERR_CONN]);
		exit(EXIT_FAILURE);
	}

	printf("%s %d\n",outMessage[OUT_START],getpid());
	reset();

	while(RUNNING){

		if((rcvid = MsgReceive(chid,&person,sizeof(person),NULL)) == -1){ /* receive message from client */
				printf("%s\n",errorMessages[CTRL_ERR_RCV]); /* ON FAIL */
				exit(EXIT_FAILURE);
		}
		
		if(person.state == ST_EXIT) /* if state is exit (user input) then terminate controller ST_EXIT -> ST_END */
			f_state = st_exit;
		else
			f_state = (FState)(*f_state)();/* which function to run? */
		
		controller_response.statusCode = EOK;
		MsgReply(rcvid, EOK, &controller_response, sizeof(controller_response));
		


		if (person.state == ST_EXIT)
			break;

	}
	
	ChannelDestroy(chid);
	ConnectDetach(coid);
	return EXIT_SUCCESS;
}

void *st_start() {		/* START STATE */
	if (person.state == ST_LS) {		/* LEFT STATE */
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s LS - st_start()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_ls;
	} else if (person.state == ST_RS) {	/* RIGHT STATE */
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s RS - st_start()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_rs;
	}
	return st_start;
}

void *st_ls() {		/* LEFT SCAN */
	if (person.state == ST_GLU) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s GLU - st_ls()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_glu;
	}
	return st_ls;
}

void *st_glu(){	 /*  GUARD LEFT UNLOCK	*/
	if (person.state == ST_LO) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s LO - st_glu()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_lo;
	}
	return st_glu;
}

void *st_lo() {	 /*  LEFT OPEN  		*/
	if (person.state == ST_WS){ /* IF weighing, then person entered from the right*/
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s WS -st_lo()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_ws;
	}else if(person.state == ST_LC){ /* if right closed then person entered from the left */
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
				printf("%s LC - st_lo()\n",errorMessages[CTRL_ERR_SND]);
				exit(EXIT_FAILURE);
		}
		return st_lc;
	}
	return st_lo;
}

void *st_rs() {	 	/* RIGHT SCAN */
	if (person.state == ST_GRU) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s GRU - st_rs()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
	return st_gru;
	}
	return st_rs;
}

void *st_gru(){	 /*  GUARD RIGHT UNLOCK */
	if (person.state == ST_RO) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s RO - st_gru()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_ro;
	}
	return st_gru;
}

void *st_ro() {	 /*  RIGHT OPEN 		*/
	if (person.state == ST_WS){ /* IF weighing, then person entered from the right*/
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s WS - st_ro()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_ws;
	}else if(person.state == ST_RC){ /* if right closed then person entered from the left */
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
					printf("%s RC- st_ro()\n",errorMessages[CTRL_ERR_SND]);
					exit(EXIT_FAILURE);
		}
		return st_rc;
	}
	return st_ro;
}

void *st_ws(){	 /*  WEIGHT SCALE 		*/
	if (person.state == ST_LC) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s LC - st_ws()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_lc;
	}
	if (person.state == ST_RC) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s RC - st_ws()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_rc;
	}
	return st_ws;
}

void *st_lc() {	 /*  LEFT CLOSE 		*/
	if (person.state == ST_GLL){
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s GLL - st_lc()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_gll;
	}
	return st_lc;
}

void *st_rc() {	 /*  RIGHT CLOSE 		*/
	if (person.state == ST_GRL){
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s GRL - st_rc() \n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_grl;
	}
	return st_rc;
}

void *st_grl(){	 /*  GUARD RIGHT LOCK 	*/
	if (person.state == ST_GLU){
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s GLU - st_grl()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_glu;
	}
	if (person.state == ST_RS) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s RS - st_grl()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_rs;
	}
	if (person.state == ST_LS) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s LS - st_grl()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_ls;
	}

	return st_grl;
}

void *st_gll(){	 /*  GUARD LEFT LOCK 	*/
	if (person.state == ST_GRU){
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s GRU - st_gll()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		return st_gru;
	}else if(person.state == ST_RS){
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			printf("%s RS - st_gll()\n",errorMessages[CTRL_ERR_SND]);
			exit(EXIT_FAILURE);
		}
		if (person.state == ST_LS) {
			if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
				printf("%s LS - st_gll()\n",errorMessages[CTRL_ERR_SND]);
				exit(EXIT_FAILURE);
			}
			return st_ls;
		}

		return st_rs;
	}
	return st_gll; /* person is exiting reset structure in main */
}

void *st_exit(){  /*  EXIT */

	if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
		printf("%s EXIT - st_exit()\n",errorMessages[CTRL_ERR_SND]);
		exit(EXIT_FAILURE);
	}
	//person.state = ST_END;
	sleep(5);
	return st_exit;
}

void reset(){
	person.id = 0;
	person.weight = 0;
	person.state = ST_START;
}


