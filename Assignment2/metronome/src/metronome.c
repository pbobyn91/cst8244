/******************************************************************************************************************
 *  FILE: metronome.c
 *  Version: 1.0
 *  Authors: Johnathon Cameron - Patrick Bobyn
 *
 *	Class: CST8244 - Real Time with QNX - Neutrino
 *	Professor: -G
 *	Assignment: Assignment 2
 *
 *	Description:
 * - This is a program meant to implement a metronome
 * 	 using a Device driven Resource Manager.
 *
 * - USAGE Commands:
 * 			Write:
 * 				- set: echo <cmd> <bpm> <tst> <tsb> > /dev/local/metrnome
 * 				 Description: set a new intervals for the metronome by entering the beats ber minute (bpm), Time-signatur-top,
 * 					   		  time-signature-bottom. Depending on which Time signatures selected, they are mapped to specific(metronom.h)
 * 					   		  print out patterns.
 * 				- pause: echo pause <[1-9] > /dev/local/metronome
 * 				  Description: The metronome device will pause for a specified amount of seconds and
 * 				 			   continue where it left off. Note: Metronome can only stop from 1 to 9 seconds
 *
 * 				- stop: echo stop > /dev/local/metronome
 * 				  Description: Stops the metronome. Can only stop the metronome if the metronome is in pause or start state.
 *
 * 				-start: echo start > /dev/local/metronome
 * 				 Description: Can only start when the metronome has been stopped. Continues from where it left off
 * 				 	          Allocating a new timer with the calculated intervals
 *
 * 			Read:
 * 				cat /dev/local/metronome - Displays the current BPM, TST,TSB, sec per interval, nano seconds
 * 				cat /dev/local/metrnome-help - Displays how to use the metronome
 *
 *****************************************************************************************************************/
#include "metronome.h"
/***********************
 * GLOBAL Variables
 ***********************/
name_attach_t * attach; /* Namespace connection */
Metronome_t Metronome;
int server_coid;
char data[255];

/*************************************
 * Function: Main
 * Description: Main function for the program
 * This function will connect to the Resource
 * manager and implement
 *
 *
 * Status : Complete
 ************************************/
int main(int argc, char *argv[]) {
	dispatch_t * dpp; /* DISPATCH */
	resmgr_io_funcs_t io_funcs;/* POSIX level IO function (read,write) */
	resmgr_connect_funcs_t conn_funcs; /* POSIX level Connection Functions (open) */
	ioattr_t ioattr[DEVICES];/* I/O attribute structure, attributes associated to Resource Manager */
	pthread_attr_t thread_attrib; /* Thread Attributes */
	dispatch_context_t * ctp;/* Returned Dispatch Context from dispatch_context_alloc()*/

	int id, i;

	int server_coid; /* Thread ID */

	if (argc != 4) { /* Check program USAGE. MUST have 4 params NOTE: including ./metronome */
		printf("ERROR: You must provide all required arguments.\n");
		usage();
		exit(EXIT_FAILURE); /* termination */
	}

	/* Pointing to overriden functions */
	iofunc_funcs_t metro_ocb_funcs = {
	_IOFUNC_NFUNCS, metro_ocb_calloc, metro_ocb_free, };

	iofunc_mount_t metro_mount = { 0, 0, 0, 0, &metro_ocb_funcs }; /* mount functions to ocb */

	/* Process Command Line Arguments - Metronome Properties*/
	Metronome.m_props.bpm = atoi(argv[1]);
	Metronome.m_props.tst = atoi(argv[2]);
	Metronome.m_props.tsb = atoi(argv[3]);

	/* Allocating dispatch handler */
	if ((dpp = dispatch_create()) == NULL) {
		fprintf(stderr, "%s:  Unable to allocate dispatch context.\n", argv[0]);
		return (EXIT_FAILURE); /* termination */
	}

	/* Init default POSIX-layer function tables */
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &conn_funcs, _RESMGR_IO_NFUNCS,
			&io_funcs);
	conn_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	for (i = 0; i < DEVICES; i++) {
		/* Initialize attribute structure with Resource Manager */
		iofunc_attr_init(&ioattr[i].attr, S_IFCHR | 0666, NULL, NULL);
		ioattr[i].device = i;
		ioattr[i].attr.mount = &metro_mount; /* mount ocb overriden funcs*/

		/* Attach path to pathname space - Resource Manager */
		if ((id = resmgr_attach(dpp, NULL, devnames[i], _FTYPE_ANY, 0,
				&conn_funcs, &io_funcs, &ioattr[i])) == ERROR) {
			fprintf(stderr, "%s:  Unable to attach name.\n", argv[0]);
			return (EXIT_FAILURE); /* On fail Terminate */
		}
	}

	/* returns dispatch context */
	ctp = dispatch_context_alloc(dpp);

	/* Initialize Thread Attribute and Create Thread */
	pthread_attr_init(&thread_attrib);
	pthread_create(NULL, &thread_attrib, &metronome_thread, &Metronome);

	/* While true */
	while (1) {
			if((ctp = dispatch_block(ctp))){
				dispatch_handler(ctp);
			}else
				printf("ERROR \n");

	}

	pthread_attr_destroy(&thread_attrib); /* destroy thread attributes */
	name_detach(attach, 0); /* detach from namespace, terminating connection */
	name_close(server_coid); /* close server connection from name_open */
	return EXIT_SUCCESS; /* Successful TERMINATION */

}
/******************************************
 * Function: usage
 * Description: User friendly print out
 * statement showing how the program/ Device
 * is used.
 *
 * Status:  Complete
 *******************************************/
