#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

volatile sig_atomic_t usr1Happened = 0;
sem_t *sem;
void sig_handler();
void* child_thread(void *arg);

int main(void) {

	int numthreads;
	pthread_attr_t attr;
	struct sigaction sa;

	sa.sa_handler = sig_handler;
	sa.sa_flags = 0; // or SA_RESTART
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("Enter the number of threads to create: ");
	scanf("%d", &numthreads);

	sem = sem_open("sem", O_CREAT, S_IWOTH, 0);

	for (int i = 0; i < numthreads; i++) {
		pthread_attr_init(&attr);
		pthread_create( NULL, &attr, &child_thread, NULL);
		pthread_attr_destroy(&attr);
	}

	while (!usr1Happened) {
	}

	sem_close(sem);
	sem_destroy(sem);
	return EXIT_SUCCESS;
}

void sig_handler() {
	usr1Happened++;
}

void *child_thread(void *arg) {
	printf("Child thread %d created\n", pthread_self());
	do {
		sem_wait(sem);
		printf("Child thread %d unblocked\n", pthread_self());
		sleep(5);
	} while (!usr1Happened);
	return NULL;
}
