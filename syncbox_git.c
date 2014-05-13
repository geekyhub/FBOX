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

int main ( int argc, char *argv[] )
{
	int pfd;
	int READ;
	char buf[BUFFER_SIZE];
	int watch;
	int i = 0;
	int status;

	if(argc != 4){
		printf("usage [./syncbox] [-t seconds] [-v] [-n] src dst\n");
		return EXIT_FAILURE;
	}
	
	pfd = inotify_init();/*creating the INOTIFY instance*/
	if (pfd == -1){
		perror("inotify_init");
	}

	const char *pathname = argv[2];
	const char *pcopy = argv[3];
	watch = inotify_add_watch(pfd, pathname, IN_ALL_EVENTS);//listing changes to directory/file
	if (watch == -1){
		perror("inotify_add_watch");
	}

	int pid = fork();
	if (pid < 0){
		perror("fork");
	}
	else if(pid == 0){
		printf("syncbox: added watch for %s\n",pathname );
		printf("syncbox: synchronising %s to %s.\n",pathname,pcopy);
		printf("syncbox: rsync -az --delete %s %s.\nsyncbox: synchronization finished.\n",pathname,pcopy);
		execl("/usr/bin/rsync","rsync","-az","--delete",pathname,pcopy,(char*)NULL);
		perror("execl");
	
	}else if(pid > 0){		          			printf( "TEST");

		waitpid(pid,&status,0);
		while(READ = read(pfd,buf,BUFFER_SIZE ) >=0){
			while ( i < READ) {
		    	struct inotify_event *event = ( struct inotify_event * ) &buf[i];
		     	if (event->len) {
		     		 if (event->mask & IN_CREATE) {
		      		 	if (event->mask & IN_ISDIR) {
		          			printf( "syncbox: directory %s created.\n", event->name );
		        		}
		        		else {
		          			printf( "syncbox: file %s created.\n", event->name );
		       			}
		      		}
		     	 	else if ( event->mask & IN_DELETE ) {
		        		if ( event->mask & IN_ISDIR ) {
		         		 	printf( "syncbox: directory %s deleted.\n", event->name );
		        		}
		        		else {
		          			printf( "syncbox: file %s deleted.\n", event->name );
		        		}
		      		}
		      		else if ( event->mask & IN_CLOSE_WRITE ) {
		        		if ( event->mask & IN_ISDIR ) {
		         		 	printf("syncbox: directory %s opened for writing is closed.\n", event->name );
		        		}
		        		else {
		          			printf("syncbox: file %s opened for writing is closed.\n", event->name );
		        		}
		      		}
		      		else if ( event->mask & IN_CLOSE_NOWRITE ) {
		        		if ( event->mask & IN_ISDIR ) {
		         		 	printf("syncbox: directory %s opened not for writing is closed\n", event->name );
		        		}
		        		else {
		          			printf("syncbox: file %s opened not for writing is closed.\n", event->name );
		        		}
		      		}
		      		else if ( event->mask & IN_DELETE_SELF ) {
		        		if ( event->mask & IN_ISDIR ) {
		         		 	printf("syncbox: directory %s monitored is deleted-exiting\n", event->name );
		         		 	exit(1);

		        		}
		        		else {
		          			printf("syncbox: file %s monitored is deleted-exiting.\n", event->name );
		          			exit(2);
		        		}
		      		}
		      		else if ( event->mask & IN_MODIFY ) {
		        		if ( event->mask & IN_ISDIR ) {
		         		 	printf("syncbox: directory %s was modified\n", event->name );
		        		}
		        		else {
		          			printf("syncbox: file %s was modified.\n", event->name );
		        		}
		      		}
		      		else if ( event->mask & IN_MOVE_SELF) {
		        		if ( event->mask & IN_ISDIR ) {
		         		 	printf("syncbox: watched directory %s was itself moved\n", event->name );
		        		}
		        		else {
		          			printf("syncbox: watched file %s was itself moved.\n", event->name );
		        		}
		      		}
		      		else if ( event->mask & IN_MOVED_FROM || event->mask & IN_MOVED_TO) {
		        		if ( event->mask & IN_ISDIR ) {
		         		 	printf("syncbox: directory %s moved/renamed\n", event->name );
		        		}
		        		else {
		          			printf("syncbox: file %s moved/renamed.\n", event->name );
		        		}
		      		}
		      		else if ( event->mask & IN_OPEN) {
		        		if ( event->mask & IN_ISDIR ) {
		         		 	printf("syncbox: directory %s was opened\n", event->name );
		        		}
		        		else {
		          			printf("syncbox: file %s was opened.\n", event->name );
		        		}
		      		}
		      		

		    	}

		    i += EVENT_SIZE + event->len;
		  }
		  
		}
		
	}

		inotify_rm_watch(pfd,watch);
		close(pfd);
	return 0;
}




