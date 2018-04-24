/*
   @mindmaze_header@
*/
#ifndef MMSYSIO_H
#define MMSYSIO_H

#include <stddef.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <io.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#endif

#include "mmpredefs.h"

/**
 * struct iovec - structure for scatter/gather I/O.
 * @iov_base:   Base address of a memory region for input or output
 * @iov_len:    The size of the memory pointed to by @iov_base
 */

#ifdef _WIN32
// structure for scatter-gather RW operation
// on win32 this is guaranteed to alias to WSABUF
struct iovec {
	unsigned long iov_len;
	void* iov_base;
};

// Not defined on windows platform, so we can keep the standard name
// without mm_ prefix
typedef unsigned long long uid_t;
typedef unsigned long long gid_t;

#ifdef _MSC_VER
#  ifndef _SSIZE_T_DEFINED
#    define _SSIZE_T_DEFINED
#    undef ssize_t
#    ifdef _WIN64
  typedef __int64 ssize_t;
#    else
  typedef int ssize_t;
#    endif /* _WIN64 */
#  endif /* _SSIZE_T_DEFINED */
#endif

typedef long long mm_off_t;
typedef unsigned long long mm_dev_t;
typedef struct {
	unsigned long long id_low;
	unsigned long long id_high;
} mm_ino_t;

#else /* _WIN32 */

typedef off_t mm_off_t;
typedef dev_t mm_dev_t;
typedef ino_t mm_ino_t;

#endif /* _WIN32 */

/**
 * mm_ino_equal() - test equality between two mm_ino_t
 * @a:  first mm_ino_t operand
 * @b:  second mm_ino_t operand
 *
 * Return: 1 if equal, 0 otherwise
 */
static inline
int mm_ino_equal(mm_ino_t a, mm_ino_t b)
{
#ifdef _WIN32
	return ((a.id_low == b.id_low) && (a.id_high == b.id_high));
#else
	return (a == b);
#endif
}


#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 *                          File manipulation                             *
 **************************************************************************/

#ifdef _WIN32

#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#endif
#ifndef O_WRONLY
#define O_WRONLY _O_WRONLY
#endif
#ifndef O_RDWR
#define O_RDWR _O_RDWR
#endif
#ifndef O_APPEND
#define O_APPEND _O_APPEND
#endif
#ifndef O_CREAT
#define O_CREAT _O_CREAT
#endif
#ifndef O_TRUNC
#define O_TRUNC _O_TRUNC
#endif
#ifndef O_EXCL
#define O_EXCL _O_EXCL
#endif

#ifndef S_IFMT
#define S_IFMT _S_IFMT
#endif
#ifndef S_IFDIR
#define S_IFDIR _S_IFDIR
#endif
#ifndef S_IFREG
#define S_IFREG _S_IFREG
#endif
#define S_IFLNK (_S_IFREG|_S_IFCHR)

#define S_ISTYPE(mode, mask)  (((mode) & S_IFMT) == (mask))
#ifndef S_ISDIR
#define S_ISDIR(mode)   S_ISTYPE((mode), S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(mode)   S_ISTYPE((mode), S_IFREG)
#endif
#ifndef S_ISLNK
#define S_ISLNK(mode)   S_ISTYPE((mode), S_IFLNK)
#endif

#ifndef S_IRUSR
#define S_IRUSR _S_IREAD
#endif
#ifndef S_IWUSR
#define S_IWUSR _S_IWRITE
#endif
#ifndef S_IXUSR
#define S_IXUSR _S_IEXEC
#endif
#ifndef S_IRWXU
#define S_IRWXU (S_IRUSR|S_IWUSR|S_IXUSR)
#endif

#ifndef F_OK
#define F_OK 0x00
#endif
#ifndef X_OK
#define X_OK 0x01
#endif
#ifndef W_OK
#define W_OK 0x02
#endif
#ifndef R_OK
#define R_OK 0x04
#endif

#endif /* _WIN32 */

