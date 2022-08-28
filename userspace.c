#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>

#define NUM_THREADS 2
#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

void *thread_function(void *ptr)		//Thread Function
{
	int ret,fd;
	char stringToSend[BUFFER_LENGTH];
	int *myid = (int *)ptr;
	fd=open("/dev/kernelDriver",O_RDWR);
	if (fd < 0)
        	perror("Failed to open the device...");
	printf("Running thread:[%d]\n",*myid);
	printf("Type in a short string to send to the kernel module:\n");
	scanf("%[^\n]%*c",stringToSend);

	printf("Writing message to the device [%s].\n",stringToSend);
	ret=write(fd,stringToSend,strlen(stringToSend));
	if (ret < 0)
		perror("Failed to write the message to the device.");

	printf("Press ENTER to read back from the device...\n");
	getchar();

	printf("Reading from the device...\n");
	ret = read(fd, receive, BUFFER_LENGTH);   // Read the response from the LKM
	if (ret < 0)
		perror("Failed to read the message from the device.");

	printf("The received message is: [%s]\n", receive);
	//Close the device
	int err = close(fd);
	if (err < 0)
		perror("Error closing file.");
}

int main(int argc,char *argv[])
{
	pthread_t threads[NUM_THREADS];
	int rc,i,j;

	printf("Starting device code example...\n");
	//Create Threads
	for (i=0;i<NUM_THREADS;i++)
	{
		rc=pthread_create(&threads[i],NULL,thread_function,(void *)&threads[i]);
		if(rc)
		{
			printf("ERROR;return code from pthread_create()is %d\n",rc);
			exit(-1);
		}
		//Wait for the thread to terminate
		pthread_join( threads[i],NULL);
	}
	pthread_exit(NULL); 		//Terminates the thread
	printf("End of the program\n");

	return 0;
}
