//
// Created by asterwyx on 11/1/19.
//
#include "http.h"

threadpool_t pool;			//这个线程池负责：1.接受客户端上传的文件包并加到任务列表 	2.将客户端要下载的文件读成数据包并加入任务列表
threadpool_t handler_pool;	//这个线程池负责：将任务队列里的包，按照包里的type和fd进行发送（客户端下载文件）或者写到本地文件（客户端上传文件）

Request *parseRequest(const char *req)
{
    Request *result = (Request *)malloc(sizeof(Request));
    char buffer[MAX_SIZE];
    int cursor = 0;
    int i = 0;
    for (i = 0; req[cursor] != ' ' ; ++i, ++cursor)
    {
        buffer[i] = req[cursor];
    }
    buffer[i] = '\0';
    result->method = (char *)malloc(strlen(buffer) + 1);
    strcpy(result->method, buffer);
    ++cursor;
    for (i = 0; req[cursor] != ' '; ++i, ++cursor)
    {
        buffer[i] = req[cursor];
    }
    buffer[i] = '\0';
    result->path = (char *)malloc(strlen(buffer) + 1);
    strcpy(result->path, buffer);
    for (; req[cursor] != '/'; ++cursor);
    ++cursor;
    for (i = 0; req[cursor] != '.'; ++i, ++cursor)
    {
        buffer[i] = req[cursor];
    }
    buffer[i] = '\0';
    result->httpVersion[0] = atoi(buffer);
    ++cursor;
    for (i = 0; req[cursor] != '\r'; ++i, ++cursor)
    {
        buffer[i] = req[cursor];
    }
    buffer[i] = '\0';
    result->httpVersion[1] = atoi(buffer);
    cursor += 2;
    for (i = 0; req[cursor] != '\r'; ++i, ++cursor)
    {
        buffer[i] = req[cursor];
    }
    buffer[i] = '\0';
    result->accept = (char *)malloc(10);
    if (strlen(buffer) == 0) {
        result->accept[0] = '\0';
    } else {
        int j = 0;
        for (j = 0 ; buffer[j] != ' ' ; ++j);
        ++j;
        for (i = 0; j < strlen(buffer); j++, i++) {
            result->accept[i] = buffer[j];
        }
        result->accept[i] = '\0';
    }
    cursor += 4;
    strcpy(buffer, req + cursor);
    result->body = (char *)malloc(strlen(buffer) + 1);
    strcpy(result->body, buffer);
    // 如果主体以\r\n结尾，去掉\r\n存储
    if (result->body[strlen(result->body) - 1] == '\n' && result->body[strlen(result->body) - 2] == '\r') {
        result->body[strlen(result->body) - 1] = '\0';
        result->body[strlen(result->body) - 1] = '\0';
    }
    return result;
}

char *printResponse(Response *res)
{
    char *result = (char *)malloc(BUF_SIZE);
    sprintf(result + strlen(result), "HTTP/%d.%d ", res->httpVersion[0], res->httpVersion[1]);
    sprintf(result + strlen(result), "%d ", res->statusCode);
    sprintf(result + strlen(result), "%s\r\n", res->description);
    if (strcmp(res->contentType, "") == 0) {
        sprintf(result + strlen(result), "Content-Type: %s\r\n", res->contentType);
    }
    if (res->contentLength != 0) {
        sprintf(result + strlen(result), "Content-Length: %zu\r\n", res->contentLength);
    }
    sprintf(result + strlen(result), "\r\n");
    sprintf(result + strlen(result), "%s\r\n", res->body);
    return result;
}

