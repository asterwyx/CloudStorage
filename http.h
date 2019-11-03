//
// Created by asterwyx on 11/1/19.
//

#ifndef CLOUDSTORAGESERVER_HTTP_H
#define CLOUDSTORAGESERVER_HTTP_H
#include <glob.h>
#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "eventCtl.h"
#include "threadpool.h"

#define MAX_SIZE 4096
#define DESC_SIZE 100
#define BUF_SIZE 4096
#define PATH "/home/asterwyx/CloudStorageRootFolder%s"
typedef struct request {
    char *method;
    char *path;
    int httpVersion[2];
    char *accept;
    char *body;

} Request;

typedef struct response {
    int httpVersion[2];
    int statusCode;
    char *description;
    char *contentType;
    size_t contentLength;
    char *body;
} Response;

Request *parseRequest(const char *req);
char *printResponse(Response *res);
void doGet(void *arg);
void doPost(void *arg);
void doDelete(void *arg);

void init();
void handle(void *arg);
#endif //CLOUDSTORAGESERVER_HTTP_H