/* file types returned when scanning a directory */
#define MM_DT_UNKNOWN 0
#define MM_DT_FIFO (1 << 1)
#define MM_DT_CHR  (1 << 2)
#define MM_DT_BLK  (1 << 3)
#define MM_DT_DIR  (1 << 4)
#define MM_DT_REG  (1 << 5)
#define MM_DT_LNK  (1 << 6)
#define MM_DT_SOCK (1 << 7)
#define MM_DT_ANY  (0XFF)

#define MM_RECURSIVE    (1 << 31)
#define MM_FAILONERROR  (1 << 30)
#define MM_NOFOLLOW     (1 << 29)

/**
 * struct mm_stat - file status data
 * @dev:        Device ID of device containing file
 * @ino:        File serial number
 * @mode:       Mode of file (Indicate file type and permission)
 * @nlink:      Number of hard links to the file
 * @uid:        Currently unused
 * @gid:        Currently unused
 * @size:       For regular files, the file size in bytes.
 *              For symbolic links, the length in bytes of the UTF-8
 *              pathname contained in the symbolic link (including null
 *              termination).
 * @blksize:    Currently unused
 * @blkcnt:     Currently unused
 * mtime:       time of last modification
 * ctime:       time of last status change
 */
struct mm_stat {
	mm_dev_t dev;
	mm_ino_t ino;
	mode_t mode;
	int nlink;
	uid_t uid;
	gid_t gid;
	mm_off_t size;
	size_t blksize;
	size_t nblocks;
	time_t mtime;
	time_t ctime;
};

MMLIB_API int mm_open(const char* path, int oflag, int mode);
MMLIB_API int mm_close(int fd);
MMLIB_API int mm_fsync(int fd);
MMLIB_API ssize_t mm_read(int fd, void* buf, size_t nbyte);
MMLIB_API ssize_t mm_write(int fd, const void* buf, size_t nbyte);
MMLIB_API mm_off_t mm_seek(int fd, mm_off_t offset, int whence);
MMLIB_API int mm_ftruncate(int fd, mm_off_t length);
MMLIB_API int mm_fstat(int fd, struct mm_stat* buf);
MMLIB_API int mm_stat(const char* path, struct mm_stat* buf, int flags);
MMLIB_API int mm_check_access(const char* path, int amode);

MMLIB_API int mm_dup(int fd);
MMLIB_API int mm_dup2(int fd, int newfd);
MMLIB_API int mm_pipe(int pipefd[2]);

MMLIB_API int mm_unlink(const char* path);
MMLIB_API int mm_link(const char* oldpath, const char* newpath);
MMLIB_API int mm_symlink(const char* oldpath, const char* newpath);
MMLIB_API int mm_readlink(const char* path, char* buf, size_t bufsize);

MMLIB_API int mm_mkdir(const char* path, int mode, int flags);
MMLIB_API int mm_chdir(const char* path);
MMLIB_API int mm_rmdir(const char* path);
MMLIB_API int mm_remove(const char* path, int flags);


/**************************************************************************
 *                      Directory navigation                              *
 **************************************************************************/
typedef struct mm_dirstream MMDIR;

struct mm_dirent {
	size_t reclen;  /* this record length */
	int type;       /* file type (see above) */
	int id;         /* reserved for later use */
	char name[];    /* Null-terminated filename */
};

MMLIB_API MMDIR* mm_opendir(const char* path);
MMLIB_API void mm_closedir(MMDIR* dir);
MMLIB_API void mm_rewinddir(MMDIR* dir);
MMLIB_API const struct mm_dirent* mm_readdir(MMDIR* dir, int * status);


/**************************************************************************
 *                             Process spawning                           *
 **************************************************************************/
#ifdef _WIN32
typedef DWORD mm_pid_t;
#else
typedef pid_t mm_pid_t;
#endif

/**
 * struct mm_remap_fd - file descriptor mapping for child creation
 * @child_fd:   file descriptor in the child
 * @parent_fd:  file descriptor in the parent process to @child_fd must be
 *              mapped. If @child_fd must be specifically closed in the
 *              child, @parent_fd can be set to -1;
 *
 * Use in combination of mm_spawn(), this structure is meant to be in an
 * array that define the file descriptor remapping in child.
 */
struct mm_remap_fd {
	int child_fd;
	int parent_fd;
};

