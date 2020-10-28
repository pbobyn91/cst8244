#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <float.h>
#include <limits.h>
#include <unistd.h>
#include "../../des_controller/src/des.h"

/***********************
 *  Function Declaration
 ***********************/
void display_current_state(person_t* person); /* function to display the current state of a person */

int main(void) {

	person_t pmsg; /* persons message */
	ctrl_resp_t response; /* response message from server */

	int chid,rcvid; /* local variables */

	/***************
	 * PHASE I
	 ***************/
	if((chid = ChannelCreate(0)) == -1) /* Create Channel */
	{ /* ON FAIL */
		perror("ERROR: Failed to create channel\n");
		exit(EXIT_FAILURE);
	}

	print("Display PID : %d\n", getpid()); /* display the Process ID of the server */

	/*****************
	 * PHASE II
	 *****************/

	while(RUNNING){ /* normal behavior of a server infinite loop */

		if((rcvid = MsgReceive(chid,&pmsg,sizeof(pmsg),NULL)) == -1){ /* receive message from client */
			printf("ERROR : Message not received\n"); /* ON FAIL */
			exit(EXIT_FAILURE);
		}

		display_current_state(&pmsg); /* display the current state of the person */
		MsgReply(rcvid, EOK, &response, sizeof(response)); /* reply EOK back to controller */

	}

	if(response.statusCode != EOK){
		printf("ERROR: %s", response.errMsg);
	}

	ChannelDestroy(chid);
	return EXIT_SUCCESS;

}
void display_current_state(person_t* person){

	int current_state;
	/* DISPLAY current State messages from outMessage */
	switch(msg->state){
		case ST_LS: /* Left scan*/
		case ST_RS:/*Right scan*/
			print("%s %d \n", outMessage[OUT_SCAN_MSG], person->id); /* OUT message with person ID card number*/
			break;

		case ST_WS: /* Weigh scale state */
			printf("%s , %d \n", outMessage[OUT_WEIGHT_MSG], person->weight); /* OUT message with person weight*/
			break;
		case ST_LO:
			printf("ST_LO\n");
			break;
		case ST_RO:
			printf("ST_RO\n");
			break;
		case ST_LC:
			printf("ST_LC\n");
			break;
		case ST_RC:
			printf("ST_RC\n");
			break;
		case ST_GRL:
			printf("ST_GRL\n");
			break;
		case ST_GRU:
			printf("ST_GRU\n");
			break;
		case ST_GLL:
			printf("ST_GLL\n");
			break;
		case ST_GLU:
			printf("ST_GLU\n");
			break;
		case ST_EXIT:
			printf("%s \n", outMessage[OUT_EXIT_MSG]);
			printf("%s \n",outMessage[OUT_STOP_MSG]);
			break;
		default:
			printf("ERROR: Invalid input");
			break;
	}
}
