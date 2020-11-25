#include "metronome.h"
/*************************
 * Function Declaration
 ************************/
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);/* POSIX I/O Function  @ Override*/
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);/* POSIX I/O Function  @ Override*/
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
		void *extra);/* POSIX Connections Function  @ Override*/
void *metronome_thread(void*properties); /* Thread Work function */
void set_timer_props(metro_props_t * metro_props);/* setting timer properties */
int search_idx_table(metro_props_t * metro_props);/* search through Config table for metronome */
void stop_timer(struct itimerspec * itime, timer_t timer_id); /* Stops current timer */
void start_timer(struct itimerspec * itime, timer_t timer_id);/* starts current timer */
void usage();/* Error message USAGE print out */

/***********************
 * GLOBAL Variables
 ***********************/
name_attach_t * metro_conn; /* Namespace connection */
//timer_props_t timer_props;
metro_props_t metro_props;
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
	dispatch_context_t * ctp; /* Returned Dispatch Context from dispatch_context_alloc()*/
	int id, i;

	int server_coid; /* Thread ID */

	if (argc != 4) { /* Check program USAGE. MUST have 4 params NOTE: including ./metronome */
		printf("ERROR: You must provide all required arguments.\n");
		usage();
		exit(EXIT_FAILURE); /* termination */
	}

	/* Process Command Line Arguments - Metronome Properties*/
	metro_props.bpm = atoi(argv[1]);
	metro_props.tst = atoi(argv[2]);
	metro_props.tsb = atoi(argv[3]);

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

		/* Attach path to pathname space - Resource Manager */
		if ((id = resmgr_attach(dpp, NULL, devnames[i], _FTYPE_ANY, 0,
				&conn_funcs, &io_funcs, &ioattr[i])) == -1) {
			fprintf(stderr, "%s:  Unable to attach name.\n", argv[0]);
			return (EXIT_FAILURE); /* On fail Terminate */
		}
	}
	/* returns dispatch context */
	ctp = dispatch_context_alloc(dpp);

	/* Initialize Thread Attribute and Create Thread */
	pthread_attr_init(&thread_attrib);
	pthread_create(NULL, &thread_attrib, &metronome_thread, &metro_props);

	/* While true */
	while (1) {
		ctp = dispatch_block(ctp); /* block while waiting for event  - returns context*/
		dispatch_handler(ctp); /* handle the event from dispatch block - context */
	}

	pthread_attr_destroy(&thread_attrib); /* destroy thread attributes */
	name_detach(metro_conn, 0); /* detach from namespace, terminating connection */
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
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {

	int index; /* Search index */
	int nb;

	if (data == NULL)
		return 0;

	if (ocb->attr->device == 1) {
		sprintf(data,
				"Metronome Resource Manager (Resmgr)\n\nUsage: metronome <bpm> <ts-top> <ts-bottom>\n\nAPI:\n pause[1-9]\t\t-pause the metronome for 1-9 seconds\n quit:\t\t- quit the metronome\n set <bpm> <ts-top> <ts-bottom>\t- set the metronome to <bpm> ts-top/ts-bottom\n start\t\t- start the metronome from stopped state\n stop\t\t- stop the metronome; use 'start' to resume\n");
	} else {
		/* Search for current properties in Metronome Property Table */
		index = search_idx_table(&metro_props);

		/* TIMER PROPERTIES Calculated and set ALREADY SET, no point on repeating code bps,measure,interval*/
		sprintf(data,
				"[metronome: %d beats/min, time signature: %d/%d, sec-per-interval: %.2f, nanoSecs: %lf]\n",
				metro_props.bpm, t[index].tst, t[index].tsb,
				metro_props.timer_props.interval,
				metro_props.timer_props.nano_sec);
	}
	nb = strlen(data);

	//test to see if we have already sent the whole message.
	if (ocb->offset == nb)
		return 0;

	//We will return which ever is smaller the size of our data or the size of the buffer
	nb = min(nb, msg->i.nbytes);

	//Set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nb);

	//Copy data into reply buffer.
	SETIOV(ctp->iov, data, nb);

	//update offset into our data used to determine start position for next read.
	ocb->offset += nb;

	//If we are going to send any bytes update the access time for this resource.
	if (nb > 0)
		ocb->flags |= IOFUNC_ATTR_ATIME;

	return (_RESMGR_NPARTS(1));
}

