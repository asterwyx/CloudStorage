//
// Created by asterwyx on 11/1/19.
//
#include "util.h"


char *strupr(char *str)
{
    char *result=str;
    for (; *str!='\0'; str++)
        *str = (char)toupper(*str);
    return result;
}


char *getFileType(const char *filename){ //根据扩展名返回文件类型描述
    char sExt[32];
    const char *p_start = filename;
    memset(sExt, 0, sizeof(sExt));
    while (*p_start)
    {
        if (*p_start == '.')
        {
            p_start++;
            strncpy(sExt, p_start, sizeof(sExt));
            break;
        }
        p_start++;
    }

    if (strncmp(sExt, "bmp", 3) == 0)
        return "image/bmp";

    if (strncmp(sExt, "gif", 3) == 0)
        return "image/gif";

    if (strncmp(sExt, "ico", 3) == 0)
        return "image/x-icon";

    if (strncmp(sExt, "jpg", 3) == 0)
        return "image/jpeg";

    if (strncmp(sExt, "avi", 3) == 0)
        return "video/avi";

    if (strncmp(sExt, "css", 3) == 0)
        return "text/css";

    if (strncmp(sExt, "dll", 3) == 0)
        return "application/x-msdownload";

    if (strncmp(sExt, "exe", 3) == 0)
        return "application/x-msdownload";

    if (strncmp(sExt, "dtd", 3) == 0)
        return "text/xml";

    if (strncmp(sExt, "mp3", 3) == 0)
        return "audio/mp3";

    if (strncmp(sExt, "mpg", 3) == 0)
        return "video/mpg";

    if (strncmp(sExt, "png", 3) == 0)
        return "image/png";

    if (strncmp(sExt, "ppt", 3) == 0)
        return "application/vnd.ms-powerpoint";

    if (strncmp(sExt, "xls", 3) == 0)
        return "application/vnd.ms-excel";

    if (strncmp(sExt, "doc", 3) == 0)
        return "application/msword";

    if (strncmp(sExt, "mp4", 3) == 0)
        return "video/mpeg4";

    if (strncmp(sExt, "ppt", 3) == 0)
        return "application/x-ppt";

    if (strncmp(sExt, "wma", 3) == 0)
        return "audio/x-ms-wma";

    if (strncmp(sExt, "wmv", 3) == 0)
        return "video/x-ms-wmv";

    return "text/html";
}

size_t getFileSize(FILE * fp)
{
    rewind(fp);
    int fileSize = 0;
    if (fp == NULL)
        return 0;
    fseek(fp, 0, SEEK_END);
    fileSize = (size_t)ftell(fp);
    rewind(fp);
    return fileSize;
}