void usage() {
	printf(
			"Command: ./metronome <beats/minute> <time-signature-top> <time-signature-bottom>\n");
}

/*******************************************
 * Function: io_read
 * Description: POSIX level
 * I/O funnction read adaptation
 * for our mentronome
 *
 * Status: Complete
 *******************************************/
int io_read(resmgr_context_t *ctp, io_read_t *msg, metro_ocb_t *mocb) {

	int index; /* Search index */
	int nb;

	if (data == NULL)
		return 0;

	if (mocb->ocb.attr->device == METRONOME_HELP) { /* Check if device is /dev/local/metronome-help */
		sprintf(data,
				"Metronome Resource Manager (Resmgr)\n\nUsage: metronome <bpm> <ts-top> <ts-bottom>\n\nAPI:\n pause[1-9]\t\t\t-pause the metronome for 1-9 seconds\n quit:\t\t\t\t- quit the metronome\n set <bpm> <ts-top> <ts-bottom>\t- set the metronome to <bpm> ts-top/ts-bottom\n start\t\t\t\t- start the metronome from stopped state\n stop\t\t\t\t- stop the metronome; use 'start' to resume\n");
	} else { /* else perform /dev/local/metronome tasks */

		/* Search for current properties in Metronome Property Table */
		index = search_idx_table(&Metronome);

		/* TIMER PROPERTIES Calculated and set ALREADY SET, no point on repeating code bps,measure,interval*/

		sprintf(data,
				"[metronome: %d beats/min, time signature: %d/%d, sec-per-interval: %.2f, nanoSecs: %.0lf]\n",
				Metronome.m_props.bpm, t[index].tst,
				t[index].tsb, Metronome.t_props.interval,
				Metronome.t_props.nano_sec);
	}
	nb = strlen(data);

	//test to see if we have already sent the whole message.
	if (mocb->ocb.offset == nb)
		return 0;

	//We will return which ever is smaller the size of our data or the size of the buffer
	nb = min(nb, msg->i.nbytes);

	//Set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nb);

	//Copy data into reply buffer.
	SETIOV(ctp->iov, data, nb);

	//update offset into our data used to determine start position for next read.
	mocb->ocb.offset += nb;

	//If we are going to send any bytes update the access time for this resource.
	if (nb > 0)
		mocb->ocb.flags |= IOFUNC_ATTR_ATIME;

	return (_RESMGR_NPARTS(1));
}

/***************************************
 * Function : io_write
 * Description: POSIX level
 * I/O funnction for writing adaptation
 * for our mentronome
 *
 *
 * Status : Complete
 ****************************************/