void doGet(void *arg)
{
    my_event_t *my_event = (my_event_t *)arg;
    Request *req = (Request *)my_event->r_ptr;
    Response *result = (Response *)malloc(sizeof(Response));
    result->description = (char *)malloc(DESC_SIZE);
    result->body = (char *)malloc(MAX_SIZE);
    memset(result->body, 0, MAX_SIZE);
    result->httpVersion[0] = req->httpVersion[0];
    result->httpVersion[1] = req->httpVersion[1];
    result->statusCode = 400;
    result->contentLength = 0;
    result->contentType = (char *)malloc(20);
    memset(result->contentType, 0, 20);
    char *filename = (char *)malloc(BUF_SIZE);
    sprintf(filename, PATH, req->path);
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        result->statusCode = 404;
        strcpy(result->description, "Not Found");
    } else {
        size_t fileSize = getFileSize(fp);
        if (fread(result->body, fileSize, 1, fp) != 0) {
            strcpy(result->description, "OK");
            result->statusCode = 200;
            result->contentLength = fileSize;
            result->contentType = getFileType(req->path);
        }
        else
        {
            strcpy(result->description, "Internal Server Error");
            result->statusCode = 500;
            strcpy(result->body, "Can't read from the file!");
        }
    }

    fclose(fp);
    my_event->r_ptr = result;
    threadpool_add_task(&handler_pool, handle, arg);
}
void doPost(void *arg)
{
    my_event_t *my_event = (my_event_t *)arg;
    Request *req = my_event->r_ptr;
    Response *result = (Response *)malloc(sizeof(Response));
    result->description = (char *)malloc(DESC_SIZE);
    result->body = (char *)malloc(MAX_SIZE);
    memset(result->body, 0, MAX_SIZE);
    result->httpVersion[0] = req->httpVersion[0];
    result->httpVersion[1] = req->httpVersion[1];
    result->statusCode = 400;
    result->contentLength = 0;
    result->contentType = (char *)malloc(20);
    memset(result->contentType, 0, 20);
    char *filename = (char *)malloc(BUF_SIZE);
    sprintf(filename, PATH, req->path);
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        result->statusCode = 400;
        strcpy(result->description, "Bad Request");
    } else {
        unsigned long length = strlen(req->body);
        if (fwrite(req->body, length, 1, fp) != 0) {
            strcpy(result->description, "Created");
            result->statusCode = 201;
            strcpy(result->body, "Successfully update the file!");
        }
        else
        {
            strcpy(result->description, "Internal Server Error");
            result->statusCode = 500;
            strcpy(result->body, "Can't write to the file!");
        }
    }
    fclose(fp);
    my_event->r_ptr = result;
    threadpool_add_task(&handler_pool, handle, arg);
}

void doDelete(void *arg)
{
    my_event_t *my_event = (my_event_t *)arg;
    Request *req = my_event->r_ptr;
    Response *result = (Response *)malloc(sizeof(Response));
    result->description = (char *)malloc(DESC_SIZE);
    result->body = (char *)malloc(MAX_SIZE);
    memset(result->body, 0, MAX_SIZE);
    result->httpVersion[0] = req->httpVersion[0];
    result->httpVersion[1] = req->httpVersion[1];
    result->statusCode = 400;
    result->contentLength = 0;
    result->contentType = (char *)malloc(20);
    memset(result->contentType, 0, 20);
    char *filename = (char *)malloc(BUF_SIZE);
    sprintf(filename, PATH, req->path);
    int status = access(filename, F_OK);
    if (status != 0) {
        result->statusCode = 404;
        strcpy(result->description, "Not Found");
        strcpy(result->body, "File doesn't exist!");
    } else {
        remove(filename);
        result->statusCode = 200;
        strcpy(result->description, "OK");
        strcpy(result->body, "Successfully delete the file!");
    }
    my_event->r_ptr = result;
    threadpool_add_task(&handler_pool, handle, arg);
}



void init() {
    threadpool_init(&pool, POOL_NUM);		//初始化线程池
    threadpool_init(&handler_pool, POOL_NUM-1);
}

void handle(void *arg) {
    int sendLen;
    my_event_t *my_event = (my_event_t *)arg;
    char *buffer;
    buffer = printResponse((Response *)my_event->r_ptr);
    sendLen = send(my_event->fd, buffer, strlen(buffer), 0);
    if (sendLen < 0) {
        perror("send error");
        event_del(g_efd, my_event);
        close(my_event->fd);
    } else {
        event_set(my_event, my_event->fd, service, arg);
        event_add(g_efd, EPOLLIN, my_event);
    }
}

void service(int fd, int events, void *arg)
{
    int readLen;
    char *buffer = (char *)malloc(BUF_SIZE);
    my_event_t *my_event = (my_event_t *)arg;
    printf("Receive data \t fd:%d\n", fd);
    readLen = recv(fd, buffer, BUF_SIZE, 0);
    if (readLen <= 0) {
        event_del(g_efd, my_event);
        close(fd);
        return;
    }
    buffer[readLen] = '\0';
    Request *request;
    request = parseRequest(buffer);
    my_event->r_ptr = request;
    if (strcmp(request->method, "GET") == 0) {
        event_del(g_efd, my_event);
        threadpool_add_task(&pool, doGet, arg);
    } else if (strcmp(request->method, "POST") == 0) {
        event_del(g_efd, my_event);
        threadpool_add_task(&pool, doPost, arg);
    } else if (strcmp(request->method, "DELETE") == 0) {
        event_del(g_efd, my_event);
        threadpool_add_task(&pool, doDelete, arg);
    }
    free(buffer);
}
