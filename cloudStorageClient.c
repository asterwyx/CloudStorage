//
// Created by asterwyx on 10/28/19.
//
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "eventCtl.h"


int main(int argc, char *argv[])
{
    int client_sock;
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(SERV_PORT);
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(client_sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1)
    {
        printf("Connect failed\n");
    }
    char test[BUF_SIZE] = "POST /eventCtl.h HTTP/1.1\r\n"
                          "Accept: *\r\n"
                          "\r\n";
    FILE *fp = fopen("../eventCtl.h", "rb");
    size_t fileSize = getFileSize(fp);
    fread(test + strlen(test), fileSize, 1, fp);
    char data[BUF_SIZE];
    printf("%s\n", test);
    send(client_sock, test, strlen(test), 0);
    recv(client_sock, data, BUF_SIZE, 0);
    printf("Echo:%s\n", data);
    close(client_sock);
    return 0;
}