#define MM_SPAWN_DAEMONIZE 0x00000001
#define MM_SPAWN_KEEP_FDS  0x00000002  // Keep all inheritable fd in child

MMLIB_API int mm_spawn(mm_pid_t* child_pid, const char* path,
                       int num_map, const struct mm_remap_fd* fd_map,
                       int flags, char* const* argv, char* const* envp);

#define MM_WSTATUS_CODEMASK     0x000000FF
#define MM_WSTATUS_EXITED       0x00000100
#define MM_WSTATUS_SIGNALED     0x00000200

MMLIB_API int mm_wait_process(mm_pid_t pid, int* status);


/**************************************************************************
 *                            memory mapping                              *
 **************************************************************************/

#define MM_MAP_READ     0x00000001
#define MM_MAP_WRITE	0x00000002
#define MM_MAP_EXEC     0x00000004
#define MM_MAP_SHARED   0x00000008

#define MM_MAP_RDWR	(MM_MAP_READ | MM_MAP_WRITE)
#define MM_MAP_PRIVATE  0x00000000


MMLIB_API void* mm_mapfile(int fd, mm_off_t offset, size_t len, int mflags);
MMLIB_API int mm_unmap(void* addr);

MMLIB_API int mm_shm_open(const char* name, int oflag, int mode);
MMLIB_API int mm_anon_shm(void);
MMLIB_API int mm_shm_unlink(const char* name);


/**************************************************************************
 *                        Interprocess communication                      *
 **************************************************************************/

/**
 * struct mmipc_msg - structure for IPC message
 * @iov:        scatter/gather array
 * @fds:        array of file descriptor to pass/receive
 * @num_iov:    number of element in @iov
 * @num_fds:    number of file descriptors in @fds
 * @flags:      flags on received message
 * @num_fds_max: maximum number of file descriptors in @fds
 * @reserved:   reserved for future use (must be NULL)
 */
struct mmipc_msg {
	struct iovec* iov;
	int* fds;
	int num_iov;
	int num_fds;
	int flags;
	int num_fds_max;
	void* reserved;
};

struct mmipc_srv;

MMLIB_API struct mmipc_srv* mmipc_srv_create(const char* addr);
MMLIB_API void mmipc_srv_destroy(struct mmipc_srv* srv);
MMLIB_API int mmipc_srv_accept(struct mmipc_srv* srv);
MMLIB_API int mmipc_connect(const char* addr);
MMLIB_API int mmipc_connected_pair(int fds[2]);
MMLIB_API ssize_t mmipc_sendmsg(int fd, const struct mmipc_msg* msg);
MMLIB_API ssize_t mmipc_recvmsg(int fd, struct mmipc_msg* msg);


/**************************************************************************
 *                          Network communication                         *
 **************************************************************************/



/**
 * struct msghdr - structure for socket message
 * @msg_name:       optional address
 * @msg_namelen:    size of address
 * @msg_iov:        scatter/gather array
 * @msg_iovlen:     number of element in @msg_iov (beware of type, see NOTE)
 * @msg_control:    Ancillary data
 * @msg_controllen: length of ancillary data
 * @msg_flags:      flags on received message
 *
 * NOTE:
 * Although Posix mandates that @msg_iovlen is int, many platform do not
 * respect this: it is size_t on Linux, unsigned int on Darwin, int on
 * freebsd. It is safer to consider @msg_iovlen is defined as size_t in
 * struct (avoid one hole due to alignment on 64bits platform) and always
 * manipulate as int (forcing the cast).
 */

#if _WIN32
struct msghdr {
	void*         msg_name;
	socklen_t     msg_namelen;
	struct iovec* msg_iov;
	size_t        msg_iovlen;
	void*         msg_control;
	socklen_t     msg_controllen;
	int           msg_flags;
};
#endif


/**
 * struct mmsock_multimsg - structure for transmitting multiple messages
 * @msg:        message
 * @datalen:    number of received byte for @msg
 *
 * This should alias with struct mmsghdr on system supporting recvmmsg()
 */
struct mmsock_multimsg {
	struct msghdr msg;
	unsigned int  datalen;
};


