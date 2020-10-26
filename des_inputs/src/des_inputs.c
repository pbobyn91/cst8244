/**************************************************
 * File: des_inputs.c
 * Version: 1.0
 * Author: Johnathon Cameron - Patrick Bobyn
 *
 * Description:
 *  - This file is the client function of the program
 *  - It starts by connecting to the controller
 *  - and send a person structure to it.
 *
 *  - It controls and sets the current state of a person
 *  in the DES application.
 *
 *  Communication: des_inputs <-> des_controller <-> des_display
 *
 **************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <float.h>
#include <limits.h>
#include <unistd.h>
#include "../../des_controller/src/des.h"
/**********************
 * Function declaration
 **********************/
void input_event_prompt(char * input, person_t* person); /* Used to manage state transitions */
void prompt_for_id(person_t* person); /* Used to prompt user for ID */
void prompt_for_weight(person_t* person); /*Used to prompt user for weight */
/*************************************************
 * Function: main
 * Parameters: int, char*
 * Return: int
 *
 * Description: main function of the des_inputs
 * project. des_inputs acts as a client
 * and communicates with the controller
 *
 * the main function will used input_event_prompt
 * to manage and set the current state a person
 * is in while going through the DES system.
 **************************************************/
int main(int argc, char* argv[]){

	pid_t cpid; /* controller pid storage var */
	int coid; /* return from connect attach  */
	person_t person; /* Person structure defined in des.h */
	ctrl_resp_t controller_response; /* response from the controller, struct defined in des.h */

	/* Check if correct number of command line arguments */
	if(argc !=2){
		printf("USAGE: ./des_inputs <controller-pid> \n");
		exit(EXIT_FAILURE);
	}

	cpid = atoi(argv[1]); /* get controllers pid from command line arguments */

	/* Connect to controller */
	if((coid = ConnectAttach(ND_LOCAL_NODE,cpid,1,_NTO_SIDE_CHANNEL,0)) == -1){ /* ON FAIL */
		printf("ERROR: Could not connect to controller\n");
		exit(EXIT_FAILURE);
	}


	while(RUNNING){ /* Infinite Loop */
		char input[10]=""; /* no valid command is more then 4 chars,but will give more room... NOTE redeclared after every loop */
		printf("\n Enter the event type (ls = left scan, rs = right scan, ws = weight scale, lo = left open, \n"
				"ro = right open, lc = left closed, rc = right closed, gru = guard right unlock, grl = guard right lock, \n"
				"gll = guard left lock, glu = guard left unlock, exit = exit programs) \n");

		scanf("%s",&input);
		printf("\n");

		input_event_prompt(input,&person); /*input*/

		/* PHASE II send message to controller */
		if(MsgSend(coid,&person,sizeof(person),&controller_response,sizeof(controller_response)) == -1){
			printf("ERROR: Sending Message");
			exit(EXIT_FAILURE);
		}

		/* Check if message is null ( null as in no length) */
		if(sizeof(controller_response) == 0){
			printf("ERROR: Null response from server");
			exit(EXIT_FAILURE);
		}
		/* Make sure no error messages */
		if(controller_response.statusCode !=EOK){ /* DID server generate error?*/
			printf("ERROR:Message from server: %s", controller_response.errMsg);
		}
	}

	/* Disconnect from server */
	ConnectDetach(coid);
	return EXIT_SUCCESS;

}
/**************************************
 * Function: input_event_prompt
 * Parameters: char *, person_t
 * Return: void
 * Description:
 * 	Function used to transitions through
 * 	program states. Upon user input
 * 	the persons transition/states
 * 	will change.
 **************************************/
void input_event_prompt(char * input, person_t* person){

	if(strcmp(input,inMessage[IN_LS])){
		person->state = ST_LS; /* SET LEFT SCAN STATE */
		prompt_for_id(person);/*prompt*/
	}else if(strcmp(input,inMessage[IN_RS])){
		person->state = ST_RS;/* SET RIGHT SCAN STATE */
		prompt_for_id(person);/*prompt*/
	}else if(strcmp(input,inMessage[IN_WS])){
		person->state = ST_WS; /* SET WEIGHT SCALE STATE */
		prompt_for_weight(person);/*prompt*/
	}else if(strcmp(input,inMessage[IN_LO]))
		person->state = ST_LO; /* SET LEFT OPEN STATE */
	else if(strcmp(input,inMessage[IN_RO]))
		person->state = ST_RO; /* SET RIGHT OPEN STATE */
	else if(strcmp(input,inMessage[IN_LC]))
		person->state = ST_LC; /* SET LEFT CLOSE STATE */
	else if(strcmp(input,inMessage[IN_RC]))
		person->state = ST_RC; /* SET RIGHT CLOSE STATE */
	else if(strcmp(input,inMessage[IN_GRL]))
		person->state = ST_GRL; /* SET GUARD RIGHT LOCK */
	else if(strcmp(input,inMessage[IN_GRU]))
		person->state = ST_GRU; /* SET GUARD RIGHT UNLOCK */
	else if(strcmp(input,inMessage[IN_GLL]))
		person->state = ST_GLL; /* SET GUARD LEFT LOCK */
	else if(strcmp(input,inMessage[IN_GLU]))
		person->state = ST_GLU; /* SET GUARD LEFT UNLOCK */
	else if(strcmp(input,inMessage[IN_EXIT]))
		person->state = ST_EXIT; /* SET EXIT STATE */

}
/********************
 * Function: prompt_for_id
 * Parameters: person_t
 * Description:
 * Simple function used to prompt user to enter ID
 ********************/
void prompt_for_id(person_t* person){
	printf("Enter ID:\n"); /* enter ID */
	scanf("%d", &person->id);/* retrieve ID */
	printf("\n");
}
/********************
 * Function: prompt_for_id
 * Parameters: person_t
 * Description:
 * Simple function used to prompt user to enter weight
 ********************/
void prompt_for_weight(person_t* person){
	printf("Please Enter your Weight");
	scanf("%d",&person->weight);
	printf("\n");
}



