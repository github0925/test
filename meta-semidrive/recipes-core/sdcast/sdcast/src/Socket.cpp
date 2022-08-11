#include "Socket.h"
#include<arpa/inet.h>
#include <linux/ioctl.h>
#include <sstream>

#define PRINT(fmt, args...) printf("socket: [%s] " fmt "\n", __func__, ##args)

Socket::Socket(): mSockFd(-1), mServerFd(-1) {
}

Socket::~Socket() {
    PRINT("called");
}

#define MAGIC_ACT "EEEACT"
int Socket::Act() {
#if 0
    char buf[] = MAGIC_ACT;
    ssize_t ret = write(mSockFd, buf, strlen(buf));
    if (ret != (ssize_t)strlen(buf)) {
        PRINT("failed: ret=%d %d(%s)", (int)ret, errno, strerror(errno));
        return -errno;
    }
#endif
    return 0;
}

int Socket::WaitAct() {
#if 0
    char buf[64];
    ssize_t ret = read(mSockFd, buf, 64);
    if (ret > 64 || ret <= 0) {
        PRINT("failed: ret=%d %d(%s)", (int)ret, errno, strerror(errno));
        return -errno;
    }
#endif
    return 0;
}

ssize_t Socket::Write(const void *buf, size_t len) {
    if (mSockFd <= 0) {
        PRINT("connection is invalid");
        return -1;
    }
    ssize_t sz = write(mSockFd, buf, len);
    if (sz != (ssize_t)len) {
        //PRINT("socket send error:  %d(%s)", errno, strerror(errno));
    }
    WaitAct();
    return sz;
}

ssize_t Socket::Read(void *buf, size_t len) {
    if (mSockFd <= 0) {
        PRINT("connection is invalid");
        return -1;
    }
    ssize_t sz = read(mSockFd, buf, len);
    if (sz <= 0) {
        //PRINT("socket read error: %d(%s)", errno, strerror(errno));
        //TODO
        close(mSockFd);
    }
    Act();
    return sz;
}

int UnixFDTransport::WriteFd(void *ptr, int nbytes, int send_fd) {
    struct msghdr msg;
    struct iovec iov[1];
    struct cmsghdr *pcmsg;
    int ret;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);

    pcmsg = CMSG_FIRSTHDR(&msg);
    pcmsg->cmsg_len = CMSG_LEN(sizeof(int));
    pcmsg->cmsg_level = SOL_SOCKET;
    pcmsg->cmsg_type = SCM_RIGHTS;
    *((int *)CMSG_DATA(pcmsg)) = send_fd;

    ret = sendmsg(mSockFd, &msg, 0);
    // WaitAct();
    return ret;
}

int UnixFDTransport::ReadFd(void *ptr, int nbytes, int *recv_fd) {
    struct msghdr msg;
    struct iovec iov[1];
    struct cmsghdr *pcmsg;
    int ret;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    ret = recvmsg(mSockFd, &msg, 0);
    // Act();
    if (ret <= 0) {
        PRINT("recvmsg error: %d(%s)", errno, strerror(errno));
        return ret;
    }
    if ((pcmsg = CMSG_FIRSTHDR(&msg)) != NULL && (pcmsg->cmsg_len == CMSG_LEN(sizeof(int)))) {
        if (pcmsg->cmsg_level != SOL_SOCKET) {
            PRINT("cmsg_leval is not SOL_SOCKET\n");
            return -1;
        }

        if (pcmsg->cmsg_type != SCM_RIGHTS) {
            PRINT("cmsg_type is not SCM_RIGHTS");
            return -2;
        }

        *recv_fd = *((int *) CMSG_DATA(pcmsg));

        return ret;
    }

    return 0;
}

int DummyFDTransport::ReadFd(void *ptr, int nbytes, int *recv_fd)
{
    return 0;
}

int DummyFDTransport::WriteFd(void *ptr, int nbytes, int send_fd)
{
    return 0;
}


int UnixSocket::Init(int type, int chn, const char *addr) {
    (void) addr;
    if (chn > 31) {
        PRINT("channel %d is invalid , choose another one please!!", chn);
        return -1;
    }
    mType = type;
    std::ostringstream os;
    os << UNIXSTR_PATH << "." << chn;
    mSocketPath = os.str();
    mSockFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (mSockFd < 0) {
        PRINT("socket failed.\n");
        return -1;
    }

    bzero(&serv_un, sizeof(serv_un));
    serv_un.sun_family = AF_UNIX;
    strcpy(serv_un.sun_path, mSocketPath.c_str());
    bool reuseaddr = true;
    setsockopt(mSockFd, SOL_SOCKET , SO_REUSEADDR,
        (const char*)&reuseaddr, sizeof(bool));
    mServerFd = mSockFd;

    if (mType == COMM_TYPE_SERVER) {
        unlink(mSocketPath.c_str());
        int ret = bind(mSockFd, (SA *)&serv_un, sizeof(serv_un));
        if (ret < 0) {
            PRINT("bind failed. errno = %d.\n", errno);
            close (mSockFd);
            return -1;
        }
    }
    mFDTransport = std::make_shared<UnixFDTransport>(mSockFd);

    return 0;
}