/**
 * mm_socket() - create an endpoint for communication
 * @domain:     communications domain in which a socket is to be created
 * @type:       type of socket to be created
 *
 * The mm_socket() function creates an unbound socket in a communications
 * domain, and return a file descriptor that can be used in later function
 * calls that operate on sockets. The allocated file descriptor is the
 * lowest numbered file descriptor not currently open for that process.
 *
 * The @domain argument specifies the address family used in the
 * communications domain. The address families supported by the system are
 * system-defined. However you can expect that the following to be
 * available:
 *
 * %AF_UNSPEC
 *   The address family is unspecified.
 * %AF_INET
 *   The Internet Protocol version 4 (IPv4) address family.
 * %AF_INET6
 *   The Internet Protocol version 6 (IPv6) address family.
 *
 * The @type argument specifies the socket type, which determines the
 * semantics of communication over the socket. The following socket types
 * are defined; Some systems may specify additional types:
 *
 * SOCK_STREAM
 *   Provides sequenced, reliable, bidirectional, connection-mode byte
 *   streams
 * SOCK_DGRAM
 *   Provides datagrams, which are connectionless-mode, unreliable messages
 *   of fixed maximum length.
 *
 * Return: a non-negative integer representing the file descriptor in case
 * of success. Otherwise -1 is returned with error state set accordingly.
 */
MMLIB_API int mm_socket(int domain, int type);


/**
 * mm_bind() - bind a name to a socket
 * @sockfd:     file descriptor of the socket to be bound
 * @addr:       points to a &struct sockaddr containing the address bind
 * @addrlen:    length of the &struct sockaddr pointed to by @addr
 *
 * This assigns a local socket address @addr to a socket identified by
 * descriptor @sockfd that has no local socket address assigned. Socket
 * created with mm_socket() are initially unnamed; they are identified only
 * by their address family.
 *
 * Return: 0 in case of success, -1 otherwise with error state set
 * accordingly.
 */
MMLIB_API int mm_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);


/**
 * mm_gesockname() - returns  the  current address to which the socket sockfd is bound
 * @sockfd:     file descriptor to which the socket is bound
 * @addr:       points to a &struct sockaddr containing the bound address
 * @addrlen:    length of the &struct sockaddr pointed to by @addr
 *
 * getsockname() returns the current address to which the socket sockfd is bound,
 * in the buffer pointed to by @addr.
 * The @addrlen argument should be initialized to indicate the amount of space
 * (in bytes) pointed to by addr. On return it contains the actual size of the
 * socket address.
 *
 * Return: 0 in case of success, -1 otherwise with error state set
 * accordingly.
 */
MMLIB_API int mm_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);


/**
 * mm_listen() - listen for socket connections
 * @sockfd:     file descriptor of the socket that must listen
 * @backlog:    hint for the queue limit
 *
 * This mark a connection-mode socket, specified by the @sockfd argument, as
 * accepting connections. The @backlog argument provides a hint to the
 * implementation which the implementation shall use to limit the number of
 * outstanding connections in the socket's listen queue.
 *
 * return: 0 in case of success, -1 otherwise with error state set
 * accordingly.
 */
MMLIB_API int mm_listen(int sockfd, int backlog);


/**
 * mm_accept() - accept a new connection on a socket
 * @sockfd:     file descriptor of a listening socket
 * @addr:       NULL or pointer  to &struct sockaddr containing the address
 *              of accepted socket
 * @addrlen:    a pointer to a &typedef socklen_t object which on input
 *              specifies the length of the supplied &struct sockaddr
 *              structure, and on output specifies the length of the stored
 *              address. It can be NULL if @addr is NULL.
 *
 * This extracts the first connection on the queue of pending connections,
 * create a new socket with the same socket type protocol and address family
 * as the specified socket, and allocate a new file descriptor for that
 * socket. The allocated file descriptor is the lowest numbered file
 * descriptor not currently open for that process.
 *
 * Return: a non-negative integer representing the file descriptor in case
 * of success. Otherwise -1 is returned with error state set accordingly.
 */
MMLIB_API int mm_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen);


