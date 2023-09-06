#include <fcntl.h>

int set_nonblock(int iSock) {
    int iFlags;

    iFlags = fcntl(iSock, F_GETFL, 0);
    iFlags |= O_NONBLOCK;
    iFlags |= O_NDELAY;
    int ret = fcntl(iSock, F_SETFL, iFlags);
    return ret;
}

void set_addr(const char *pszIP, const unsigned short shPort, struct sockaddr_in *addr) {
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(shPort);
    int nIP = 0;
    if (!pszIP || '\0' == *pszIP || 0 == strcmp(pszIP, "0") || 0 == strcmp(pszIP, "0.0.0.0") ||
        0 == strcmp(pszIP, "*")) {
        nIP = htonl(INADDR_ANY);
    } else {
        nIP = inet_addr(pszIP);
    }
    addr->sin_addr.s_addr = nIP;
}

int open_tcp_socket(const unsigned short shPort, const char *pszIP, bool bReuse) {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd >= 0) {
        if (shPort != 0) {
            if (bReuse) {
                int nReuseAddr = 1;
                setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &nReuseAddr, sizeof(nReuseAddr));
            }
            struct sockaddr_in addr;
            set_addr(pszIP, shPort, &addr);
            int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
            if (ret != 0) {
                close(fd);
                return -1;
            }
        }
    }
    return fd;
}