/***************************************
 * Function : io_write
 * Description: POSIX level
 * I/O funnction for writing adaptation
 * for our mentronome
 *
 *
 * Status : Code review, not sure if we should send Min Value
 ****************************************/
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {

	int nb = 0;

	if (msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg))) {
		/* have all the data */
		char *buf;
		char * pause_msg;
		char * set_msg;
		int i, small_integer = 0;
		buf = (char *) (msg + 1);

		if (strstr(buf, "pause") != NULL) {
			for (i = 0; i < 2; i++) {
				pause_msg = strsep(&buf, " ");
			}
			small_integer = atoi(pause_msg);
			if (small_integer >= 1 && small_integer <= 9) {
				//FIXME :: replace getprio() with SchedGet()
				MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
				PAUSE_PULSE_CODE, small_integer);
			} else {
				printf("Integer is not between 1 and 9.\n");
			}
		} else if (strstr(buf, "quit") != NULL) {
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
			QUIT_PULSE_CODE, small_integer);
		} else if (strstr(buf, "start") != NULL) {
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL), START_PULSE_CODE,
					small_integer);
		} else if (strstr(buf, "stop") != NULL) {
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL), STOP_PULSE_CODE,
					small_integer);
		} else if (strstr(buf, "set") != NULL) {
			for (i = 0; i < 4; i++) {
				set_msg = strsep(&buf, " "); /* start splitting string */

				if (i == 1) { /* First string BPM */
					metro_props.bpm = atoi(set_msg);
				} else if (i == 2) {/* SECOND STRING TST */
					metro_props.tst = atoi(set_msg);
				} else if (i == 3) {/* THIRD STRING TSB */
					metro_props.tsb = atoi(set_msg);
				}
			}

			MsgSendPulse(server_coid, SchedGet(0, 0, NULL), SET_PULSE_CODE,
					small_integer);

		} else {
			strcpy(data, buf);
		}

		nb = msg->i.nbytes;
	}
	_IO_SET_WRITE_NBYTES(ctp, nb);

	if (msg->i.nbytes > 0)
		ocb->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

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
	if ((server_coid = name_open(METRO_ATTACH, 0)) == -1) { /* open namespace connection when file is opened */
		perror("ERROR - name_open failed - io_open() \n "); /* On error */
		return EXIT_FAILURE;
	}
	return (iofunc_open_default(ctp, msg, handle, extra));
}