/**
 * mm_connect() - connect a socket to a peer
 * @sockfd:     file descriptor of the socket
 * @addr:       pointer  to &struct sockaddr containing the peer address
 * @addrlen:    length of the supplied &struct sockaddr pointed by @addr
 *
 * This attempt to make a connection on a connection-mode socket or to set
 * or reset the peer address of a connectionless-mode socket.  If the socket
 * has not already been bound to a local address, mm_connect() shall bind it
 * to an address which is an unused local address.
 *
 * If the initiating socket is not connection-mode, then mm_connect() sets
 * the socket's peer address, and no connection is made. For %SOCK_DGRAM
 * sockets, the peer address identifies where all datagrams are sent on
 * subsequent mm_send() functions, and limits the remote sender for
 * subsequent mm_recv() functions.
 *
 * If the initiating socket is connection-mode, then mm_connect() attempts
 * to establish a connection to the address specified by the @addr argument.
 * If the connection cannot be established immediately and the call will
 * block for up to an unspecified timeout interval until the connection is
 * established.
 *
 * return: 0 in case of success, -1 otherwise with error state set
 * accordingly.
 */
MMLIB_API int mm_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);


/**
 * mm_setsockopt() - set the socket options
 * @sockfd:     file descriptor of the socket
 * @level:      protocol level at which the option resides
 * @optname:    option name
 * @optval:     pointer to option value
 * @optlen:     size of the option value
 *
 * This sets the option specified by the @optname argument, at the protocol
 * level specified by the @level argument, to the value pointed to by the
 * @optval argument for the socket associated with the file descriptor
 * specified by the @sockfd argument.
 *
 * The level argument specifies the protocol level at which the option
 * resides. To set options at the socket level, specify the level argument
 * as %SOL_SOCKET. To set options at other levels, supply the appropriate
 * level identifier for the protocol controlling the option. For example, to
 * indicate that an option is interpreted by the TCP (Transport Control
 * Protocol), set level to %IPPROTO_TCP.
 *
 * return: 0 in case of success, -1 otherwise with error state set
 * accordingly.
 */
MMLIB_API int mm_setsockopt(int sockfd, int level, int optname,
                            const void *optval, socklen_t optlen);


/**
 * mm_getsockopt() - get the socket options
 * @sockfd:     file descriptor of the socket
 * @level:      protocol level at which the option resides
 * @optname:    option name
 * @optval:     pointer to option value
 * @optlen:     pointer to size of the option value on input, actual length
 *              of option value on output
 *
 * This function retrieves the value for the option specified by the
 * @optname argument for the socket specified by the socket argument. If
 * the size of the option value is greater than @optlen, the value stored
 * in the object pointed to by the @optval argument shall be silently
 * truncated. Otherwise, the object pointed to by the @optlen argument
 * shall be modified to indicate the actual length of the value.
 *
 * return: 0 in case of success, -1 otherwise with error state set
 * accordingly.
 */
MMLIB_API int mm_getsockopt(int sockfd, int level, int optname,
                            void *optval, socklen_t* optlen);


/**
 * mm_shutdown() - shut down socket send and receive operations
 * @sockfd:     file descriptor of the socket
 * @how:        type of shutdown
 *
 * This causes all or part of a full-duplex connection on the socket associated
 * with the file descriptor socket to be shut down. The type of shutdown is
 * controlled by @how which can be one of the following values:
 *
 * SHUT_RD
 *   Disables further receive operations.
 * SHUT_WR
 *   Disables further send operations.
 * SHUT_RDWR
 *   Disables further send and receive operations.
 *
 * return: 0 in case of success, -1 otherwise with error state set
 * accordingly.
 */
MMLIB_API int mm_shutdown(int sockfd, int how);


/**
 * mm_send() - send a message on a connected socket
 * @sockfd:     socket file descriptor.
 * @buffer:     buffer containing the message to send.
 * @length:     the length of the message in bytes
 * @flags:      type of message transmission
 *
 * This initiates transmission of a message from the specified socket to its
 * peer. The mm_send() function sends a message only when the socket is
 * connected (including when the peer of a connectionless socket has been
 * set via mm_connect()).
 *
 * @flags specifies the type of message transmission. If flags contains
 * MSG_OOB, the call send out-of-band data on sockets that support
 * out-of-band communications. The significance and semantics of out-of-band
 * data are protocol-specific.
 *
 * The length of the message to be sent is specified by the length argument.
 * If the message is too long to pass through the underlying protocol,
 * mm_send() will fail and no data shall be transmitted (This is typically
 * the case of datagram protocol, like UDP). If space is not
 * available at the sending socket to hold the message to be transmitted,
 * mm_send() will block until space is available. In the case of a stream
 * protocol (like TCP), there are possibility that the sent data is actually
 * smaller than requested (for example due to early interruption because of
 * signal delivery).
 *
 * Return: the number of bytes actually sent in case of success, -1
 * otherwise with error state set accordingly.
 */