int UnixSocket::Connect() {
    int num = 0;
    while (connect(mSockFd,(SA *)&serv_un, sizeof(serv_un)) == -1) {
        if(num < 1000){
            if(num%30 == 0){
                PRINT("Socket::Connect failed. %d (%s)\n", errno, strerror(errno));
                PRINT("Socket::Connect try again..\n");
            }
            num++;
        }
        usleep(1000 * 1000);
    }
    PRINT("connected");
    return 0;
}

int UnixSocket::WaitForClient() {
    if (mType == COMM_TYPE_CLIENT) {
        PRINT("do not make client socket wait for others");
        return -1;
    }
    PRINT("wait for clients: %d", mSockFd);
    if (mServerFd != mSockFd) {
        //close invalid client sock fd
        close(mSockFd);
    }
    int err = listen(mServerFd, CLIENT_MAX);
    if ( err < 0) {
        PRINT("listen error: %d %d", err, errno);
        return -1;
    }

    struct sockaddr_un clnt_un;
    socklen_t clnt_len = sizeof(clnt_un);

    int connfd;
    if ((connfd = accept(mServerFd, (struct sockaddr*)&clnt_un, &clnt_len)) < 0) {
        PRINT("accept error: %d(%s)", errno, strerror(errno));
        return -2;
    }
    PRINT("client [%d] has connected", connfd);
    mHasConnected = true;
    // mSockFd has changed
    mSockFd  = connfd;

    return 0;
}

int NetSocket::Init(int comm_type, int chn, const char *addr) {
    if (chn > 31) {
        PRINT("channel %d is invalid , choose another one please!!", chn);
        return -1;
    }

    mType = comm_type;
    int num = 0;

    while(1) {
        mSockFd = socket(AF_INET, SOCK_STREAM, 0);
        if (mSockFd < 0) {
            PRINT("socket failed.\n");
            usleep(1000 * 1000);
            continue;
        }

        bzero(&serv_in, sizeof(serv_in));
        serv_in.sin_family = AF_INET;

        serv_in.sin_addr.s_addr = inet_addr(addr);
        serv_in.sin_port = htons(AINET_PORT + chn);

        mServerFd = mSockFd;

        if (mType == COMM_TYPE_SERVER) {
            int reuseaddr = 1;
            setsockopt(mSockFd, SOL_SOCKET ,SO_REUSEADDR,(const char*)&reuseaddr, sizeof(reuseaddr));
            int reuseport = 1;
            setsockopt(mSockFd, SOL_SOCKET ,SO_REUSEPORT,(const char*)&reuseport, sizeof(reuseport));

            int ret = bind(mSockFd, (struct sockaddr *)&serv_in, sizeof(serv_in));
            if (ret < 0) {
                PRINT("bind %s failed. errno = %d.\n", addr, errno);
                close(mSockFd);
                usleep(1000 * 1000);
                continue;
            }
        }

        if(connect(mSockFd,(struct sockaddr *)&serv_in, sizeof(serv_in)) == -1) {
            if(num%30 == 0 && num < 500){
                PRINT("Socket::Connect failed. %d (%s),mSockFd is %d\n", errno, strerror(errno),mSockFd);
                PRINT("try again..: addr: %u : %hu\n", serv_in.sin_addr.s_addr, serv_in.sin_port);
            }
            close(mSockFd);
            num++;
            usleep(5 * 1000 * 1000);
            continue;
        }
        break;
    }

    mFDTransport = std::make_shared<DummyFDTransport>(mSockFd);

    return 0;
}

int NetSocket::Connect() {
    PRINT("connected");
    return 0;
}

int NetSocket::WaitForClient() {
    if (mType == COMM_TYPE_CLIENT) {
        PRINT("client socket do not wait for others");
        return -1;
    }
    PRINT("NetSocket::WaitForClient wait for clients: %d", mSockFd);
    if (mServerFd != mSockFd) {
        //close invalid client sock fd
        close(mSockFd);
    }
    int err = listen(mServerFd, CLIENT_MAX);
    if ( err < 0) {
        PRINT("listen error: %d %d", err, errno);
        return -1;
    }

    struct sockaddr_in clnt_in;
    socklen_t clnt_len = sizeof(clnt_in);

    int connfd;
    if ((connfd = accept(mServerFd, (struct sockaddr*)&clnt_in, &clnt_len)) < 0) {
        PRINT("accept error: %d(%s)", errno, strerror(errno));
        return -2;
    }
    PRINT("client [%d] has connected", connfd);
    mHasConnected = true;
    // mSockFd has changed
    mSockFd  = connfd;

    return 0;
}
//RPMSG socket
RpmsgSocket::RpmsgSocket() {

}

RpmsgSocket::~RpmsgSocket() {

}

void RpmsgSocket::DeInit() {
    close(mSockFd);
    close(mServerFd);
}

int RpmsgSocket::Init(int comm_type, int chn, const char *addr) {
    //open rpmsg
    while(1) {
        mSockFd = open("/dev/ivi", O_RDWR);
        if(mSockFd < 0) {
            sleep(1);
            continue;
        }
        break;
    }

    mServerFd = mSockFd;
    mHasConnected = true;
    return 0;
}

int RpmsgSocket::WaitForClient() {
    return 0;
}

int RpmsgSocket::Connect() {
    return 0;
}
