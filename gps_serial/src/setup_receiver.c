/*
 * Placeholder PetaLinux user application.
 *
 * Replace this with your application code
 */
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>

#define WAIT 5
void usage();


int main(int argc, char *argv[])
{
	  unsigned int sec;
      fd_set rfds;

// SETTING
	int gpsuart;
    struct timeval  timeout;
	//  signal(SIGALRM, usage);




	gpsuart=open("/dev/ttyUL1", O_RDWR | O_NOCTTY);

    FD_ZERO(&rfds);
    FD_SET(gpsuart, &rfds);


	 timeout.tv_sec = WAIT;    // WAIT seconds
     timeout.tv_usec = 0;    // 0 milliseconds

// WRITING
	char buffsend[7]={64,64,67,106,41,13,10};
	write(gpsuart,buffsend,sizeof(buffsend));

//sec= alarm(5);
//printf("sec=%d\n",sec);
// READING

		 char read_buffer[10];   /* Buffer to store the data received              */
		 int  bytes_read = 0;    /* Number of bytes read by the read() system call */
		 int i = 0;
	//	bytes_read = read(gpsuart,&read_buffer,5);
		 bytes_read = select(1, &rfds, NULL, NULL, &timeout);
	//	 printf("\nBytes Ricevuti %d", bytes_read);
	//	 printf("\n\n");



	//	 for(i=0;i<bytes_read;i++){   /*printing only the received characters*/
	//		 printf("%c",read_buffer[i]);
	//	 }

	//	 printf("%c",read_buffer[0]);

	//	 if (read_buffer[0] = "1")
		 	 if(bytes_read)

				 printf("{\"message\": \"OK OK\"}");
		 	 else
		 	printf("{\"message\": \"failed\"}");



	close(gpsuart);
}





void usage()
{
//	printf("{\"message\": \"failed\"}");
	system("killall gps_serial");
    return 0;
}