MMLIB_API ssize_t mm_send(int sockfd, const void *buffer, size_t length, int flags);


/**
 * mm_recv() - receive a message from a socket
 * @sockfd:     socket file descriptor.
 * @buffer:     buffer containing the message to receive.
 * @length:     the size of buffer pointed by @buffer
 * @flags:      type of message reception
 *
 * This receives a message from a connection-mode or connectionless-mode
 * socket. It is normally used with connected sockets because it does not
 * permit the application to retrieve the source address of received data.
 *
 * @flags specifies the type of message reception. Values of this argument
 * are formed by logically OR'ing zero or more of the following values:
 *
 * %MSG_PEEK
 *   Peeks at an incoming message. The data is treated as unread and the
 *   next mm_recv() or similar function shall still return this data.
 * %MSG_OOB
 *   Requests out-of-band data. The significance and semantics of
 *   out-of-band data are protocol-specific.
 * %MSG_WAITALL
 *   On SOCK_STREAM sockets this requests that the function block until the
 *   full amount of data can be returned. The function may return the
 *   smaller amount of data if the socket is a message-based socket, if a
 *   signal is caught, if the connection is terminated, if MSG_PEEK was
 *   specified, or if an error is pending for the socket.
 *
 * The mm_recv() function return the length of the message written to the
 * buffer pointed to by the buffer argument. For message-based sockets, such
 * as SOCK_DGRAM and SOCK_SEQPACKET, the entire message will be read in a
 * single operation. If a message is too long to fit in the supplied buffer,
 * and %MSG_PEEK is not set in the @flags argument, the excess bytes will be
 * discarded. For stream-based sockets, such as SOCK_STREAM, message
 * boundaries will be ignored. In this case, data is returned to the
 * user as soon as it becomes available, and no data will be discarded.
 *
 * If the MSG_WAITALL flag is not set, data will be returned only up to the
 * end of the first message.
 *
 * If no messages are available at the socket, mm_recv() will block until a
 * message arrives.
 *
 * Return: the number of bytes actually received in case of success, -1
 * otherwise with error state set accordingly.
 */
MMLIB_API ssize_t mm_recv(int sockfd, void *buffer, size_t length, int flags);


/**
 * mm_sendmsg() - send a message on a socket using a message structure
 * @sockfd:     socket file descriptor.
 * @msg:        message structure containing both the destination address
 *              (if any) and the buffers for the outgoing message.
 * @flags:      type of message transmission
 *
 * This functions send a message through a connection-mode or
 * connectionless-mode socket. If the socket is connectionless-mode, the
 * message will be sent to the address specified by @msg. If the socket
 * is connection-mode, the destination address in @msg is ignored.
 *
 * The @msg->msg_iov and @msg->msg_iovlen fields of message specify zero or
 * more buffers containing the data to be sent. @msg->msg_iov points to an
 * array of iovec structures; @msg->msg_iovlen is set to the dimension of
 * this array. In each &struct iovec structure, the &iovec.iov_base field
 * specifies a storage area and the &iovec.iov_len field gives its size in
 * bytes. Some of these sizes can be zero.  The data from each storage area
 * indicated by @msg.msg_iov is sent in turn.
 *
 * Excepting for the specification of the message buffers and destination
 * address, the behavior of mm_sendmsg() is the same as mm_send().
 *
 * Return: the number of bytes actually sent in case of success, -1
 * otherwise with error state set accordingly.
 */
MMLIB_API ssize_t mm_sendmsg(int sockfd, const struct msghdr* msg, int flags);