int io_write(resmgr_context_t *ctp, io_write_t *msg, metro_ocb_t *mocb) {

	int nb = 0;


	if (mocb->ocb.attr->device == METRONOME_HELP) { /* Cannot write to device /dev/local/metronome-help */
		printf("\nError: Cannot Write to device /dev/local/metronome-help\n");
		nb = msg->i.nbytes;
		_IO_SET_WRITE_NBYTES(ctp, nb);
		return (_RESMGR_NPARTS(0));
	}

		if (msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg))) {

			/* have all the data */
			char *buf; /* current buffer */
			char * pause_msg; /*use if the message is pause */
			char * set_msg; /* if set command is used */
			int i, small_integer = 0; /*integer value for the pause */
			buf = (char *) (msg + 1); /* add '/0' */

			if (strstr(buf, "pause") != NULL) { /* Check if client sent Pause request */
				for (i = 0; i < 2; i++) { /* check for both <cmd> <value> */
					pause_msg = strsep(&buf, " "); /* split the current string */
				}
				small_integer = atoi(pause_msg); /* convert the value to int */
				if (small_integer >= 1 && small_integer <= 9) {/* validation*/
					//FIXME :: replace getprio() with SchedGet()
					MsgSendPulse(server_coid, SchedGet(0, 0, NULL), /* send pulse Pause pulse code with value in seconds to pause */
					PAUSE_PULSE_CODE, small_integer);
				} else { /* validation error message */
					printf("Integer is not between 1 and 9.\n");
				}

			} else if (strstr(buf, "quit") != NULL) { /* Check if request is to quit */
				MsgSendPulse(server_coid, SchedGet(0, 0, NULL),/* Send QUIT PULSE */
				QUIT_PULSE_CODE, small_integer);
			} else if (strstr(buf, "start") != NULL) { /* Check for START request */
				MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
						START_PULSE_CODE, small_integer); /* Send Start pulse code */
			} else if (strstr(buf, "stop") != NULL) { /* Check for Stop request */
				MsgSendPulse(server_coid, SchedGet(0, 0, NULL), STOP_PULSE_CODE,
						small_integer); /* Send Stop pulse code */

			} else if (strstr(buf, "set") != NULL) { /* Check for set request */
				for (i = 0; i < 4; i++) { /*split string */
					set_msg = strsep(&buf, " "); /* start splitting string  <bpm> <tst> <tsb>*/

					if (i == 1) { /* First string BPM */
						Metronome.m_props.bpm = atoi(set_msg); /* set to new bpm */
					} else if (i == 2) {/* SECOND STRING TST */
						Metronome.m_props.tst = atoi(set_msg);/* set to new tst */
					} else if (i == 3) {/* THIRD STRING TSB */
						Metronome.m_props.tsb = atoi(set_msg);/* set to new tsb */
					}
				}

				MsgSendPulse(server_coid, SchedGet(0, 0, NULL), SET_PULSE_CODE,
						small_integer); /* Send Set pulse, value doesn't matter since new values are already set */

			} else {
				printf("\nInvalid Command\n"); /* if ever command is invalid */
				strcpy(data, buf);
			}

			nb = msg->i.nbytes;
		}

	_IO_SET_WRITE_NBYTES(ctp, nb);

	if (msg->i.nbytes > 0)
		mocb->ocb.flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS(0));
}
/***************************************
 * Function : io_open
 * Description: POSIX level
 * Connection funnction for connection adaptation
 * to our metronome device
 *
 * CONN - /dev/local/metronome
 *
 * Status : Complete
 ****************************************/
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
		void *extra) {

	if ((server_coid = name_open(METRO_ATTACH, 0)) == ERROR) { /* open namespace connection when file is opened */
		perror("ERROR - name_open failed - io_open() \n "); /* On error */
		return EXIT_FAILURE;
	}
	return (iofunc_open_default(ctp, msg, &handle->attr, extra));
}

void *metronome_thread(void * ta) {
	struct sigevent event; /* event, listening for pulse */
	struct itimerspec itime; /* timer specs */
	timer_t timer_id; /* timer id */
	my_message_t msg; /* message structure */
	int rcvid; /* return from msg receive */
	int index = 0; /* indexing */
	char * tp; /* pointer, going to be used to point to metronome table */
	int timer_status;/* could have been put in timer_props but meh */

	/* PHASE 1 */
	if ((attach = name_attach(NULL, METRO_ATTACH, 0)) == NULL) {/* attach namespace */
		printf("Error - name_attach - ./metronome.c \n");
		exit(EXIT_FAILURE);
	}

	/* SETUP EVENT HANDLER */
	event.sigev_notify = SIGEV_PULSE; /* PULSE TIMER */
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0,
			attach->chid,
			_NTO_SIDE_CHANNEL, 0); /* Attach Connection for timer */
	event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT; /* priority of event/pulse */
	event.sigev_code = METRO_PULSE_CODE; /* set pulse code to metronome pulse code 0 */

	/* CREATE TIMER ON PULSE EVENT */
	timer_create(CLOCK_REALTIME, &event, &timer_id);

	/* Search for current properties in table */
	index = search_idx_table(&Metronome);

	set_timer_props(&Metronome);/* Set properties for timer */

	start_timer(&itime, timer_id, &Metronome);/* start time with props */

	tp = t[index].pattern;/* table pointer to indexes of pattern string */

	/* PHASE 2 */

	for (;;) {
		/*Check for Pulse */
		if ((rcvid = MsgReceive(attach->chid, &msg, sizeof(msg),
				NULL)) == ERROR) {
			printf("Error - MessageReceive() - ./metronome\n");
			exit(EXIT_FAILURE);
		}

		if (rcvid == PULSE) { /* PULSE RECEIVED */
			switch (msg.pulse.code) {
			case METRO_PULSE_CODE: /* GENERAL PULSE SENT by timer */
				if (*tp == '|') { /* check if first char | */
					printf("%.2s", tp); /* print first 2 chars */
					tp = (tp + 2);/* pointer now starts at 3 */
				} else if (*tp == '\0') { /* check if end of string*/
					printf("\n");
					tp = t[index].pattern;/* reset to beginning */
				} else {
					printf("%c", *tp++);
				}

				break;
			case PAUSE_PULSE_CODE: /* PAUSE PULSE */
				if (timer_status == START) {
					itime.it_value.tv_sec = msg.pulse.value.sival_int; /*Pause timer for specified amount of seconds */
					timer_settime(timer_id, 0, &itime, NULL); /* set timer to pause */
				}
				break;
			case QUIT_PULSE_CODE: /* QUIT PULSE */
				timer_delete(timer_id); /* delete timer */
				name_detach(attach, 0); /* terminate namespace, close channel */
				name_close(server_coid); /* close server connection */
				exit(EXIT_SUCCESS); /* Terminate Program */
			case SET_PULSE_CODE: /* SET PULSE */
				index = search_idx_table(&Metronome); /* search for new index * new metronome properties */
				tp = t[index].pattern; /* point to new pattern */
				set_timer_props(&Metronome);/* set new timer properties */
				start_timer(&itime, timer_id, &Metronome);/*start new timer with new props */
				printf("\n");/* start on new line */
				break;
			case START_PULSE_CODE: /* START PULSE */
				if (timer_status == STOPPED) { /* ONLY IF status is stopped */
					start_timer(&itime, timer_id, &Metronome); /* Start timer */
					timer_status = START;
				}
				break;
			case STOP_PULSE_CODE: /* STOP PULSE */
				if (timer_status == START || timer_status == PAUSED) { /* only possible to stop if running or paused */
					stop_timer(&itime, timer_id); /* STOP timer */
					timer_status = STOPPED;/* CHANGE STATUS */
				}
				break;
			}
		}
		fflush(stdout); /* flush standard output */

	}
	return NULL;
}
/***********************************************
 * Function: set_timer_props
 * Description: Function used to
 * set the timer properties and
 * perform the calculations necessary
 * for the timer to perform the
 * appropriate intervals before sending a pulse
 ************************************************/
