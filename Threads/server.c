#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

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
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t addrLen = sizeof(clientAddr);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		perror("bind");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	struct request r;
	struct ack a;

	int size;

	int choice;

	while (1)
	{

		recvfrom(sockfd, &choice, sizeof(int), 0, (struct sockaddr *)&clientAddr, &addrLen);

		if (choice == 2)
		{

			while (1)
			{
				recvfrom(sockfd, &r, sizeof(r), 0, (struct sockaddr *)&clientAddr, &addrLen);

				a.opcode = 4;
				strcpy(a.message, "request granted");

				sendto(sockfd, &a, sizeof(a), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
				break;
			}

			FILE *fp;

			int l = strlen(r.filename);

			/*if(r.filename[l-3]=='t' && r.filename[l-2] == 'x' && r.filename[l-1] == 't')
			fp = fopen(r.filename, "w");
			else*/

			fp = fopen(r.filename, "wb");

			int total, i, b = 1;

			recvfrom(sockfd, &size, sizeof(int), 0, (struct sockaddr *)&clientAddr, &addrLen);
			printf("\nMaximum block size = %d\n", size);
			struct transfer
			{
				short opcode;
				short bno;
				char block[size];
			};

			struct transfer t;

			recvfrom(sockfd, &total, sizeof(int), 0, (struct sockaddr *)&clientAddr, &addrLen);
			printf("\nTotal number of blocks to be recieved = %d\n", total);

			for (i = 0; i < total; i++)
			{

				recvfrom(sockfd, &t, sizeof(t), 0, (struct sockaddr *)&clientAddr, &addrLen);

				fwrite(t.block, 1, sizeof(t.block), fp);

				a.opcode = 4;
				strcpy(a.message, "Block received\n");
				sendto(sockfd, &a, sizeof(a), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

				printf("\nBlock %d recieved\n", b);
				b++;
			}

			printf("\nFile recieved\n");

			fclose(fp);
		}

		else if (choice == 1)
		{

			while (1)
			{
				recvfrom(sockfd, &r, sizeof(r), 0, (struct sockaddr *)&clientAddr, &addrLen);

				if (access(r.filename, F_OK) != -1)
				{
					a.opcode = 4;
					strcpy(a.message, "request granted");

					sendto(sockfd, &a, sizeof(a), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

					break;
				}

				else
				{
					a.opcode = 5;
					strcpy(a.message, "File does not exist");

					sendto(sockfd, &a, sizeof(a), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

					break;
				}
			}

			if (a.opcode == 5)
				continue;

			FILE *fp;

			int l = strlen(r.filename);

			fp = fopen(r.filename, "rb");

			fseek(fp, 0, SEEK_END);
			int file_size = ftell(fp);
			rewind(fp);

			printf("\nFile size = %d\n", file_size);
			// return 0;

			int total = file_size / SIZE + 1;

			printf("\nTotal = %d\n", total);
			int block = 0, index = 0;

			sleep(1);

			int s = SIZE;

			sendto(sockfd, &s, sizeof(int), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

			sleep(1);

			sendto(sockfd, &total, sizeof(int), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

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

					sendto(sockfd, &t, sizeof(t), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

					// sleep(1);

					struct ack a;
					if (recvfrom(sockfd, &a, sizeof(a), 0, (struct sockaddr *)&clientAddr, &addrLen) == -1)
					{
						perror("recv");
						fclose(fp);
						close(sockfd);
						exit(EXIT_FAILURE);
					}

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
					sendto(sockfd, &t, sizeof(t), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
					// sleep(1);
					struct ack a;
					recvfrom(sockfd, &a, sizeof(a), 0, (struct sockaddr *)&clientAddr, &addrLen);

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

		else
		{
			close(sockfd);
			break;
		}
	}
	return 0;
}
