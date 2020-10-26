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
//FState f_state = st_ready; /* Initially start at ready state  TODO pointer to function*/


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

		//if(person.state == ST_EXIT)
			//st_exit();

	}

}
