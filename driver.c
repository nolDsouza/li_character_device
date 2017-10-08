#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define BUFF_LEN 98
#define EXTRACHARS 2

static char receive[BUFF_LEN+EXTRACHARS];

int main(void) {
	int ret, fd;
	char stringToSend[BUFF_LEN+EXTRACHARS];
	printf("Starting device driver application\n");
	fd = open("/dev/s3600251Device", O_RDWR);
	if (fd < 0) {
		perror("Failed to open the device...");
		return errno;
	}
	
	printf("Successfully opened character device\n"
		"Type in a short string to send to the kernel model:\n");
	fgets(stringToSend, BUFF_LEN, stdin);
	printf("Writing message to device [%s].\n", stringToSend);
	ret = write(fd, stringToSend, strlen(stringToSend));
	if (ret < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}
	
	printf("Press ENTER to read back from the device...\n");
	getchar();

	printf("Reading from the device...\n");
	ret = read(fd, receive, BUFF_LEN);
	if (ret < 0) {
		perror("Failed to read the message from the device.");
		return errno;
	}
	printf("The received message is: [%s]\n", receive);
	printf("End of the Program\n");
	return 0;
}