/**
 * mm_recvmsg() - receive a message from a socket using a message structure
 * @sockfd:     socket file descriptor.
 * @msg:        message structure containing both the source address (if
 *              set) and the buffers for the inbound message.
 * @flags:      type of message reception
 *
 * This function receives a message from a connection-mode or connectionless-mode
 * socket. It is normally used with connectionless-mode sockets because it
 * permits the application to retrieve the source address of received data.
 *
 * In the &struct mmsock_msg structure, the &msghdr.msg_name and
 * &msghdr.msg_namelen members specify the source address if the socket is
 * unconnected. If the socket is connected, those members are ignored. The
 * @msg->msg_name may be a null pointer if no names are desired or required.
 * The @msg->msg_iov and @msg->msg_iovlen fields are used to specify where
 * the received data will be stored. @msg->msg_iov points to an array of
 * &struct iovec structures; @msg->msg_iovlen is set to the dimension of
 * this array.  In each &struct iovec structure, the &iovec.iov_base field
 * specifies a storage area and the &iovec.iov_len field gives its size in
 * bytes. Each storage area indicated by @msg.msg_iov is filled with
 * received data in turn until all of the received data is stored or all of
 * the areas have been filled.
 *
 * The recvmsg() function returns the total length of the message. For
 * message-based sockets, such as SOCK_DGRAM and SOCK_SEQPACKET, the entire
 * message is read in a single operation. If a message is too long to fit in
 * the supplied buffers, and %MSG_PEEK is not set in the flags argument, the
 * excess bytes will be discarded, and %MSG_TRUNC will be set in
 * @msg->flags. For stream-based sockets, such as SOCK_STREAM, message
 * boundaries are ignored. In this case, data will be returned to the user
 * as soon as it becomes available, and no data will be discarded.
 *
 * Excepting for the specification of message buffers and source address,
 * the behavior of mm_recvmsg() is the same as mm_rec().
 *
 * Return: the number of bytes actually received in case of success, -1
 * otherwise with error state set accordingly.
 */
MMLIB_API ssize_t mm_recvmsg(int sockfd, struct msghdr* msg, int flags);


/**
 * mm_send_multimsg() - send multiple messages on a socket
 * @sockfd:     socket file descriptor.
 * @vlen:       size of @msgvec array
 * @msgvec:     pointer to an array of &struct mmsock_multimsg structures
 * @flags:      type of message transmission
 *
 * This function is an extension of mm_sendmsg that allows the
 * caller to send multiple messages to a socket using a single
 * call. This is equivalent to call mm_sendmsg() in a loop for each element
 * in @msgvec.
 *
 * On return from mm_sendmmsg(), the &struct mmsock_multimsg.data_len fields
 * of successive elements of @msgvec are updated to contain the number of
 * bytes transmitted from the corresponding &struct mmsock_multimsg.msg.
 *
 * Return: On success, it returns the number of messages sent from @msgvec;
 * if this is less than @vlen, the caller can retry with a further
 * mm_sendmmsg() call to send the remaining messages. On error, -1 is
 * returned and the error state is set accordingly.
 */
MMLIB_API int mm_send_multimsg(int sockfd, int vlen,
                               struct mmsock_multimsg *msgvec, int flags);


/**
 * mm_recv_multimsg() - receive multiple messages from a socket
 * @sockfd:     socket file descriptor.
 * @vlen:       size of @msgvec array
 * @msgvec:     pointer to an array of &struct mmsock_multimsg structures
 * @flags:      type of message reception
 * @timeout:    timeout for receive operation. If NULL, the operation blocks
 *              indefinitively
 *
 * This function is an extension of mm_sendmsg that allows the
 * caller to receive multiple messages from a socket using a single
 * call. This is equivalent to call mm_recvmsg() in a loop for each element
 * in @msgvec with loop break if @timeout has been reached.
 *
 * On return from mm_recvmmsg(), the &struct mmsock_multimsg.data_len fields
 * of successive elements of @msgvec are updated to contain the number of
 * bytes received from the corresponding &struct mmsock_multimsg.msg.
 *
 * Return: On success, it returns the number of messages received from @msgvec;
 * if this is less than @vlen, the caller can retry with a further
 * mm_recvmmsg() call to receive the remaining messages. On error, -1 is
 * returned and the error state is set accordingly.
 */
