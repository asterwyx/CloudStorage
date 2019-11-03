//
// Created by asterwyx on 11/1/19.
//

#ifndef CLOUDSTORAGESERVER_UTIL_H
#define CLOUDSTORAGESERVER_UTIL_H
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// Lower to upper case
char *strupr(char *str);
size_t getFileSize(FILE *fp);
char *getFileType(const char *filename);
#endif //CLOUDSTORAGESERVER_UTIL_H