void *metronome_thread(void * properties) {
	metro_props_t * metro_props = properties; /* properties of metronome */
	struct sigevent event; /* event, listening for pulse */
	struct itimerspec itime; /* timer specs */
	timer_t timer_id; /* timer id */
	my_message_t msg; /* message structure */
	int rcvid; /* return from msg receive */
	int index = 0; /* indexing */
	char * tp; /* pointer, going to be used to point to metronome table */
	int timer_status;

	/* PHASE 1 */
	if ((metro_conn = name_attach(NULL, METRO_ATTACH, 0)) == NULL) {/* attach namespace */
		printf("Error - name_attach - ./metronome.c \n");
		exit(EXIT_FAILURE);
	}

	/* SETUP EVENT HANDLER */
	event.sigev_notify = SIGEV_PULSE; /* PULSE TIMER */
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, metro_conn->chid,
	_NTO_SIDE_CHANNEL, 0); /* Attach Connection for timer */
	event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT; /* priority of event/pulse */
	event.sigev_code = METRO_PULSE_CODE; /* set pulse code to metronome pulse code 0 */

	/* CREATE TIMER ON PULSE EVENT */
	timer_create(CLOCK_REALTIME, &event, &timer_id);

	/* Search for current properties in table */
	index = search_idx_table(metro_props);

	set_timer_props(metro_props);/* Set properties for timer */ //TODO set everytime there is a change in thread parameters needs to be done in a pulse

	start_timer(&itime, timer_id);

	tp = t[index].pattern;/* table pointer to indexes of pattern string */

	/* PHASE 2 */

	for (;;) {
		/* PULSE RECEIVED */
		if ((rcvid = MsgReceive(metro_conn->chid, &msg, sizeof(msg), NULL))
				== -1) {
			printf("Error - MessageReceive() - ./metronome\n");
			exit(EXIT_FAILURE);
		}

		if (rcvid == 0) {
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
				itime.it_value.tv_sec = msg.pulse.value.sival_int;
				timer_settime(timer_id, 0, &itime, NULL);
				break;
			case QUIT_PULSE_CODE: /* QUIT PULSE */
				timer_delete(timer_id);
				name_detach(metro_conn, 0);
				name_close(server_coid);
				exit(EXIT_SUCCESS);
			case SET_PULSE_CODE: /* SET PULSE */
				index = search_idx_table(metro_props); /* search for new index * new metronome properties */
				tp = t[index].pattern; /* point to new pattern */
				set_timer_props(metro_props);/* set new timer properties */
				start_timer(&itime, timer_id);/*start new timer with new props */
				printf("\n");/* start on new line */
				break;
			case START_PULSE_CODE: /* START PULSE */
				if (timer_status == STOPPED) { /* ONLY IF status is stopped */
					start_timer(&itime, timer_id);
					timer_status = START;
				}
				break;
			case STOP_PULSE_CODE: /* STOP PULSE */
				if (timer_status == START || timer_status == PAUSED) { /* only possible to stop if running or paused */
					stop_timer(&itime, timer_id); /* STOP */
					timer_status = STOPPED;/* CHANGE STATUS */
				}
				break;
			}
		}
		fflush(stdout);

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
void set_timer_props(metro_props_t * metro_props) {
	metro_props->timer_props.bps = (double) 60 / metro_props->bpm; /* 60 sec / beats per minute = seconds per beat */
	/* Beat per Measure */
	metro_props->timer_props.measure = metro_props->timer_props.bps * 2; /* beats per second * 2 = beat per measure */
	/* seconds per interval */
	metro_props->timer_props.interval = metro_props->timer_props.measure
			/ metro_props->tsb;
	/* Nano Seconds */
	metro_props->timer_props.nano_sec = (metro_props->timer_props.interval
			- (int) metro_props->timer_props.interval) * 1e+9;

}

/******************************************************
 * Function: search_idx_table
 * Description: Used to search through the
 * table comparing with the props of the command line
 * arguments and the device properties, to search
 ******************************************************/
int search_idx_table(metro_props_t * props) {
	for (int i = 0; i < 8; i++) {
		if (t[i].tsb == props->tsb && t[i].tst == props->tst) {
			return i;
		}
	}
	return -1; /* INVALID POSSIBLE ERROR CHECK */
}
/*****************************
 * Function: stop_timer
 * Description: Used to stop
 * the current timer.
 *****************************/
void stop_timer(struct itimerspec * itime, timer_t timer_id) {
	itime->it_value.tv_sec = 0;
	itime->it_value.tv_nsec = 0;
	itime->it_interval.tv_sec = 0;
	itime->it_interval.tv_nsec = 0;
	timer_settime(timer_id, 0, itime, NULL);
}
/*****************************
 * Function: start_timer
 * Description: Used to start
 * the current timer.
 *****************************/
void start_timer(struct itimerspec * itime, timer_t timer_id) {
	itime->it_value.tv_sec = 1;
	itime->it_value.tv_nsec = 0;
	itime->it_interval.tv_sec = metro_props.timer_props.interval;
	itime->it_interval.tv_nsec = metro_props.timer_props.nano_sec;
	timer_settime(timer_id, 0, itime, NULL);
}

