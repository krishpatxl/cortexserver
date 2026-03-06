#ifndef FILES_H
#define FILES_H

const char *guess_mime_type(const char *path);
int send_file_response(int fd, const char *path);

#endif