void set_timer_props(Metronome_t * Metronome) {
	Metronome->t_props.bps = (double) 60 / Metronome->m_props.bpm; /* 60 sec / beats per minute = seconds per beat */
	/* Beat per Measure */
	Metronome->t_props.measure = Metronome->t_props.bps * 2; /* beats per second * 2 = beat per measure */
	/* seconds per interval */
	Metronome->t_props.interval = Metronome->t_props.measure
			/ Metronome->m_props.tsb;
	/* Nano Seconds */
	Metronome->t_props.nano_sec = (Metronome->t_props.interval
			- (int) Metronome->t_props.interval) * 1e+9;

}

/******************************************************
 * Function: search_idx_table
 * Description: Used to search through the
 * table comparing with the props of the command line
 * arguments and the device properties, to search
 ******************************************************/
int search_idx_table(Metronome_t * Metronome) {
	int index = 0;
	for (index = 0; index < 8; index++) { /* search through table */
		if (t[index].tsb == Metronome->m_props.tsb
				&& t[index].tst == Metronome->m_props.tst) {
			return index; /* return index if found */
		}
	}
	return ERROR; /* INVALID POSSIBLE ERROR CHECK, not implemented, not in specs scope */
}
/*****************************
 * Function: stop_timer
 * Description: Used to stop
 * the current timer.
 *****************************/
void stop_timer(struct itimerspec * itime, timer_t timer_id) {
	/* SET TIMEer to 0 to stop */
	itime->it_value.tv_sec = 0; /* Time elapse 0 seconds */
	timer_settime(timer_id, 0, itime, NULL); /* set new time */
}
/*****************************
 * Function: start_timer
 * Description: Used to start
 * the current timer.
 *****************************/
void start_timer(struct itimerspec * itime, timer_t timer_id,
		Metronome_t* Metronome) {
	/* START new time with new or current properties properties */
	itime->it_value.tv_sec = 1;
	itime->it_value.tv_nsec = 0;
	itime->it_interval.tv_sec = Metronome->t_props.interval; /* if ever a set was after a stop resets all values set will start the timer*/
	itime->it_interval.tv_nsec = Metronome->t_props.nano_sec;
	timer_settime(timer_id, 0, itime, NULL);
}
/******************************
 * Override IOFUNC_OCB_T ocb calloc()
 ******************************/
metro_ocb_t * metro_ocb_calloc(resmgr_context_t *ctp, ioattr_t *mattr) {
	metro_ocb_t *mocb;
	mocb = calloc(1, sizeof(metro_ocb_t));
	mocb->ocb.offset = 0;
	return (mocb);
}

/******************************
 * Override IOFUNC_OCB_T free()
 ******************************/
void metro_ocb_free(metro_ocb_t *mocb) {
	free(mocb);
}

