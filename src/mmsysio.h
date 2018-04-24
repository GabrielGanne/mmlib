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

#ifdef _WIN32
/**
 * struct iovec - structure for scatter/gather I/O.
 * @iov_base:   Base address of a memory region for input or output
 * @iov_len:    The size of the memory pointed to by @iov_base
 *
 * Note: on win32 this is guaranteed to alias to WSABUF
 */
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
 * @nblocks:     Currently unused
 * @mtime:       time of last modification
 * @ctime:       time of last status change
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


#if _WIN32
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


MMLIB_API int mm_socket(int domain, int type);
MMLIB_API int mm_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
MMLIB_API int mm_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
MMLIB_API int mm_listen(int sockfd, int backlog);
MMLIB_API int mm_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
MMLIB_API int mm_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
MMLIB_API int mm_setsockopt(int sockfd, int level, int optname,
                            const void *optval, socklen_t optlen);

MMLIB_API int mm_getsockopt(int sockfd, int level, int optname,
                            void *optval, socklen_t* optlen);

MMLIB_API int mm_shutdown(int sockfd, int how);
MMLIB_API ssize_t mm_send(int sockfd, const void *buffer, size_t length, int flags);
MMLIB_API ssize_t mm_recv(int sockfd, void *buffer, size_t length, int flags);
MMLIB_API ssize_t mm_sendmsg(int sockfd, const struct msghdr* msg, int flags);
MMLIB_API ssize_t mm_recvmsg(int sockfd, struct msghdr* msg, int flags);
MMLIB_API int mm_send_multimsg(int sockfd, int vlen,
                               struct mmsock_multimsg *msgvec, int flags);

MMLIB_API int mm_recv_multimsg(int sockfd, int vlen,
                               struct mmsock_multimsg *msgvec, int flags,
                               struct timespec *timeout);

MMLIB_API int mm_getaddrinfo(const char *node, const char *service,
                             const struct addrinfo *hints,
                              struct addrinfo **res);

MMLIB_API int mm_getnamedinfo(const struct sockaddr *addr, socklen_t addrlen,
                              char *host, socklen_t hostlen,
                              char *serv, socklen_t servlen, int flags);

MMLIB_API void mm_freeaddrinfo(struct addrinfo *res);
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

MMLIB_API int mm_poll(struct mm_pollfd *fds, int nfds, int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
