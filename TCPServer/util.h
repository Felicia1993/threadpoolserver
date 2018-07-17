#ifndef UTIL
#define UTIL
#include <cstdlib>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer);
ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &sbuff);
void handle_for_sigpipe();
void setSocketNodelay(int fd);
int setSocketNonBlocking(int fd);
int socket_bind_listen(int port);

#endif
