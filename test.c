//
// Created by asterwyx on 11/2/19.
//
#include "http.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    char *message = "POST /test.txt HTTP/1.1\r\n"
                    "Accept: *\r\n\r\n"
                    "test message\r\n";
    Request *request = parseRequest(message);
    printf("%s\n", request->method);
    printf("%s\n", request->path);
    printf("%s\n", request->accept);
    printf("HTTP/%d.%d\n", request->httpVersion[0], request->httpVersion[1]);
    printf("%s\n", request->body);
    Response *response = doPost(request);
     printf("HTTP/%d.%d\n", response->httpVersion[0], response->httpVersion[1]);
     printf("%d\n", response->statusCode);
     printf("%s\n", response->description);
     printf("%s\n", response->contentType);
    printf("%lu\n", response->contentLength);
    printf("%s\n", response->body);
    return 0;
}