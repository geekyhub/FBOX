/*320202
Tinashe Matate
OPERATING SYSTEMS ASSIGNMENT
tmatate@jacobs-university.de
*/

#include <sys/wait.h>
#include <sys/inotify.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <string.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUFFER_SIZE (1024*( EVENT_SIZE + 16 ))

typedef struct inotify_event* event;
/*finds out what event took place */
void processEvent(event e, char* eventType);
/*implements the select function to wait for events to read*/
int my_select(int fd,int mytime);
/*prints event respective of what is affected,directory or file*/
void printEvent(event e, char* eventType);
void usage(int argc);


int main ( int argc, char *argv[] ) {

	char buf[BUFFER_SIZE];
	int pfd,ii,READ,watch,status,VERBOSE = 0,DRY_RUN = 0;
	const char *pathname;
	const char *pcopy;
	usage(argc);
	for (ii = 0;ii < argc; ii++){
		if(strcmp(argv[ii],"-v") == 0){
			VERBOSE = 1;
		}

		if(strcmp(argv[ii],"-n") == 0){
			DRY_RUN = 1;
		}

	}
	if (VERBOSE == 1  && DRY_RUN == 1){
		pathname = argv[5];
		pcopy = argv[6];
	}else if((!VERBOSE && DRY_RUN) || (VERBOSE && !DRY_RUN)){
		pathname = argv[4];
		pcopy = argv[5];
	} else{
		usage(argc);
	}
	/*creating the INOTIFY instance*/
	pfd = inotify_init();
	if (pfd == -1){
		perror("inotify_init");
	}

	int timeset = atoi(argv[2]);
	//listing changes to directory/file
	watch = inotify_add_watch(pfd, pathname,IN_ALL_EVENTS);
	if(VERBOSE)
		printf("syncbox: added watch for %s\n",pathname);
	while(my_select(pfd,timeset) != -1) {
		READ = read(pfd,buf,BUFFER_SIZE);
		int i = 0;
		while (i < READ) {
	    	struct inotify_event *event = ( struct inotify_event * ) &buf[i];
	     	if (event->len) {
	     		if (event->mask & IN_CREATE) {
	     		 	processEvent(event, "created");
	      		}
	     	 	else if ( event->mask & IN_DELETE ) {
	     		 	processEvent(event, "deleted-exiting");
	     		 	inotify_rm_watch(pfd,watch);
					close(pfd);
					exit(1);

	      		}
	      		else if ( event->mask & IN_CLOSE_WRITE ) {
	      			processEvent(event, "opened for writing is closed");
	      		}
	      		else if ( event->mask & IN_CLOSE_NOWRITE ) {
	      			processEvent(event, "opened not for writing is closed");
	      		}
	      		else if ( event->mask & IN_DELETE_SELF ) {
	      			processEvent(event, "monitored is deleted-exiting");
	      			inotify_rm_watch(pfd,watch);
					close(pfd);
	      			exit(1);
	      		}
	      		else if ( event->mask & IN_MODIFY ) {
	      			processEvent(event, "was modified");
	      		}
	      		else if ( event->mask & IN_MOVE_SELF) {
	      			processEvent(event, "was itself moved");
	      		}
	      		else if ( event->mask & IN_MOVED_FROM || event->mask & IN_MOVED_TO) {
	      			processEvent(event, "moved/renamed");
	      		}
	      		else if ( event->mask & IN_OPEN) {
	      			processEvent(event, "was opened");
	      		}
	      				      		
	    	}
		   
			i += EVENT_SIZE + event->len;
		}

	}
	int pid = fork();
			if (pid < 0) {
				perror("fork");
			}
			else if(pid == 0){
				if (VERBOSE && DRY_RUN==0) {
					printf("syncbox: synchronising %s to %s.\n",pathname,pcopy);
					execl("/usr/bin/rsync","rsync","-az","--delete",pathname,pcopy,(char*)NULL);
					perror("execl");
				}
				else if(VERBOSE && DRY_RUN){
					printf("syncbox: DRY_RUN-no synchronization took place\n");
				}
							
			}
			else {		    
				waitpid(pid, &status, 0);
				if (VERBOSE && DRY_RUN == 0) {
					printf("syncbox: rsync -az --delete %s %s.\nsyncbox: synchronization finished.\n",pathname,pcopy);
				}
			}

	inotify_rm_watch(pfd,watch);
	close(pfd);
	return 0;
}

void processEvent(event e, char* eventType) {
	printEvent(e, eventType);
}
int my_select(int fd,int mytime) {
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	struct timeval timeout;
	timeout.tv_sec = mytime;
	timeout.tv_usec = 0;
	select(fd+1, &fds, NULL, NULL, &timeout);
	if (FD_ISSET(fd, &fds)) {
		return fd;
	}
	return -1;
}

void printEvent(event e, char* eventType) {
	if (e->mask & IN_ISDIR) {
		printf("syncbox: directory %s %s.\n", e->name, eventType);
	}
	else {
		printf("syncbox: file %s %s.\n", e->name, eventType);
	}
}

void usage(int argc) {
	if(argc == 7 || argc == 6 ||argc == 5){
	}else{
		printf("\nusage [./syncbox] [-t seconds] [-v](optional) [-n](optional) src dst\n");
		exit(1);
	}
}





