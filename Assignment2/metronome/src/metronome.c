#include "metronome.h"
/*************************
 * Function Declaration
 ************************/
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
		void *extra);
void *metronome_thread(void*context);
void usage();

/***********************
 * GLOBAL Variables
 ***********************/
name_attach_t * metro_conn; /* Namespace connection */
int bpm,tst,tsb;
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
	iofunc_attr_t ioattr;/* I/O attribute structure, attributes associated to Resource Manager */
	pthread_attr_t thread_attrib; /* Thread Attributes */
	dispatch_context_t * ctp; /* Returned Dispatch Context from dispatch_context_alloc()*/
	int id;
	metro_props_t metro_props; /* Metronome Device properties (bpm,tst,tsb) */

	int server_coid; /* Thread ID */

	if (argc != 4) { /* Check program USAGE. MUST have 4 params NOTE: including ./metronome */
		printf("ERROR: You must provide all required arguments.\n");
		usage();
		exit(EXIT_FAILURE); /* termination */
	}

	/* Process Command Line Arguments - Metronome Properties*/
	metro_props.bpm = atoi(argv[1]); /* Beats per minute */
	bpm = atoi(argv[1]);
	metro_props.tst = atoi(argv[2]); /* time - signature top */
	tst = atoi(argv[2]);
	metro_props.tsb = atoi(argv[3]);/* time - signature bottom */
	tsb = atoi(argv[3]);

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

	/* Initialize attribute structure with Resource Manager */
	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);

	/* Attach path to pathname space - Resource Manager */
	if ((id = resmgr_attach(dpp, NULL, METRO_PATH, _FTYPE_ANY, 0, &conn_funcs,
			&io_funcs, &ioattr)) == -1) {
		fprintf(stderr, "%s:  Unable to attach name.\n", argv[0]);
		return (EXIT_FAILURE); /* On fail Terminate */
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
	return EXIT_SUCCESS; /* Succesful TERMINATION */

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

	double bps; /* Beats per second */
	double measure; /* Beat per measure */
	double interval; /* Sec per interval */
	double nano_sec; /* nano seconds value */
	int index; /* Search index */
	int nb;

	if (data == NULL)
		return 0;

	/* Search for current properties in Metronome Property Table */
	for(int i = 0; i< 8 ; i++){
		if (t[i].tsb == tsb && t[i].tst == tst){ /* if properties found in table, save index */
			index = i;
			break; /* once found stop searching */
		}
	}

	bps = (double) 60 / bpm; /* 60 sec / beats per minute = seconds per beat */
	/* Beat per Measure */
	measure = bps * 2; /* beats per second * 2 = beat per measure */
	/* seconds per interval */
	interval = measure / tsb;
	/* Nano Seconds */
	nano_sec = (interval - (int) interval) * 1e+9;
	sprintf(data,"[metronome: %d beats/min, time signature: %d/%d, sec-per-interval: %.2f, nanoSecs: %lf]\n",bpm,t[index].tst,t[index].tsb, interval,nano_sec);


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
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;

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
		char *pause_msg;
		int i, small_integer;
		buf = (char *) (msg + 1);

		//TODO not sure also need to add stop start
		//	if(strstr(buf, "set") != NULL){

		//}

		if (strstr(buf, "pause") != NULL) {
			for (i = 0; i < 2; i++) {
				pause_msg = strsep(&buf, " ");
			}
			small_integer = atoi(pause_msg);
			if (small_integer >= 1 && small_integer <= 9) {
				//FIXME :: replace getprio() with SchedGet()
				MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
				PAUSE, small_integer);
			} else {
				printf("Integer is not between 1 and 9.\n");
			}
		} else {
			strcpy(data, buf);
		}

		if (strstr(buf, "quit") != NULL) {
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
			QUIT, small_integer);
		}

		nb = msg->i.nbytes;
	}
	_IO_SET_WRITE_NBYTES(ctp, nb);

	if (msg->i.nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

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

void *metronome_thread(void * ctp) {
	metro_props_t * metro_props = ctp;
	struct sigevent event;
	struct itimerspec itime;
	timer_t timer_id;
	my_message_t msg;
	int rcvid;
	double bps = 0; /* Beats per second */
	double measure = 0; /* Beat per measure */
	double interval = 0; /* Sec per interval */
	double nano = 0; /* Nano Seconds */
	int index = 0;
	char * tp;

	/* PHASE 1 */
	if ((metro_conn = name_attach(NULL, METRO_ATTACH, 0)) == NULL) {
		printf("Error - name_attach - ./metronome.c \n");
		exit(EXIT_FAILURE);
	}

	/* SETUP EVENT HANDLER */
	event.sigev_notify = SIGEV_PULSE; /* PULSE TIMER */
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, metro_conn->chid, _NTO_SIDE_CHANNEL, 0); /* Attach Connection for timer */
	event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
	event.sigev_code = METRO_PULSE_CODE; /* set pulse code to metronome pulse code 0 */

	/* CREATE TIMER ON PULSE EVENT */
	timer_create(CLOCK_REALTIME, &event, &timer_id);

	/* Search for current properties in table */
	for (int i = 0; i < 8; i++) {
		if (t[i].tsb == metro_props->tsb && t[i].tst == metro_props->tst) {
			index = i;
			break;
		}
	}


	/* TIMER CALCULATION */
	/* Beats per second */
	bps = (double) 60 / metro_props->bpm; /* 60 sec / beats per minute = seconds per beat */
	/* Beat per Measure */
	measure = bps * 2; /* beats per second * 2 = beat per measure */
	/* seconds per interval */
	interval = measure / metro_props->tsb;
	/* Nano Seconds */
	nano = interval - (int) interval;

	itime.it_value.tv_sec = 1;
	itime.it_value.tv_nsec = 500000000;

	itime.it_interval.tv_sec = interval;
	itime.it_interval.tv_nsec = nano * 1e+9;

	timer_settime(timer_id, 0, &itime, NULL);

	tp = t[index].pattern;

	/* PHASE 2 */

	for (;;) {
		/* PULSE REVEIVED */
		if ((rcvid = MsgReceive(metro_conn->chid, &msg, sizeof(msg), NULL))
				== -1) {
			printf("Error - MessageReceive() - ./metronome\n");
			exit(EXIT_FAILURE);
		}

		if (rcvid == 0) {
			switch (msg.pulse.code) {
			case METRO_PULSE_CODE:
				printf("%c", *tp++);
				break;
			case PAUSE:
				itime.it_value.tv_sec = msg.pulse.value.sival_int;
				timer_settime(timer_id, 0, &itime, NULL);
				break;
			case QUIT:
				timer_delete(timer_id);
				name_detach(metro_conn, 0);
				name_close(server_coid);
				exit(EXIT_SUCCESS);

			}
		}

	}
}

