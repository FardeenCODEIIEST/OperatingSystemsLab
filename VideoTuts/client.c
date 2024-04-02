#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define SIZE 51200

struct request
{
	short opcode;
	char filename[100];
};

struct ack
{
	short opcode;
	char message[100];
};

struct transfer
{
	short opcode;
	short bno;
	char block[SIZE];
};

int main()
{
	int sockfd;
	struct sockaddr_in serverAddr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	int choice;

	struct ack a;

	while (1)
	{

		printf("\n1. Read request\n2. Write request\n0. Exit\n");

		scanf("%d", &choice);

		sendto(sockfd, &choice, sizeof(int), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

		if (choice == 2)
		{

			printf("\nEnter file name\n");
			char name[100];
			scanf("%s", name);

			int l = strlen(name);

			struct request r;
			r.opcode = 1;
			strcpy(r.filename, name);

			FILE *fp;

			/*if(name[l-3] == 't' && name[l-2] == 'x' && name[l-1] == 't')
			fp = fopen(name, "r");
			else*/

			fp = fopen(name, "rb");

			fseek(fp, 0, SEEK_END);
			int file_size = ftell(fp);
			rewind(fp);

			printf("\nFile size = %d\n", file_size);
			// return 0;

			int total = file_size / SIZE + 1;

			printf("\nTotal = %d\n", total);
			int block = 0, index = 0;

			struct ack a;

			while (1)
			{
				sendto(sockfd, &r, sizeof(r), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

				recvfrom(sockfd, &a, sizeof(a), 0, NULL, NULL);

				if (a.opcode == 4)
				{
					printf("\nMessage = %s\n", a.message);
					break;
				}
			}

			sleep(1);

			int s = SIZE;

			sendto(sockfd, &s, sizeof(int), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

			sendto(sockfd, &total, sizeof(int), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

			if (total >= 2)
			{
				while (index < total)
				{
					struct transfer t;
					t.opcode = 2;
					t.bno = block;
					// t.block = (char*)malloc(SIZE*sizeof(char));
					memset(t.block, 0, sizeof(t.block));
					fread(t.block, 1, SIZE, fp);

					sendto(sockfd, &t, sizeof(t), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

					// sleep(1);

					struct ack a;
					recvfrom(sockfd, &a, sizeof(a), 0, NULL, NULL);

					if (a.opcode == 4)
					{
						printf("\nMessage = %s", a.message);
						block++;
						index++;
						sleep(1);
					}
					else if (a.opcode == 5)
						fseek(fp, -SIZE, SEEK_CUR);

					// free(t.block);
				}
			}

			int rem_size = file_size;

			if (rem_size > 0 && rem_size < SIZE)
			{

				struct transfer t;
				t.opcode = 2;
				t.bno = block;
				memset(t.block, 0, sizeof(t.block));

				fread(t.block, 1, rem_size, fp);

				while (1)
				{
					sendto(sockfd, &t, sizeof(t), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
					// sleep(1);
					struct ack a;
					recvfrom(sockfd, &a, sizeof(a), 0, NULL, NULL);

					if (a.opcode == 4)
					{
						printf("\nMessage = %s\n", a.message);
						break;
					}
					else if (a.opcode == 5)
						fseek(fp, -rem_size, SEEK_CUR);
				}
			}

			fclose(fp);
		}

		else if (choice == 1)
		{

			printf("\nEnter file name\n");
			char name[100];
			scanf("%s", name);

			int l = strlen(name);

			struct request r;
			r.opcode = 2;
			strcpy(r.filename, name);

			// struct ack a;

			while (1)
			{

				sendto(sockfd, &r, sizeof(r), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

				recvfrom(sockfd, &a, sizeof(a), 0, NULL, NULL);

				printf("\nMessage = %s\n", a.message);
				break;
			}

			if (a.opcode == 5)
				continue;

			FILE *fp;

			/*if(name[l-3] == 't' && name[l-2] == 'x' && name[l-1] == 't')
			fp = fopen(name, "r");
			else*/

			fp = fopen(name, "wb");

			int total, i, b = 1;
			int size;

			recvfrom(sockfd, &size, sizeof(int), 0, NULL, NULL);

			recvfrom(sockfd, &total, sizeof(int), 0, NULL, NULL);

			printf("\nMaximum block size = %d\n", size);
			struct transfer
			{
				short opcode;
				short bno;
				char block[size];
			};

			struct transfer t;
			// struct ack a;

			printf("\nTotal number of blocks to be recieved = %d\n", total);

			for (i = 0; i < total; i++)
			{

				recvfrom(sockfd, &t, sizeof(t), 0, NULL, NULL);

				fwrite(t.block, 1, sizeof(t.block), fp);

				a.opcode = 4;
				strcpy(a.message, "Block received\n");
				sendto(sockfd, &a, sizeof(a), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

				printf("\nBlock %d recieved\n", b);
				b++;
			}

			printf("\nFile recieved\n");

			fclose(fp);
		}

		else
		{
			close(sockfd);
			break;
		}
	}

	return 0;
}