MMLIB_API int mm_recv_multimsg(int sockfd, int vlen,
                               struct mmsock_multimsg *msgvec, int flags,
                               struct timespec *timeout);

/**
 * mm_getaddrinfo() - get address information
 * @node:       descriptive name or address string (can be NULL)
 * @service:    string identifying the requested service (can be NULL)
 * @hints:      input values that may direct the operation by providing
 *              options and by limiting the returned information (can be
 *              NULL)
 * @res:        return value that will contain the resulting linked list of
 *              &struct addrinfo structures.
 *
 * Same as getaddrinfo() from POSIX excepting that mmlib error state will be
 * set in case of error.
 *
 * Return: 0 in case of success, -1 otherwise with error state set
 * accordingly.
 */
MMLIB_API int mm_getaddrinfo(const char *node, const char *service,
                             const struct addrinfo *hints,
                              struct addrinfo **res);

/**
 * mm_getnamedinfo() - get name information
 * @addr:       socket address
 * @addrlen:    size of @addr
 * @host:       buffer receiving the host name
 * @hostlen:    size of buffer in @host
 * @serv:       buffer receiving the service name
 * @servlen:    size of buffer in @serv
 * @flags:      control of processing of mm_getnamedinfo()
 *
 * Same as getnamedinfo() from POSIX excepting that mmlib error state will
 * be set in case of error.
 *
 * Return: 0 in case of success, -1 otherwise with error state set
 * accordingly.
 */
MMLIB_API int mm_getnamedinfo(const struct sockaddr *addr, socklen_t addrlen,
                              char *host, socklen_t hostlen,
                              char *serv, socklen_t servlen, int flags);

/**
 * mm_freeaddrinfo() - free linked list of address
 * @res:        linked list of addresses returned by @mm_getaddrinfo()
 *
 * Deallocate linked list of address allocated by a successfull call to
 * mm_getaddrinfo(). If @res is NULL, mm_getnamedinfo() do nothing.
 */
MMLIB_API void mm_freeaddrinfo(struct addrinfo *res);


/**
 * mm_create_sockclient() - Create a client socket and connect it to server
 * @uri:        URI indicating the ressource to connect
 *
 * This functions resolves URI resource, create a socket and try to connect
 * to resource. The service, protocol, port, hostname will be parsed from
 * @uri and the resulting socket will be configured and connected to the
 * resource.
 *
 * Return: a non-negative integer representing the file descriptor in case
 * of success. Otherwise -1 is returned with error state set accordingly.
 */
MMLIB_API int mm_create_sockclient(const char* uri);


#if defined(_WIN32)
#  define MM_POLLIN  0x0100
#  define MM_POLLOUT 0x0010
#else /*  defined(_WIN32) */
#  define MM_POLLIN  POLLIN
#  define MM_POLLOUT POLLOUT
#endif /* defined(_WIN32) */

#if defined(_WIN32)
struct mm_pollfd {
	int   fd;       /* file descriptor  */
	short events;   /* requested events */
	short revents;  /* returned events  */
};
#else
#define mm_pollfd pollfd
#endif

/**
 * mm_poll() - waits for one of a set of file descriptors to become ready to perform I/O.
 * @fds           array of struct pollfd. See below.
 * @nfds          number of @fds passed in argument
 * @timeout_ms    number of milliseconds that poll() should block waiting
 *
 * fd should be a *socket* file descriptor.
 * if timout_ms is set to 0, the call will return immediatly
 *                           even if no file descriptors are ready
 * if timout_ms is negative, the call will block indefinitely
 *
 * events contains a mask on trevents with the following values:
 *   MM_POLLIN   // there is data to read
 *   MM_POLLOUT  // writing is now possible
 *
 * revents will contain the output events flags, a combination of
 * MM_POLLIN and MM_POLLOUT, or 0 if unset.
 *
 * Return:
 *  * (>0) On success, the number of fds on which an event was raised
 *  * (=0) zero if poll() returned because the timeout was reached
 *  * (<0) a negative value on error
 */
MMLIB_API int mm_poll(struct mm_pollfd *fds, int nfds, int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
