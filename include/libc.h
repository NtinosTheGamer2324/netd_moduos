// libc.h
#ifndef ERRNO_H
#define ERRNO_H

/* Linux-compatible errno values (subset) */
#define EPERM   1
#define ENOENT  2
#define ESRCH   3
#define EINTR   4
#define EIO     5
#define ENXIO   6
#define E2BIG   7
#define ENOEXEC 8
#define EBADF   9
#define ECHILD  10
#define EAGAIN  11
#define ENOMEM  12
#define EACCES  13
#define EFAULT  14
#define EBUSY   16
#define EEXIST  17
#define EXDEV   18
#define ENODEV  19
#define ENOTDIR 20
#define EISDIR  21
#define EINVAL  22
#define ENFILE  23
#define EMFILE  24
#define ENOTTY  25
#define ETXTBSY 26
#define EFBIG   27
#define ENOSPC  28
#define ESPIPE  29
#define EROFS   30
#define EMLINK  31
#define EPIPE   32
#define EDOM    33
#define ERANGE  34

#define ENOSYS  38

#endif // ERRNO
/* Linux-ish errno variable (per-process). Since ModuOS user programs are
 * typically single translation units, a header-defined static is sufficient.
 */
static int errno;

#pragma once
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#ifndef STRING_H
#define STRING_H
/* Local string helpers (strlen, strcmp, etc.) */
#pragma once
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* --- 1. SEARCH & COMPARISON --- */

static inline size_t strlen(const char *str) {
    const char *s = str;
    while (*s) ++s;
    return s - str;
}

static inline int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) { ++s1; ++s2; }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

static inline int strncmp(const char *s1, const char *s2, size_t n) {
    if (n == 0) return 0;
    while (n-- > 0 && *s1 && (*s1 == *s2)) {
        if (n == 0) break;
        ++s1; ++s2;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

static inline int strcasecmp(const char *s1, const char *s2) {
    while (*s1) {
        int c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
        int c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
        if (c1 != c2) return c1 - c2;
        s1++; s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

static inline char *strchr(const char *str, int c) {
    while (*str) { if (*str == (char)c) return (char *)str; ++str; }
    return (c == '\0') ? (char *)str : NULL;
}

static inline char *strrchr(const char *str, int c) {
    const char *last = NULL;
    while (*str) { if (*str == (char)c) last = str; ++str; }
    return (c == '\0') ? (char *)str : (char *)last;
}

static inline char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    for (; *haystack; haystack++) {
        const char *h = haystack, *n = needle;
        while (*h && *n && (*h == *n)) { h++; n++; }
        if (!*n) return (char *)haystack;
    }
    return NULL;
}

/* --- 2. THE TOKENIZER (Crucial for DB parsing) --- */

static inline char *strtok(char *str, const char *delim) {
    static char *last = NULL;
    if (str) last = str;
    if (!last || *last == '\0') return NULL;

    // Skip leading delimiters
    while (*last && strchr(delim, *last)) last++;
    if (*last == '\0') return NULL;

    char *token = last;
    // Find end of token
    while (*last && !strchr(delim, *last)) last++;
    
    if (*last != '\0') {
        *last = '\0';
        last++;
    }
    return token;
}

/* --- 3. SAFE COPY & CONCAT (BSD Style) --- */

static inline size_t strlcpy(char *dest, const char *src, size_t size) {
    size_t i = 0;
    size_t src_len = strlen(src);
    if (size > 0) {
        for (i = 0; i < size - 1 && src[i] != '\0'; i++) dest[i] = src[i];
        dest[i] = '\0';
    }
    return src_len;
}

static inline size_t strlcat(char *dest, const char *src, size_t size) {
    size_t dest_len = strlen(dest);
    size_t src_len = strlen(src);
    if (dest_len >= size) return size + src_len;
    strlcpy(dest + dest_len, src, size - dest_len);
    return dest_len + src_len;
}

static inline char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++) != '\0');
    return dest;
}

static inline char *strncpy(char *dest, const char *src, size_t n) {
    size_t i = 0;
    for (; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}

static inline char *strcat(char *dest, const char *src) {
    char *d = dest;
    while (*d) d++;
    while ((*d++ = *src++) != '\0');
    return dest;
}

static inline char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (*d) d++;
    size_t i = 0;
    for (; i < n && src[i]; i++) d[i] = src[i];
    d[i] = '\0';
    return dest;
}

static inline size_t safe_strcpy(char *dest, size_t dest_sz, const char *src) {
    if (!dest || !src || dest_sz == 0) return 0;
    size_t i = 0;
    for (; i + 1 < dest_sz && src[i]; i++) dest[i] = src[i];
    dest[i] = '\0';
    return i;
}

// Your requested implementation
static inline size_t safe_strcat(char *dest, size_t dest_sz, const char *src) {
    return strlcat(dest, src, dest_sz);
}

/* --- 4. MEMORY OPERATIONS --- */

static inline void *memset(void *dest, int val, size_t len) {
    unsigned char *ptr = dest;
    while (len--) *ptr++ = (unsigned char)val;
    return dest;
}

static inline void *memcpy(void *dest, const void *src, size_t len) {
    unsigned char *d = dest; const unsigned char *s = src;
    while (len--) *d++ = *s++;
    return dest;
}

static inline void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest; const unsigned char *s = src;
    if (d < s) { while (n--) *d++ = *s++; }
    else if (d > s) { d += n; s += n; while (n--) *--d = *--s; }
    return dest;
}

static inline int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = s1, *p2 = s2;
    while (n--) { if (*p1 != *p2) return *p1 - *p2; p1++; p2++; }
    return 0;
}

/* --- 5. NUMERIC CONVERSION --- */

static inline int atoi(const char *str) {
    int res = 0, sign = 1;
    while (*str == ' ' || *str == '\t') str++;
    if (*str == '-') { sign = -1; str++; }
    while (*str >= '0' && *str <= '9') { res = res * 10 + (*str - '0'); str++; }
    return res * sign;
}

// Optimized itoa/ulltoa helpers
static inline void ulltoa(unsigned long long value, char *str, int base, int upper) {
    char *p = str;
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    if (value == 0) { *p++ = '0'; *p = '\0'; return; }
    while (value > 0) { *p++ = digits[value % base]; value /= base; }
    *p = '\0';
    // Reverse
    char *start = str, *end = p - 1;
    while (start < end) { char t = *start; *start++ = *end; *end-- = t; }
}

static inline void itoa(int value, char *str, int base) {
    if (value < 0 && base == 10) { *str++ = '-'; ulltoa((unsigned int)-value, str, 10, 0); }
    else { ulltoa((unsigned int)value, str, base, 0); }
}

/* --- 6. THE PRINTF ENGINE --- */

static inline int snprintf(char *str, size_t size, const char *fmt, ...) {
    if (!str || size == 0) return 0;

    va_list args;
    va_start(args, fmt);

    char *out = str;
    size_t rem = size ? size - 1 : 0;
    int total = 0;

    for (const char *p = fmt; *p; p++) {
        if (*p == '%' && *(p+1)) {
            p++;
            char buf[64]; memset(buf, 0, 64);
            int long_flag = 0;
            int longlong_flag = 0;

            // Handle length modifiers
            if (*p == 'l') {
                long_flag = 1;
                p++;
                if (*p == 'l') {
                    longlong_flag = 1;
                    p++;
                }
            }

            switch (*p) {
                case 's': {
                    const char *s = va_arg(args, const char*);
                    if (!s) s = "(null)";
                    for (int i=0; s[i]; i++, total++) {
                        if (rem > 0) { *out++ = s[i]; rem--; }
                    }
                    break;
                }
                case 'd':
                case 'i': {
                    if (longlong_flag) {
                        long long val = va_arg(args, long long);
                        if (val < 0) { *buf = '-'; ulltoa((unsigned long long)(-val), buf+1, 10, 0); }
                        else ulltoa((unsigned long long)val, buf, 10, 0);
                    } else if (long_flag) {
                        long val = va_arg(args, long);
                        if (val < 0) { *buf = '-'; ulltoa((unsigned long)(-val), buf+1, 10, 0); }
                        else ulltoa((unsigned long)val, buf, 10, 0);
                    } else {
                        int val = va_arg(args, int);
                        if (val < 0) { *buf = '-'; ulltoa((unsigned int)(-val), buf+1, 10, 0); }
                        else ulltoa((unsigned int)val, buf, 10, 0);
                    }
                    for (int i=0; buf[i]; i++, total++) {
                        if (rem > 0) { *out++ = buf[i]; rem--; }
                    }
                    break;
                }
                case 'u': {
                    if (longlong_flag) ulltoa(va_arg(args, unsigned long long), buf, 10, 0);
                    else if (long_flag) ulltoa(va_arg(args, unsigned long), buf, 10, 0);
                    else ulltoa(va_arg(args, unsigned int), buf, 10, 0);
                    for (int i=0; buf[i]; i++, total++) {
                        if (rem > 0) { *out++ = buf[i]; rem--; }
                    }
                    break;
                }
                case 'x':
                case 'X': {
                    int upper = (*p == 'X');
                    if (longlong_flag) ulltoa(va_arg(args, unsigned long long), buf, 16, upper);
                    else if (long_flag) ulltoa(va_arg(args, unsigned long), buf, 16, upper);
                    else ulltoa(va_arg(args, unsigned int), buf, 16, upper);
                    for (int i=0; buf[i]; i++, total++) {
                        if (rem > 0) { *out++ = buf[i]; rem--; }
                    }
                    break;
                }
                case 'p': {
                    unsigned long long ptr = (unsigned long long)va_arg(args, void*);
                    strcpy(buf, "0x");
                    char tmp[32]; memset(tmp, 0, 32);
                    ulltoa(ptr, tmp, 16, 0);
                    strncat(buf, tmp, sizeof(buf)-strlen(buf)-1);
                    for (int i=0; buf[i]; i++, total++) {
                        if (rem > 0) { *out++ = buf[i]; rem--; }
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    if (rem > 0) { *out++ = c; rem--; }
                    total++;
                    break;
                }
                case '%': {
                    if (rem > 0) { *out++ = '%'; rem--; }
                    total++;
                    break;
                }
                default:
                    // Unknown specifier: print literally
                    if (rem > 0) { *out++ = '%'; rem--; }
                    total++;
                    if (rem > 0) { *out++ = *p; rem--; }
                    total++;
                    break;
            }
        } else {
            if (rem > 0) { *out++ = *p; rem--; }
            total++;
        }
    }

    *out = '\0';
    va_end(args);
    return total;
}

#endif // STRING_H

// Syscall numbers (shared with kernel)
#ifndef SYSCALL_NUMBERS_H
#define SYSCALL_NUMBERS_H
// This file can be included by both kernel and userspace programs
#define SYS_EXIT        0
#define SYS_FORK        1
#define SYS_READ        2
#define SYS_WRITE       3
#define SYS_OPEN        4
#define SYS_CLOSE       5
#define SYS_WAIT        6
#define SYS_EXEC        7
#define SYS_WAITX       42 /* waitpid-like: (pid, status*, options) */
#define SYS_EXECVE      43 /* execve(path, argv, envp) */
#define SYS_PUTENV      44 /* putenv("KEY=VALUE") */
#define SYS_GETENV      45 /* getenv(key, buf, buflen) -> len or -errno */
#define SYS_ENVLIST     46 /* envlist(buf, buflen) -> bytes_written or -errno */
#define SYS_ENVLIST2    47 /* envlist2(offset_inout, buf, buflen) -> bytes_written or -errno */
#define SYS_UNSETENV    48 /* unsetenv(key) -> 0 or -errno */
#define SYS_PROCLIST    49 /* procs(buf, buflen) -> count or -errno */
#define SYS_PIDINFO     50 /* md64api_get_pid_info(pid, out, out_size) -> 0 or -errno */
#define SYS_GETPID      8
#define SYS_GETPPID     9
#define SYS_SLEEP       10
#define SYS_YIELD       11
/* SYS_MALLOC/FREE removed - handled by userland libc */
#define SYS_SBRK        14  /* Low-level heap expansion only */
#define SYS_KILL        15
#define SYS_TIME        16
#define SYS_CHDIR       17
#define SYS_GETCWD      18
#define SYS_STAT        19
#define SYS_MKDIR       20
#define SYS_RMDIR       21
#define SYS_UNLINK      22
#define SYS_LSEEK       23
#define SYS_WRITEFILE   24
#define SYS_INPUT       28 // DO NOT USE - Deprecated
#define SYS_SSTATS      29 /* SYS_SSTATS (29) removed — use $/dev/md64api/sysinfo via DevFS instead. */
#define SYS_SSTATS2     38 /* fill user buffer with md64api_sysinfo_data_u  This literrly does not live anywhere, useless shit. - Deprecated*/
// VFS formatting / mkfs
#define SYS_VFS_MKFS    36
#define SYS_VFS_GETPART 37
#define SYS_VFS_MBRINIT 41
/* User identity */
#define SYS_GETUID      33
#define SYS_SETUID      34
/* Graphics/VGA - REMOVED: ~~Use $/dev/graphics/video0 instead~~ use $/dev/mvc/mvi0*/
/* Virtual memory mapping (userland dynamic linker support) */
#define SYS_MMAP        39
#define SYS_MUNMAP      40
#define SYS_DEV_MMAP    51
/* Networking - REMOVED: Use $/user/network/* (NetMan service) instead */
/* FS tracing (timing) */
#define SYS_FS_TRACE        55 /* arg1=0/1 set, returns previous state */
#define SYS_OPENDIR         56 /* opendir(path) -> dirfd */
#define SYS_READDIR         57 /* readdir(dirfd, namebuf, bufsz, is_dir*, size*) */
#define SYS_CLOSEDIR        58 /* closedir(dirfd) */

/* Userland USERFS nodes */
#define SYS_USERFS_REGISTER 64

/* Custom IPC / Device control operations (non-POSIX) */
#define SYS_INVOKE      71  /* invoke(fd, in_buf, in_size, out_buf, out_size) */

/* Filesystem mount/unmount */
#define SYS_MOUNT       65  /* mount(vdrive_id, partition_lba, fs_type) -> slot or -errno */
#define SYS_UNMOUNT     66  /* unmount(slot) -> 0 or -errno */
#define SYS_MOUNTS      67  /* mounts(buf, buflen) -> count or -errno */

/* POSIX fd operations */
#define SYS_DUP         68  /* dup(oldfd) -> newfd or -errno */
#define SYS_DUP2        69  /* dup2(oldfd, newfd) -> newfd or -errno */
#define SYS_PIPE        70  /* pipe(fds[2]) -> 0 or -errno */

/* POSIX process identity */
#define SYS_GETGID      72  /* getgid() -> gid */
#define SYS_SETGID      73  /* setgid(gid) -> 0 or -errno */
#define SYS_GETEUID     74  /* geteuid() -> euid */
#define SYS_GETEGID     75  /* getegid() -> egid */

/* GPU Core - REMOVED: Use $/dev/graphics/video0 (DevFS) instead */

/* POSIX Signals */
#define SYS_SIGNAL             87  /* signal(signum, handler) -> old_handler or -errno */
#define SYS_RAISE              88  /* raise(signum) -> 0 or -errno */
#define SYS_SIGACTION          89  /* sigaction(signum, act*, oldact*) -> 0 or -errno */

/* File Descriptor Injection (for TTY manager) */
#define SYS_FD_INJECT          90  /* fd_inject(pid, fd, fd_obj) -> 0 or -errno */

/* POSIX Sessions and Process Groups */
#define SYS_SETSID             91  /* setsid() -> new SID or -errno */
#define SYS_SETPGID            92  /* setpgid(pid, pgid) -> 0 or -errno */
#define SYS_GETPGID            93  /* getpgid(pid) -> pgid or -errno */
#define SYS_GETSID             94  /* getsid(pid) -> sid or -errno */

/* ioctl commands for controlling terminal */
#define TIOCSCTTY              0x540E  /* Set controlling terminal */
#define TIOCNOTTY              0x5422  /* Give up controlling terminal */

/* Signal numbers (POSIX-compatible) */
#define SIGHUP     1   /* Hangup */
#define SIGINT     2   /* Interrupt (Ctrl+C) */
#define SIGQUIT    3   /* Quit */
#define SIGILL     4   /* Illegal instruction */
#define SIGTRAP    5   /* Trace trap */
#define SIGABRT    6   /* Abort */
#define SIGBUS     7   /* Bus error */
#define SIGFPE     8   /* Floating point exception */
#define SIGKILL    9   /* Kill (uncatchable) */
#define SIGUSR1    10  /* User-defined 1 */
#define SIGSEGV    11  /* Segmentation fault */
#define SIGUSR2    12  /* User-defined 2 */
#define SIGPIPE    13  /* Broken pipe */
#define SIGALRM    14  /* Alarm */
#define SIGTERM    15  /* Termination */
#define SIGCHLD    17  /* Child process status */
#define SIGCONT    18  /* Continue */
#define SIGSTOP    19  /* Stop (uncatchable) */
#define SIGTSTP    20  /* Terminal stop (Ctrl+Z) */

/* Signal handler types */
#define SIG_DFL    ((void (*)(int))0)  /* Default action */
#define SIG_IGN    ((void (*)(int))1)  /* Ignore signal */
#define SIG_ERR    ((void (*)(int))-1) /* Error return */
#define SYS_RT_SIGRETURN   25

#endif


//fs.h - Kernel filesystem interface
#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_MOUNTS 26

/* Filesystem types */
typedef enum {
    FS_TYPE_UNKNOWN   = 0,
    FS_TYPE_FAT32     = 1,
    FS_TYPE_ISO9660   = 2,
    FS_TYPE_EXTERNAL  = 3,
    FS_TYPE_MDFS      = 4
} fs_type_t;

/* File information structure */
typedef struct {
    char name[260];           /* File/directory name */
    uint32_t size;            /* File size in bytes */
    int is_directory;         /* 1 if directory, 0 if file */
    uint32_t cluster;         /* Starting cluster (FAT32) or extent (ISO9660) */
} fs_file_info_t;

/* Mount handle - encapsulates filesystem-specific handle */
struct fs_ext_driver_ops;

typedef struct {
    fs_type_t type;           /* Filesystem type */
    int handle;               /* Filesystem-specific handle */
    int valid;                /* Is this mount valid? */

    // External filesystem support (FS_TYPE_EXTERNAL)
    const struct fs_ext_driver_ops *ext_ops;
    void *ext_ctx;
    char ext_name[16];
} fs_mount_t;

/* Public mount metadata returned to userland */
typedef struct {
    int slot;                /* Mount slot index */
    int vdrive_id;           /* vDrive ID */
    uint32_t partition_lba;  /* Partition LBA */
    fs_type_t type;          /* Filesystem type */
    char label[64];          /* Volume label or mount name */
} fs_mount_info_t;

/* --- KERNEL MOUNT TABLE MANAGEMENT --- */

/**
 * Initialize filesystem mount table
 * Called once during kernel init
 */
void fs_init(void);

/**
 * Format a partition with FAT32 filesystem
 * WARNING: This will erase all data on the specified partition!
 * 
 * The partition MUST be unmounted before formatting.
 * After formatting, you must mount it with fs_mount_drive() to use it.
 *
 * @param vdrive_id: Virtual drive ID (from vDrive subsystem)
 * @param partition_lba: Partition starting LBA offset
 * @param partition_sectors: Size of the partition in sectors (512 bytes each)
 * @param volume_label: Volume label string (max 11 chars, NULL for "NO NAME")
 * @param sectors_per_cluster: Cluster size in sectors (0 for auto, or 1/2/4/8/16/32/64/128)
 * @return: 0 on success, negative on error
 *          -1: vDrive not ready
 *          -2: Drive is mounted (unmount first)
 *          -3: Invalid partition size
 *          -4: Format failed
 * 
 * Example:
 *   // Format vDrive 0, starting at LBA 2048, size 1GB (2097152 sectors)
 *   fs_format(0, 2048, 2097152, "MYVOLUME", 0);
 *   // Then mount it:
 *   int slot = fs_mount_drive(0, 2048, FS_TYPE_FAT32);
 */
int fs_format(int vdrive_id, uint32_t partition_lba, uint32_t partition_sectors,
              const char* volume_label, uint32_t sectors_per_cluster);

/**
 * Mount a drive (auto-detect or specific type)
 * @param vdrive_id: Virtual drive ID (from vDrive subsystem)
 * @param partition_lba: Partition LBA offset (0 for whole disk/auto-detect)
 * @param type: FS_TYPE_UNKNOWN for auto-detect, or specific type
 * @return: Slot ID (0-25) on success, negative on error
 *          -2: Already mounted
 *          -3: Mount table full
 *          -4: Unknown filesystem type
 *          -5: Mount failed
 *          -6: vDrive not ready
 */
// Forward declarations for external driver ops
struct fs_dir;
struct fs_dirent;
typedef struct fs_dir fs_dir_t;
typedef struct fs_dirent fs_dirent_t;

// External FS driver ops (for third-party FS modules)
typedef struct fs_ext_driver_ops {
    // Return 1 if this FS recognizes the drive/partition, 0 otherwise.
    int (*probe)(int vdrive_id, uint32_t partition_lba);

    // Mount and populate mount->ext_ctx (and any other required fields).
    // mount->ext_ops and mount->ext_name are filled by core.
    int (*mount)(int vdrive_id, uint32_t partition_lba, fs_mount_t *mount);

    // Optional unmount hook. If NULL, core will just drop the mount.
    void (*unmount)(fs_mount_t *mount);

    // Optional format/mkfs hook.
    // vDrive-based; partition_lba is start of partition; partition_sectors is length.
    // Returns 0 on success.
    int (*mkfs)(int vdrive_id, uint32_t partition_lba, uint32_t partition_sectors, const char *volume_label);

    // Read support
    int (*read_file)(fs_mount_t *mount, const char *path, void *buffer, size_t buffer_size, size_t *bytes_read);

    // Optional write support (NULL => read-only)
    int (*write_file)(fs_mount_t *mount, const char *path, const void *buffer, size_t size);

    int (*stat)(fs_mount_t *mount, const char *path, fs_file_info_t *info);
    int (*file_exists)(fs_mount_t *mount, const char *path);
    int (*directory_exists)(fs_mount_t *mount, const char *path);
    int (*list_directory)(fs_mount_t *mount, const char *path);

    // Optional directory mutation
    int (*mkdir)(fs_mount_t *mount, const char *path);
    int (*rmdir)(fs_mount_t *mount, const char *path);

    // Optional file mutation
    int (*unlink)(fs_mount_t *mount, const char *path);

    // Directory iteration
    fs_dir_t* (*opendir)(fs_mount_t *mount, const char *path);
    int (*readdir)(fs_dir_t *dir, fs_dirent_t *entry);
    void (*closedir)(fs_dir_t *dir);
} fs_ext_driver_ops_t;

// Register external filesystem driver (string-based). Built-ins always win; external drivers are tried only after.
int fs_register_driver(const char *name, const fs_ext_driver_ops_t *ops);

// Invoke an external filesystem driver's mkfs callback (if provided).
int fs_ext_mkfs(const char *driver_name, int vdrive_id, uint32_t partition_lba, uint32_t partition_sectors, const char *volume_label);

// Internal helper: update MBR partition type for the partition starting at start_lba.
int fs_mbr_set_type_for_lba(int vdrive_id, uint32_t start_lba, uint8_t new_type);

int fs_mount_drive(int vdrive_id, uint32_t partition_lba, fs_type_t type);

/**
 * Unmount filesystem by slot ID
 * @param slot: Slot ID (0-25, corresponds to A-Z)
 * @return: 0 on success, negative on error
 */
int fs_unmount_slot(int slot);

/**
 * Get mount handle by slot ID
 * @param slot: Slot ID (0-25)
 * @return: Pointer to mount structure, or NULL if invalid/unmounted
 */
fs_mount_t* fs_get_mount(int slot);

/**
 * Get mount metadata
 * @param slot: Slot ID
 * @param vdrive_id: Output - virtual drive ID (can be NULL)
 * @param partition_lba: Output - partition LBA (can be NULL)
 * @param type: Output - filesystem type (can be NULL)
 * @return: 0 on success, -1 if slot invalid/unmounted
 */
int fs_get_mount_info(int slot, int* vdrive_id, uint32_t* partition_lba, fs_type_t* type);

/*
 * Get a stable human-readable mount label.
 * Examples:
 *  - "vDrive1" (whole-disk/superfloppy/ISO)
 *  - "vDrive1-P1" (partitioned disk, partition #1)
 */
int fs_get_mount_label(int slot, char *out, size_t out_size);

/* Return 0 if not a partitioned mount, otherwise 1..4 for MBR partitions. */
int fs_get_mount_partition_index(int slot);

/**
 * List all active mounts (prints to VGA)
 */
void fs_list_mounts(void);

/**
 * Get total number of active mounts
 * @return: Number of mounted filesystems
 */
int fs_get_mount_count(void);

/* --- FILE OPERATIONS --- */

/**
 * Read entire file into buffer
 * @param mount: Mount handle (from fs_get_mount)
 * @param path: File path (e.g., "/dir/file.txt")
 * @param buffer: Output buffer
 * @param buffer_size: Size of output buffer
 * @param bytes_read: Optional - actual bytes read (can be NULL)
 * @return: 0 on success, negative on error
 */
int fs_read_file(fs_mount_t* mount, const char* path, void* buffer, 
                 size_t buffer_size, size_t* bytes_read);

/* FS tracing (timing) */
void fs_set_trace(int enabled);
int fs_get_trace(void);

/**
 * Write entire file from buffer.
 * Currently supported for FAT32 only.
 *
 * @param mount: Mount handle (from fs_get_mount)
 * @param path: File path (e.g., "/dir/file.txt")
 * @param buffer: Input buffer
 * @param size: Number of bytes to write
 * @return: 0 on success, negative on error
 */
int fs_write_file(fs_mount_t* mount, const char* path, const void* buffer, size_t size);

// Offset-aware write (used by FD layer for sequential writes). Returns 0 on success.
int fs_write_file_at(fs_mount_t* mount, const char* path, const void* buffer, size_t size, size_t offset);


/**
 * Get file information
 * @param mount: Mount handle
 * @param path: File path
 * @param info: Output file info structure
 * @return: 0 on success, negative on error
 */
int fs_stat(fs_mount_t* mount, const char* path, fs_file_info_t* info);

/**
 * Check if file exists
 * @param mount: Mount handle
 * @param path: File path
 * @return: 1 if exists, 0 if not
 */
int fs_file_exists(fs_mount_t* mount, const char* path);

/* --- DIRECTORY OPERATIONS --- */

/* Directory entry structure for iteration */
typedef struct fs_dirent {
    char name[260];           /* Entry name */
    uint32_t size;            /* File size in bytes */
    int is_directory;         /* 1 if directory, 0 if file */
    uint32_t reserved;        /* Reserved for future use */
} fs_dirent_t;

/* Directory handle for iteration */
typedef struct fs_dir {
    fs_mount_t* mount;        /* Mount point */
    char path[256];           /* Directory path */
    size_t position;          /* Current position in directory */
    void* fs_specific;        /* Filesystem-specific data */

    // External FS directory iteration
    const fs_ext_driver_ops_t *ext_ops;
} fs_dir_t;

/**
 * List directory contents
 * @param mount: Mount handle
 * @param path: Directory path (NULL or "/" for root)
 * @return: 0 on success, negative on error
 */
int fs_list_directory(fs_mount_t* mount, const char* path);

/**
 * Check if directory exists
 * @param mount: Mount handle
 * @param path: Directory path
 * @return: 1 if exists and is directory, 0 otherwise
 */
int fs_directory_exists(fs_mount_t* mount, const char* path);

// Create a directory at path (absolute within mount). Returns 0 on success.
int fs_mkdir(fs_mount_t* mount, const char* path);

// Remove an empty directory at path (absolute within mount). Returns 0 on success.
int fs_rmdir(fs_mount_t* mount, const char* path);

// Remove a file at path (absolute within mount). Returns 0 on success.
int fs_unlink(fs_mount_t* mount, const char* path);

/**
 * Open directory for iteration
 * @param mount: Mount handle
 * @param path: Directory path (NULL or "/" for root)
 * @return: Directory handle, or NULL on error
 */
fs_dir_t* fs_opendir(fs_mount_t* mount, const char* path);

/**
 * Read next directory entry
 * @param dir: Directory handle
 * @param entry: Output entry structure
 * @return: 1 on success, 0 at end of directory, -1 on error
 */
int fs_readdir(fs_dir_t* dir, fs_dirent_t* entry);

/**
 * Close directory handle
 * @param dir: Directory handle
 */
void fs_closedir(fs_dir_t* dir);

/* --- UTILITY FUNCTIONS --- */

/**
 * Get filesystem type name
 * @param type: Filesystem type
 * @return: String name (e.g., "FAT32", "ISO9660")
 */
const char* fs_type_name(fs_type_t type);

// Rescan all ready vDrives and mount any new filesystems using FS_TYPE_UNKNOWN.
// Intended to be called after SQRM modules register external filesystem drivers.
void fs_rescan_all(void);

// MBR partition lookup helper.
// part_no: 1..4 (primary partitions)
// Returns 0 on success.
int fs_mbr_get_partition(int vdrive_id, int part_no, uint32_t *out_start_lba, uint32_t *out_sectors, uint8_t *out_type);

#endif /* FS_H */
// MD64API (userland-visible kernel interfaces)
#ifndef MODUOS_KERNEL_MD64API_GRP_H
#define MODUOS_KERNEL_MD64API_GRP_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * MD64API Graphics (GRP)
 *
 * This is a small, file-based API that targets devices under:
 *   $/dev/graphics/
 *
 * The canonical primary device is:
 *   $/dev/graphics/video0
 */

#define MD64API_GRP_DEFAULT_DEVICE "$/dev/graphics/video0"

typedef enum {
    MD64API_GRP_MODE_TEXT     = 0,
    MD64API_GRP_MODE_GRAPHICS = 1,
} md64api_grp_mode_t;

typedef enum {
    MD64API_GRP_FMT_UNKNOWN   = 0,
    MD64API_GRP_FMT_RGB565    = 1,
    MD64API_GRP_FMT_XRGB8888  = 2,
} md64api_grp_format_t;

/* Binary payload returned by reading from $/dev/graphics/video0 */
typedef struct __attribute__((packed)) {
    uint64_t fb_addr;     /* virtual address of linear framebuffer (0 in text mode) */
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t  bpp;
    uint8_t  mode;        /* md64api_grp_mode_t */
    uint8_t  fmt;         /* md64api_grp_format_t */
    uint8_t  reserved;
} md64api_grp_video_info_t;

#ifdef __cplusplus
}
#endif

#endif

#ifndef MD64API_USER_H
#define MD64API_USER_H

#include <stdint.h>

/*
 * User-safe system info structure.
 * No pointers: all strings are copied into fixed-size buffers.
 */

typedef struct md64api_sysinfo_data_u {
    uint64_t sys_available_ram;
    uint64_t sys_total_ram;

    /* Version strings (copied from kernel; user-safe) */
    char SystemVersion[32];
    char KernelVersion[64];

    char KernelVendor[64];
    char os_name[32];
    char os_arch[16];

    char pcname[128];
    char username[32];
    char domain[32];
    char kconsole[64];

    char cpu[16];
    char cpu_manufacturer[16];
    char cpu_model[64];
    int cpu_cores;
    int cpu_threads;
    int cpu_hyperthreading_enabled;
    int cpu_base_mhz;
    int cpu_max_mhz;
    int cpu_cache_l1_kb;
    int cpu_cache_l2_kb;
    int cpu_cache_l3_kb;
    char cpu_flags[128];

    int is_virtual_machine;
    char virtualization_vendor[32];

    char gpu_name[128];
    char gpu_driver[64];
    int gpu_vram_mb;

    uint64_t storage_total_mb;
    uint64_t storage_free_mb;
    char primary_disk_model[128];

    char bios_vendor[64];
    char bios_version[64];
    char motherboard_model[64];

    int secure_boot_enabled;
    int tpm_version;

    /* System uptime in seconds since boot. */
    uint64_t uptime_seconds;

    /* RTC date/time snapshot (from kernel RTC). */
    uint8_t rtc_second;
    uint8_t rtc_minute;
    uint8_t rtc_hour;
    uint8_t rtc_day;
    uint8_t rtc_month;
    uint16_t rtc_year;
} md64api_sysinfo_data_u;

#endif

#ifndef MODUOS_FS_USERFS_USER_API_H
#define MODUOS_FS_USERFS_USER_API_H

#include <stdint.h>
#include <stddef.h>

typedef int64_t ssize_t;

#ifdef __cplusplus
extern "C" {
#endif

typedef ssize_t (*userfs_user_read_fn)(void *ctx, void *buf, size_t count);
typedef ssize_t (*userfs_user_write_fn)(void *ctx, const void *buf, size_t count);
typedef ssize_t (*userfs_user_invoke_fn)(void *ctx, const void *in_buf,  size_t in_size, void *out_buf, size_t out_size);

typedef struct {
    userfs_user_read_fn read;
    userfs_user_write_fn write;
    userfs_user_invoke_fn invoke;
} userfs_user_ops_t;

typedef enum {
    USERFS_PERM_READ_ONLY  = 0x1,
    USERFS_PERM_WRITE_ONLY = 0x2,
    USERFS_PERM_READ_WRITE = 0x3,
    USERFS_PERM_INVOKE     = 0x4,
} userfs_perm_t;

typedef struct {
    const char *path;         /* path relative to $/user */
    const char *owner_id;     /* owner identity string */
    uint32_t perms;           /* USERFS_PERM_* */
    userfs_user_ops_t ops;    /* user callbacks (unused in-kernel) */
    void *ctx;                /* user context pointer */
} userfs_user_node_t;

#ifdef __cplusplus
}
#endif

#endif

// SYS_WRITEFILE is provided by syscall_numbers.h

// File descriptor constants
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

// Open flags
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT     0x0100
#define O_TRUNC     0x0200
#define O_APPEND    0x0400
#define O_NONBLOCK  0x0800

// Signal numbers (subset of POSIX signals)
#define SIGKILL  9
#define SIGTERM 15
#define SIGCHLD 17

// NTOSIUX Signals (Used by Fault Handler) (NUM 20-35)
#define SIGFAULT_PAGE            20 // Invalid memory access (non-present page or protection violation)
#define SIGFAULT_GENERAL         21 // General Protection Fault (GPF) - privilege or limit violation
#define SIGFAULT_INVALID_OPCODE  22 // CPU encountered an undefined or illegal instruction
#define SIGFAULT_DIV0            23 // Integer division by zero or division overflow
#define SIGFAULT_DEBUG           24 // Hardware breakpoint or single-step trap
#define SIGFAULT_ALIGNMENT       25 // Data alignment check (unaligned memory access on strict archs)
#define SIGFAULT_BOUNDS          26 // Range check failure (BOUND instruction / array index out of bounds)
#define SIGFAULT_STACK_SEGMENT   27 // Stack segment fault (stack limit exceeded or not present)
#define SIGFAULT_FPU_ERROR       28 // x87 Floating-Point Unit math exception (NaN, Underflow, etc.)
#define SIGFAULT_SIMD_ERROR      29 // SSE/AVX vector math exception (Precision/Masked faults)
#define SIGFAULT_MACHINE_CHECK   30 // Critical hardware failure (ECC error, bus timeout, overheating)
#define SIGFAULT_VIRT_ERROR      31 // Virtualization-specific exception (Hypervisor/EPT violation)
#define SIGFAULT_CP_ERROR        32 // Control Protection (Shadow Stack violation / ROP protection)
#define SIGFAULT_NMI             33 // Non-Maskable Interrupt (Hardware-level "Emergency Stop")
#define SIGFAULT_SEG_NOT_PRESENT 34 // Segment not present in memory (GDT/LDT entry issue)
#define SIGFAULT_DOUBLE_FAULT    35 // Nested exception failure (usually fatal for the process)

// Type definitions
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef long ssize_t;
#endif

typedef const char* string;

/* System Information Structure */

// ---------------- MD64API SysInfo ----------------

#ifndef MD64API_USER_H
#define MD64API_USER_H

#include <stdint.h>

/*
 * User-safe system info structure.
 * No pointers: all strings are copied into fixed-size buffers.
 */

typedef struct md64api_sysinfo_data_u {
    uint64_t sys_available_ram;
    uint64_t sys_total_ram;

    /* Version strings (copied from kernel; user-safe) */
    char SystemVersion[32];
    char KernelVersion[64];

    char KernelVendor[64];
    char os_name[32];
    char os_arch[16];

    char pcname[128];
    char username[32];
    char domain[32];
    char kconsole[64];

    char cpu[16];
    char cpu_manufacturer[16];
    char cpu_model[64];
    int cpu_cores;
    int cpu_threads;
    int cpu_hyperthreading_enabled;
    int cpu_base_mhz;
    int cpu_max_mhz;
    int cpu_cache_l1_kb;
    int cpu_cache_l2_kb;
    int cpu_cache_l3_kb;
    char cpu_flags[128];

    int is_virtual_machine;
    char virtualization_vendor[32];

    char gpu_name[128];
    char gpu_driver[64];
    int gpu_vram_mb;

    uint64_t storage_total_mb;
    uint64_t storage_free_mb;
    char primary_disk_model[128];

    char bios_vendor[64];
    char bios_version[64];
    char motherboard_model[64];

    int secure_boot_enabled;
    int tpm_version;

    /* System uptime in seconds since boot. */
    uint64_t uptime_seconds;

    /* RTC date/time snapshot (from kernel RTC). */
    uint8_t rtc_second;
    uint8_t rtc_minute;
    uint8_t rtc_hour;
    uint8_t rtc_day;
    uint8_t rtc_month;
    uint16_t rtc_year;
} md64api_sysinfo_data_u;

#endif


/* Legacy pointer-based struct kept for older apps (unsafe in ring3).
 * Prefer md64api_sysinfo_data_u + SYS_SSTATS2.
 */

typedef struct md64api_sysinfo_data
{
    /* --- Memory Info --- */
    uint64_t sys_available_ram;     /* Available RAM in MB */
    uint64_t sys_total_ram;         /* Total system RAM in MB */

    /* --- OS / Kernel Info --- */
    int SystemVersion;              /* OS major version */
    int KernelVersion;              /* Kernel version number */
    string KernelVendor;            /* NTSoftware / New Technologies Software */
    string os_name;                 /* ModuOS */
    string os_arch;                 /* AMD64 only (ARM version not implemented) */

    /* --- System Identity --- */
    string pcname;                  /* Host / computer name */
    string username;                /* Current user */
    string domain;                  /* Domain / workgroup */
    string kconsole;                /* VGA console / VBE text console */

    /* --- CPU Info --- */
    string cpu;                     /* Vendor string: GenuineIntel, AuthenticAMD, etc */
    string cpu_manufacturer;        /* Intel / AMD / etc */
    string cpu_model;               /* Core i5-13600K, Ryzen 7 5800X, etc */
    int cpu_cores;                  /* Physical cores */
    int cpu_threads;                /* Logical processors */
    int cpu_hyperthreading_enabled; /* 1 = enabled, 0 = disabled */
    int cpu_base_mhz;               /* Base clock in MHz */
    int cpu_max_mhz;                /* Max turbo clock in MHz */
    int cpu_cache_l1_kb;            /* L1 cache size */
    int cpu_cache_l2_kb;            /* L2 cache size */
    int cpu_cache_l3_kb;            /* L3 cache size */
    string cpu_flags;               /* SSE, SSE2, AVX, AVX2, AES, etc */

    /* --- Virtualization Info --- */
    int is_virtual_machine;         /* 1 = VM detected, 0 = physical */
    string virtualization_vendor;   /* VMware, VirtualBox, KVM, Hyper-V, etc */

    /* --- GPU Info --- */
    string gpu_name;                /* GPU model */
    int gpu_vram_mb;                /* VRAM in MB */

    /* --- Storage Info --- */
    uint64_t storage_total_mb;      /* Total storage space in MB */
    uint64_t storage_free_mb;       /* Free storage space in MB */
    string primary_disk_model;      /* Disk model name */

    /* --- Firmware / BIOS --- */
    string bios_vendor;             /* AMI, Phoenix, UEFI vendor */
    string bios_version;            /* BIOS/UEFI version */
    string motherboard_model;       /* Motherboard identifier */

    /* --- Security Features --- */
    int secure_boot_enabled;        /* 1 = Secure Boot enabled */
    int tpm_version;                /* 0 = none, 1.2 = 1, 2.0 = 2 */

} md64api_sysinfo_data;

// Syscall wrapper (up to 3 args)
// ABI: rax=syscall_num, rdi=arg1, rsi=arg2, rdx=arg3
static inline long syscall(long num, long arg1, long arg2, long arg3) {
    long ret;
    __asm__ volatile (
        "mov %1, %%rax\n"
        "mov %2, %%rdi\n"
        "mov %3, %%rsi\n"
        "mov %4, %%rdx\n"
        "xor %%r10, %%r10\n"
        "xor %%r8,  %%r8\n"
        "xor %%r9,  %%r9\n"
        "syscall\n"
        "mov %%rax, %0"
        : "=r"(ret)
        : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3)
        : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "rcx", "r11", "memory"
    );
    return ret;
}
/*
 * 4-arg syscall wrapper
 * Kernel ABI (see src/arch/AMD64/syscall/syscall_entry.asm):
 *   rax=syscall_num, rdi=arg1, rsi=arg2, rdx=arg3, r10=arg4
 */
static inline long syscall4(long num, long arg1, long arg2, long arg3, long arg4) {
    long ret;
    __asm__ volatile (
        "mov %1, %%rax\n"
        "mov %2, %%rdi\n"
        "mov %3, %%rsi\n"
        "mov %4, %%rdx\n"
        "mov %5, %%r10\n"
        "syscall\n"
        "mov %%rax, %0"
        : "=r"(ret)
        : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4)
        : "rax", "rdi", "rsi", "rdx", "r10", "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall5(long num, long arg1, long arg2, long arg3, long arg4, long arg5) {
    long ret;
    __asm__ volatile (
        "mov %1, %%rax\n"
        "mov %2, %%rdi\n"
        "mov %3, %%rsi\n"
        "mov %4, %%rdx\n"
        "mov %5, %%r10\n"
        "mov %6, %%r8\n"
        "syscall\n"
        "mov %%rax, %0"
        : "=r"(ret)
        : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "r"(arg5)
        : "rax", "rdi", "rsi", "rdx", "r10", "r8", "rcx", "r11", "memory"
    );
    return ret;
}

/* ============================================================
   BASIC OUTPUT (now matches new SYS_WRITE)
   ============================================================ */

// Writes a single character to STDOUT
static inline void putc(char c);

// Writes a null-terminated string to STDOUT
static inline void puts_raw(const char *s);

// Write string + newline
static inline void puts(const char *s) {
    puts_raw(s);
    putc('\n');
}

static inline ssize_t input_read(char *buf, size_t max_len) 
{
    if (!buf || max_len == 0 || max_len > INT_MAX) {
        return -1;
    }

    return syscall(SYS_INPUT, (long)buf, (long)max_len, 0);
}

/* Forward declarations (input() uses file I/O wrappers declared later) */
static inline int open(const char *pathname, int flags, int mode);
static inline ssize_t read(int fd, void *buf, size_t count);
static inline int close(int fd);

// Drain the structured input queue ($/dev/input/event0).
// Useful because the kernel shell and some apps consume event0, while libc's input() reads kbd0.
static inline void yield(void) {
    syscall(SYS_YIELD, 0, 0, 0);
}

static inline void input_flush_events(void) {
    int efd = open("$/dev/input/event0", O_RDONLY | O_NONBLOCK, 0);
    if (efd >= 0) {
        /* Events are opaque to libc; we just drain bytes. */
        unsigned char buf[64];
        for (;;) {
            ssize_t n = read(efd, buf, sizeof(buf));
            if (n <= 0) break;
        }
        close(efd);
    }
}

// Flush any pending buffered input so the next line read doesn't auto-consume previous keystrokes.
// This also drains the structured input queue ($/dev/input/event0) which the shell / games may use.
static inline void input_flush(void) {
    /* Drain raw keyboard chars */
    int kfd = open("$/dev/input/kbd0", O_RDONLY | O_NONBLOCK, 0);
    if (kfd >= 0) {
        for (;;) {
            char c;
            if (read(kfd, &c, 1) != 1) break;
        }
        close(kfd);
    }

    input_flush_events();
}

/* Safer version: read input into caller-provided buffer with bounds checking.
 * Returns: bytes read (excluding null terminator), or negative on error.
 */
static inline ssize_t input_line_to_buffer(char *buf, size_t maxlen) {
    if (!buf || maxlen == 0) return -1;
    if (maxlen > 0x7FFFFFFF) return -1;

    int fd = open("$/dev/input/kbd0", O_RDONLY, 0);
    if (fd < 0) {
        if (maxlen > 0) buf[0] = 0;
        return -1;
    }

    size_t n = 0;
    size_t safe_max = (maxlen > 0) ? (maxlen - 1) : 0;
    
    for (;;) {
        char c;
        if (read(fd, &c, 1) != 1) {
            yield();
            continue;
        }
        
        if (c == '\r') continue;
        
        if (c == '\n') {
            buf[n] = 0;
            close(fd);
            input_flush_events();
            return (ssize_t)n;
        }
        
        if ((c == '\b' || c == 127) && n > 0) {
            n--;
            buf[n] = 0;
            puts_raw("\b");
            continue;
        }
        
        if (n < safe_max && c >= 32 && c < 127) {
            buf[n++] = c;
            buf[n] = 0;
            char tmp[2] = {c, 0};
            puts_raw(tmp);
        }
    }
}

// Convenience: returns pointer to a static buffer (blocking line read from kbd0)
// WARNING: NOT THREAD-SAFE. Use input_line_to_buffer() in new code.
static inline char* input() {
    static char input_buf[256];
    ssize_t ret = input_line_to_buffer(input_buf, sizeof(input_buf));
    return input_buf;  /* Return buffer even on error; will contain null string */
}

/* ============================================================
   SYSTEM INFO
   ============================================================ */

static inline int get_system_info_u(md64api_sysinfo_data_u *out) {
    return (int)syscall(SYS_SSTATS2, (long)out, (long)sizeof(*out), 0);
}

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MD_PROCLIST_NAME_MAX 64

typedef struct md_proclist_entry_u {
    uint32_t pid;
    uint32_t ppid;
    uint32_t state;
    uint64_t total_time;     // scheduler ticks (see process_t.total_time)
    char name[MD_PROCLIST_NAME_MAX];
} md_proclist_entry_u;

#ifdef __cplusplus
}
#endif


static inline int get_process_list(md_proclist_entry_u *out, size_t out_count) {
    return (int)syscall(SYS_PROCLIST, (long)out, (long)(out_count * sizeof(*out)), 0);
}

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MD64API_PID_NAME_MAX 64
#define MD64API_CWD_MAX 256
#define MD64API_MAX_PROCESSES 256

typedef struct md64api_pid_info_u {
    uint32_t pid;
    uint32_t ppid;
    uint32_t uid;
    uint32_t gid;
    uint32_t state;        // process_state_t
    uint32_t priority;
    uint64_t total_time;   // scheduler ticks
    uint64_t user_rip;
    uint64_t user_rsp;
    uint8_t  is_user;

    // Approx memory usage (bytes), derived from tracked ranges.
    uint64_t mem_image_bytes;
    uint64_t mem_heap_bytes;
    uint64_t mem_mmap_bytes;
    uint64_t mem_stack_bytes;
    uint64_t mem_total_bytes;

    char name[MD64API_PID_NAME_MAX];
    char cwd[MD64API_CWD_MAX];
} md64api_pid_info_u;

#ifdef __cplusplus
}
#endif


static inline int md64api_get_pid_info(uint32_t pid, md64api_pid_info_u *out) {
    return (int)syscall(SYS_PIDINFO, (long)pid, (long)out, (long)sizeof(*out));
}

/* Time API: milliseconds since boot */
static inline uint64_t time_ms(void) {
    return (uint64_t)syscall(SYS_TIME, 0, 0, 0);
}

/* VGA color functions (deprecated, kept as stubs for compatibility) */
static inline void vga_set_color(uint8_t fg, uint8_t bg) {
    /* Deprecated: VGA syscalls removed. Use ANSI codes or DevFS instead. */
    (void)fg; (void)bg;
}

static inline uint8_t vga_get_color(void) {
    /* Deprecated: VGA syscalls removed. Return default color. */
    return 0x07;
}

static inline void vga_reset_color(void) {
    /* Deprecated: VGA syscalls removed. */
}

/* ANSI helpers (SGR). Works because kernel VGA driver parses ESC[...m. */
#define ANSI_ESC "\x1b"
#define ANSI_RESET ANSI_ESC "[0m"

/* ============================================================
   FS tracing (kernel-side timing logs)
   ============================================================ */
static inline int fs_trace_set(int enabled) {
    return (int)syscall(SYS_FS_TRACE, (long)enabled, 0, 0);
}


/* ============================================================
   FILE I/O OPERATIONS
   ============================================================ */

// Open a file - returns file descriptor or -1 on error
static inline int open(const char *pathname, int flags, int mode) {
    return (int)syscall(SYS_OPEN, (long)pathname, flags, mode);
}

// Close a file descriptor
static inline int close(int fd) {
    return (int)syscall(SYS_CLOSE, fd, 0, 0);
}

static inline int stat(const char *path, fs_file_info_t *out_info) {
    return (int)syscall(SYS_STAT, (long)path, (long)out_info, (long)sizeof(*out_info));
}

static inline long lseek(int fd, long offset, int whence) {
    return syscall(SYS_LSEEK, (long)fd, (long)offset, (long)whence);
}

// Read from a file descriptor
static inline ssize_t read(int fd, void *buf, size_t count) {
    return (ssize_t)syscall(SYS_READ, fd, (long)buf, count);
}

// MD64API GRP helper (must come after open/read/close)
static inline int md64api_grp_get_video0_info(md64api_grp_video_info_t *out) {
    if (!out) return -1;
    int fd = open(MD64API_GRP_DEFAULT_DEVICE, O_RDONLY, 0);
    if (fd < 0) return -2;
    ssize_t r = read(fd, out, sizeof(*out));
    close(fd);
    return (r == (ssize_t)sizeof(*out)) ? 0 : -3;
}

// Write to a file descriptor (binary safe)
static inline ssize_t sys_writefile_raw(int fd, const void *buf, size_t count) {
    long ret;
    __asm__ volatile (
        "mov %1, %%rax\n"
        "mov %2, %%rdi\n"
        "mov %3, %%rsi\n"
        "mov %4, %%rdx\n"
        "syscall\n"
        "mov %%rax, %0"
        : "=r"(ret)
        : "r"((long)SYS_WRITEFILE), "r"((long)fd), "r"((long)buf), "r"((long)count)
        : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
    );
    return (ssize_t)ret;
}

static inline void putc(char c) {
    sys_writefile_raw(STDOUT_FILENO, &c, 1);
}

static inline void puts_raw(const char *s) {
    if (!s) s = " ";
    sys_writefile_raw(STDOUT_FILENO, s, strlen(s));
}

static inline void __rovo_debug_sys_writefile(void) {
    volatile int *p = (volatile int*)0x0;
    (void)p;
}

static inline ssize_t write(int fd, const void *buf, size_t count) {
    return sys_writefile_raw(fd, buf, count);
}

/* ============================================================
   PRINTING UTILITIES
   ============================================================ */

static void print_uint(unsigned int n, int base, int upper) {
    char buf[32];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";

    if (n == 0) {
        putc('0');
        return;
    }

    int i = 0;
    while (n > 0) {
        buf[i++] = digits[n % base];
        n /= base;
    }

    while (i > 0)
        putc(buf[--i]);
}

static void print_uint_padded(unsigned int n, int base, int upper, int width, char pad) {
    char buf[32];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;

    // Handle 0 explicitly to get at least one digit
    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n > 0) {
            buf[i++] = digits[n % base];
            n /= base;
        }
    }

    // Calculate how much padding is needed
    // 'i' is currently the number of digits we have
    int padding_needed = width - i;

    // If padding with '0', we print the zeros before the number
    if (pad == '0') {
        while (padding_needed > 0) {
            putc('0');
            padding_needed--;
        }
    } else {
        // If padding with space, we print spaces before
        while (padding_needed > 0) {
            putc(' ');
            padding_needed--;
        }
    }

    // Print the digits in correct order (from buffer)
    while (i > 0) {
        putc(buf[--i]);
    }
}

static void print_int(int n) {
    if (n < 0) {
        putc('-');
        n = -n;
    }
    print_uint((unsigned)n, 10, 0);
}

static void print_ulong(unsigned long n, int base, int upper) {
    char buf[64];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";

    if (n == 0) {
        putc('0');
        return;
    }

    int i = 0;
    while (n > 0) {
        buf[i++] = digits[n % base];
        n /= base;
    }

    while (i > 0)
        putc(buf[--i]);
}

static void print_ulonglong(unsigned long long n, int base, int upper) {
    char buf[64];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";

    if (n == 0) {
        putc('0');
        return;
    }

    int i = 0;
    while (n > 0) {
        buf[i++] = digits[n % base];
        n /= base;
    }

    while (i > 0)
        putc(buf[--i]);
}

static void print_long(long n) {
    if (n < 0) {
        putc('-');
        n = -n;
    }
    print_ulong((unsigned long)n, 10, 0);
}

/* String conversion utilities */
static inline long strtol(const char *str, char **endptr, int base) {
    if (!str) {
        if (endptr) *endptr = (char *)str;
        return 0;
    }
    
    /* Skip leading whitespace */
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    
    int negative = 0;
    if (*str == '-') {
        negative = 1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    /* Auto-detect base if 0 */
    if (base == 0) {
        if (*str == '0' && (str[1] == 'x' || str[1] == 'X')) {
            base = 16;
            str += 2;
        } else if (*str == '0') {
            base = 8;
        } else {
            base = 10;
        }
    } else if (base == 16 && *str == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    }
    
    long result = 0;
    const char *start = str;
    
    while (*str) {
        int digit;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'z') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'Z') {
            digit = *str - 'A' + 10;
        } else {
            break;
        }
        
        if (digit >= base) break;
        result = result * base + digit;
        str++;
    }
    
    if (endptr) *endptr = (char *)str;
    return negative ? -result : result;
}


/* ============================================================
   printf()
   ============================================================ */

static int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        /* Fast path: write literal runs in one syscall (preserves UTF-8 bytes) */
        if (*fmt != '%') {
            const char *start = fmt;
            while (*fmt && *fmt != '%') fmt++;
            if (fmt > start) {
                write(STDOUT_FILENO, start, (size_t)(fmt - start));
                continue;
            }
        }

        /* Format handling */
        if (*fmt == '%') {
            fmt++;

            int longmod = 0;
            int longlongmod = 0;

            if (*fmt == 'l') {
                longmod = 1;
                fmt++;
                if (*fmt == 'l') {
                    longlongmod = 1;
                    fmt++;
                }
            }

            switch (*fmt) {
                case 'c': {
                    char ch = (char)va_arg(ap, int);
                    write(STDOUT_FILENO, &ch, 1);
                    break;
                }

                case 's': {
                    const char *s = va_arg(ap, const char*);
                    if (!s) s = "(null)";
                    write(STDOUT_FILENO, s, strlen(s));
                    break;
                }

                case 'd':
                case 'i':
                    if (longlongmod) {
                        long long n = va_arg(ap, long long);
                        if (n < 0) {
                            char m = '-';
                            write(STDOUT_FILENO, &m, 1);
                            n = -n;
                        }
                        print_ulonglong((unsigned long long)n, 10, 0);
                    } else if (longmod) {
                        print_long(va_arg(ap, long));
                    } else {
                        print_int(va_arg(ap, int));
                    }
                    break;

                case 'u':
                    if (longlongmod)
                        print_ulonglong(va_arg(ap, unsigned long long), 10, 0);
                    else if (longmod)
                        print_ulong(va_arg(ap, unsigned long), 10, 0);
                    else
                        print_uint(va_arg(ap, unsigned int), 10, 0);
                    break;

                case 'x': {
                    int width = 0;
                    char pad = ' ';
                
                    // If the next char is '0', set padding to zero
                    if (*fmt == '0') {
                        pad = '0';
                        fmt++;
                    }
                
                    // If the next char is a digit (e.g., '2'), set the width
                    if (*fmt >= '0' && *fmt <= '9') {
                        width = *fmt - '0';
                        fmt++;
                    }
                
                    // Call the new padded function
                    print_uint_padded(va_arg(ap, unsigned int), 16, 0, width, pad);
                    break;
                }

                case 'X':
                    if (longlongmod)
                        print_ulonglong(va_arg(ap, unsigned long long), 16, 1);
                    else if (longmod)
                        print_ulong(va_arg(ap, unsigned long), 16, 1);
                    else
                        print_uint(va_arg(ap, unsigned int), 16, 1);
                    break;

                case '%': {
                    char p = '%';
                    write(STDOUT_FILENO, &p, 1);
                    break;
                }

                default: {
                    char pct = '%';
                    write(STDOUT_FILENO, &pct, 1);
                    if (*fmt) write(STDOUT_FILENO, fmt, 1);
                    break;
                }
            }

            if (*fmt) fmt++;
        }
    }

    va_end(ap);
    return 0;
}

/* sprintf - format to a string buffer */
static int sprintf(char *str, const char *fmt, ...) {
    if (!str) return -1;
    
    va_list ap;
    va_start(ap, fmt);
    
    char *out = str;
    
    while (*fmt) {
        if (*fmt != '%') {
            const char *start = fmt;
            while (*fmt && *fmt != '%') fmt++;
            size_t len = fmt - start;
            memcpy(out, start, len);
            out += len;
            continue;
        }
        
        if (*fmt == '%') {
            fmt++;
            
            int longmod = 0;
            int longlongmod = 0;
            
            if (*fmt == 'l') {
                longmod = 1;
                fmt++;
                if (*fmt == 'l') {
                    longlongmod = 1;
                    fmt++;
                }
            }
            
            switch (*fmt) {
                case 'c': {
                    *out++ = (char)va_arg(ap, int);
                    break;
                }
                
                case 's': {
                    const char *s = va_arg(ap, const char*);
                    if (!s) s = "(null)";
                    size_t len = strlen(s);
                    memcpy(out, s, len);
                    out += len;
                    break;
                }
                
                case 'd':
                case 'i': {
                    char buf[32];
                    int val;
                    if (longlongmod) {
                        long long n = va_arg(ap, long long);
                        val = (int)n; /* truncate for simplicity */
                    } else if (longmod) {
                        val = (int)va_arg(ap, long);
                    } else {
                        val = va_arg(ap, int);
                    }
                    
                    int neg = 0;
                    if (val < 0) {
                        neg = 1;
                        val = -val;
                    }
                    
                    int i = 0;
                    if (val == 0) {
                        buf[i++] = '0';
                    } else {
                        while (val > 0) {
                            buf[i++] = '0' + (val % 10);
                            val /= 10;
                        }
                    }
                    
                    if (neg) *out++ = '-';
                    while (i > 0) *out++ = buf[--i];
                    break;
                }
                
                case 'u':
                case 'x':
                case 'X': {
                    char buf[32];
                    unsigned int val;
                    int base = (*fmt == 'u') ? 10 : 16;
                    const char *digits = (*fmt == 'X') ? "0123456789ABCDEF" : "0123456789abcdef";
                    
                    if (longlongmod) {
                        val = (unsigned int)va_arg(ap, unsigned long long);
                    } else if (longmod) {
                        val = (unsigned int)va_arg(ap, unsigned long);
                    } else {
                        val = va_arg(ap, unsigned int);
                    }
                    
                    int i = 0;
                    if (val == 0) {
                        buf[i++] = '0';
                    } else {
                        while (val > 0) {
                            buf[i++] = digits[val % base];
                            val /= base;
                        }
                    }
                    
                    while (i > 0) *out++ = buf[--i];
                    break;
                }
                
                case '%':
                    *out++ = '%';
                    break;
                    
                default:
                    *out++ = '%';
                    if (*fmt) *out++ = *fmt;
                    break;
            }
            
            if (*fmt) fmt++;
        }
    }
    
    *out = '\0';
    va_end(ap);
    return (int)(out - str);
}

/* ============================================================
   MEMORY FUNCTIONS
   ============================================================ */

/* sbrk must be declared before malloc() */
static inline void* sbrk(intptr_t inc) {
    return (void*)syscall(SYS_SBRK, inc, 0, 0);
}

/*
 * Userland heap allocator (simple free-list malloc).
 *
 * This replaces the bump allocator so graphics apps and shells don't leak memory forever.
 * It is NOT thread-safe (single-threaded userland).
 */

typedef struct uheap_hdr {
    size_t size;              /* payload size */
    struct uheap_hdr *next;   /* next free block */
    uint32_t magic;
    uint32_t free;
} uheap_hdr_t;

#define UHEAP_MAGIC 0xC0FFEE55u

static uheap_hdr_t *g_uheap_free = NULL;

static inline size_t uheap_align(size_t n) {
    return (n + 15) & ~((size_t)15);
}

/* Insert block into free list in ADDRESS ORDER (required for coalescing) */
static inline void uheap_insert_free(uheap_hdr_t *b) {
    b->free = 1;

    /* Find insertion point: keep list sorted by address */
    uheap_hdr_t **pp = &g_uheap_free;
    while (*pp && *pp < b) {
        pp = &(*pp)->next;
    }
    b->next = *pp;
    *pp = b;
}

static inline void uheap_remove_free(uheap_hdr_t *b) {
    uheap_hdr_t **pp = &g_uheap_free;
    while (*pp) {
        if (*pp == b) {
            *pp = b->next;
            b->next = NULL;
            return;
        }
        pp = &(*pp)->next;
    }
}

/*
 * Coalesce adjacent free blocks in the (address-ordered) free list.
 * Two blocks A and B are adjacent if:
 *   (uint8_t*)(A+1) + A->size == (uint8_t*)B
 * i.e. A's payload ends exactly where B's header begins.
 */
static inline void uheap_coalesce(void) {
    uheap_hdr_t *cur = g_uheap_free;
    while (cur && cur->next) {
        uheap_hdr_t *next = cur->next;

        /* Check adjacency */
        uint8_t *cur_end = (uint8_t*)(cur + 1) + cur->size;
        if (cur_end == (uint8_t*)next) {
            /* Merge: absorb next into cur */
            cur->size += sizeof(uheap_hdr_t) + next->size;
            cur->next = next->next;
            /* Wipe next's magic so it can't be double-freed */
            next->magic = 0;
            /* Don't advance cur — try to merge again with new next */
        } else {
            cur = cur->next;
        }
    }
}

static inline uheap_hdr_t* uheap_request_from_kernel(size_t payload) {
    if (payload > (size_t)-1 - sizeof(uheap_hdr_t)) return NULL;

    size_t total = sizeof(uheap_hdr_t) + payload;
    total = uheap_align(total);
    if (total < payload) return NULL;

    void *mem = sbrk((intptr_t)total);
    if (!mem || (intptr_t)mem < 0) return NULL;

    uheap_hdr_t *h = (uheap_hdr_t*)mem;
    h->size = total - sizeof(uheap_hdr_t);
    h->next = NULL;
    h->magic = UHEAP_MAGIC;
    h->free = 0;
    return h;
}

static inline void uheap_split_if_needed(uheap_hdr_t *h, size_t need) {
    size_t remain = (h->size > need) ? (h->size - need) : 0;
    if (remain < (sizeof(uheap_hdr_t) + 32)) return;

    uint8_t *base = (uint8_t*)(h + 1);
    uheap_hdr_t *nh = (uheap_hdr_t*)(base + need);
    nh->size = remain - sizeof(uheap_hdr_t);
    nh->next = NULL;
    nh->magic = UHEAP_MAGIC;
    nh->free = 1;

    h->size = need;

    uheap_insert_free(nh);
}

static inline void* malloc(size_t size) {
    if (size == 0) return NULL;
    size = uheap_align(size);

    /* first-fit */
    uheap_hdr_t *prev = NULL;
    uheap_hdr_t *cur = g_uheap_free;
    while (cur) {
        if (cur->magic != UHEAP_MAGIC) return NULL;
        if (cur->free && cur->size >= size) {
            if (prev) prev->next = cur->next;
            else g_uheap_free = cur->next;
            cur->next = NULL;
            cur->free = 0;

            uheap_split_if_needed(cur, size);
            return (void*)(cur + 1);
        }
        prev = cur;
        cur = cur->next;
    }

    uheap_hdr_t *h = uheap_request_from_kernel(size);
    if (!h) return NULL;

    uheap_split_if_needed(h, size);
    return (void*)(h + 1);
}

static inline void free(void *ptr) {
    if (!ptr) return;
    uheap_hdr_t *h = ((uheap_hdr_t*)ptr) - 1;
    if (h->magic != UHEAP_MAGIC) return;
    if (h->free) return;

    uheap_insert_free(h);
    uheap_coalesce();          /* <-- merge adjacent free blocks */
}

static inline void* calloc(size_t nmemb, size_t size) {
    /* Prevent integer overflow in multiplication */
    if (nmemb > 0 && size > (size_t)-1 / nmemb) return NULL;
    
    size_t total = nmemb * size;
    void *p = malloc(total);
    if (!p) return NULL;
    memset(p, 0, total);
    return p;
}

static inline void* realloc(void *ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) { free(ptr); return NULL; }

    uheap_hdr_t *h = ((uheap_hdr_t*)ptr) - 1;
    if (h->magic != UHEAP_MAGIC) return NULL;

    size_t new_sz = uheap_align(size);

    /* Already big enough — try to split off excess */
    if (h->size >= new_sz) {
        uheap_split_if_needed(h, new_sz);
        return ptr;
    }

    /*
     * Try in-place expansion: check if the next block in memory
     * is free and adjacent, and combined they're big enough.
     */
    uint8_t *cur_end = (uint8_t*)(h + 1) + h->size;
    uheap_hdr_t *next = (uheap_hdr_t*)cur_end;

    /* Validate next block is in free list and adjacent */
    int next_is_free = 0;
    if (next->magic == UHEAP_MAGIC && next->free == 1) {
        /* Double-check it's in free list */
        for (uheap_hdr_t *f = g_uheap_free; f; f = f->next) {
            if (f == next) {
                next_is_free = 1;
                break;
            }
        }
    }

    if (next_is_free) {
        /* Prevent overflow when combining sizes */
        if (next->size > (size_t)-1 - h->size - sizeof(uheap_hdr_t)) return NULL;
        
        size_t combined = h->size + sizeof(uheap_hdr_t) + next->size;
        if (combined >= new_sz) {
            /* Absorb next into h — expand in place, no copy needed */
            uheap_remove_free(next);
            next->magic = 0;
            h->size = combined;
            uheap_split_if_needed(h, new_sz);
            return ptr;
        }
    }

    /* Fall back: allocate new, copy, free old */
    void *n = malloc(new_sz);
    if (!n) return NULL;
    memcpy(n, ptr, h->size);
    free(ptr);
    return n;
}
/* ============================================================
   PROCESS FUNCTIONS
   ============================================================ */

__attribute__((noreturn)) static inline void exit(int status) {
    syscall(SYS_EXIT, status, 0, 0);
    /* SYS_EXIT must not return; if it does, halt here. */
    for (;;) { __asm__ volatile("hlt"); }
}

static inline void exec(const char *str) {
    syscall(SYS_EXEC, (long)str, 0 , 0);
}

/* POSIX execve wrapper. On failure returns -1 and sets errno.
 * On success does not return.
 */
static inline int execve(const char *path, char *const argv[], char *const envp[]) {
    long r = syscall(SYS_EXECVE, (long)path, (long)argv, (long)envp);
    if (r < 0) { errno = (int)(-r); return -1; }
    return (int)r;
}

/* Environment (kernel-managed per-process) */
static inline int putenv(const char *kv) {
    long r = syscall(SYS_PUTENV, (long)kv, 0, 0);
    if (r < 0) { errno = (int)(-r); return -1; }
    return 0;
}

/* Returns pointer to a static buffer (overwritten per call), or NULL if missing. */
static inline const char *getenv(const char *key) {
    static char buf[256];
    long r = syscall(SYS_GETENV, (long)key, (long)buf, (long)sizeof(buf));
    if (r < 0) { errno = (int)(-r); return NULL; }
    return buf;
}

/* Serialize env into user-provided buffer as newline-separated KEY=VALUE lines.
 * Returns bytes written or -1 with errno set.
 */
static inline int envlist(char *buf, size_t buflen) {
    long r = syscall(SYS_ENVLIST, (long)buf, (long)buflen, 0);
    if (r < 0) { errno = (int)(-r); return -1; }
    return (int)r;
}

/* Streaming env listing.
 * offset is an in/out cursor (start at 0). Returns bytes written this call.
 */
static inline int envlist2(size_t *offset, char *buf, size_t buflen) {
    long r = syscall(SYS_ENVLIST2, (long)offset, (long)buf, (long)buflen);
    if (r < 0) { errno = (int)(-r); return -1; }
    return (int)r;
}

static inline int unsetenv(const char *key) {
    long r = syscall(SYS_UNSETENV, (long)key, 0, 0);
    if (r < 0) { errno = (int)(-r); return -1; }
    return 0;
}

static inline int setenv(const char *key, const char *val) {
    if (!key || !val) { errno = EINVAL; return -1; }
    char kv[512];
    size_t klen = strlen(key);
    size_t vlen = strlen(val);
    if (klen + 1 + vlen + 1 > sizeof(kv)) { errno = E2BIG; return -1; }
    memcpy(kv, key, klen);
    kv[klen] = '=';
    memcpy(kv + klen + 1, val, vlen);
    kv[klen + 1 + vlen] = 0;
    return putenv(kv);
}

static inline int dup(int oldfd) {
    return (int)syscall(SYS_DUP, (long)oldfd, 0, 0);
}

static inline int dup2(int oldfd, int newfd) {
    return (int)syscall(SYS_DUP2, (long)oldfd, (long)newfd, 0);
}

static inline int pipe(int fds[2]) {
    return (int)syscall(SYS_PIPE, (long)fds, 0, 0);
}

static inline int geteuid(void) {
    return (int)syscall(SYS_GETEUID, 0, 0, 0);
}

static inline int getgid(void) {
    return (int)syscall(SYS_GETGID, 0, 0, 0);
}

static inline int getegid(void) {
    return (int)syscall(SYS_GETEGID, 0, 0, 0);
}

static inline int setgid(int gid) {
    return (int)syscall(SYS_SETGID, (long)gid, 0, 0);
}

static inline int fork(void) {
    long r = syscall(SYS_FORK, 0, 0, 0);
    if (r < 0) { errno = (int)(-r); return -1; }
    return (int)r;
}

static inline int getpid(void) {
    return (int)syscall(SYS_GETPID, 0, 0, 0);
}

static inline int getppid(void) {
    return (int)syscall(SYS_GETPPID, 0, 0, 0);
}

/* POSIX waitpid (Linux semantics). */
#ifndef WNOHANG
#define WNOHANG 1
#endif
static inline int waitpid(int pid, int *status, int options) {
    int kopt = 0;
    if (options & WNOHANG) kopt |= 1; /* WAITX_WNOHANG */
    long r = syscall(SYS_WAITX, (long)pid, (long)status, (long)kopt);
    if (r < 0) { errno = (int)(-r); return -1; }
    return (int)r;
}

/* POSIX wait - wait for any child process */
static inline int wait(int *status) {
    long r = syscall(SYS_WAIT, (long)status, 0, 0);
    if (r < 0) { errno = (int)(-r); return -1; }
    return (int)r;
}

static inline int getuid(void) {
    return (int)syscall(SYS_GETUID, 0, 0, 0);
}

static inline int setuid(int uid) {
    return (int)syscall(SYS_SETUID, (long)uid, 0, 0);
}


static inline void sleep(unsigned int sec) {
    syscall(SYS_SLEEP, sec, 0, 0);
}

static inline int kill(int pid, int sig) {
    return (int)syscall(SYS_KILL, pid, sig, 0);
}

// Signal handler type
typedef void (*sighandler_t)(int);

// Install signal handler (returns old handler or SIG_ERR on error)
static inline sighandler_t signal(int signum, sighandler_t handler) {
    return (sighandler_t)syscall(SYS_SIGNAL, (long)signum, (long)handler, 0);
}

// Raise a signal to current process
static inline int raise(int sig) {
    return (int)syscall(SYS_RAISE, (long)sig, 0, 0);
}

// File descriptor injection (for TTY manager)
static inline int fd_inject(int pid, int fd, void *fd_obj) {
    return (int)syscall(SYS_FD_INJECT, (long)pid, (long)fd, (long)fd_obj);
}

// Directory operations
static inline int opendir(const char *path) {
    return (int)syscall(SYS_OPENDIR, (uint64_t)path, 0, 0);
}

// Change current working directory
static inline int chdir(const char *path) {
    return (int)syscall(SYS_CHDIR, (uint64_t)path, 0, 0);
}

// Get current working directory
static inline char *getcwd(char *buf, size_t size) {
    return (char*)syscall(SYS_GETCWD, (uint64_t)buf, (uint64_t)size, 0);
}

static inline int readdir(int fd, char *name_buf, size_t buf_size, int *is_dir, uint32_t *size) {
    uint64_t ret;
    __asm__ volatile (
        "mov %1, %%rax\n"
        "mov %2, %%rdi\n"
        "mov %3, %%rsi\n"
        "mov %4, %%rdx\n"
        "mov %5, %%r10\n"
        "mov %6, %%r8\n"
        "syscall\n"
        "mov %%rax, %0\n"
        : "=r"(ret)
        : "r"((uint64_t)SYS_READDIR), "r"((uint64_t)fd), "r"((uint64_t)name_buf),
          "r"((uint64_t)buf_size), "r"((uint64_t)is_dir), "r"((uint64_t)size)
        : "rax", "rdi", "rsi", "rdx", "r10", "r8", "rcx", "r11", "memory"
    );
    return (int)ret;
}

static inline int closedir(int fd) {
    return (int)syscall(SYS_CLOSEDIR, (uint64_t)fd, 0, 0);
}

// Create a directory. NOTE: kernel rejects DEVFS paths.
static inline int mkdir(const char *path) {
    return (int)syscall(SYS_MKDIR, (uint64_t)path, 0, 0);
}

// Remove a directory. NOTE: kernel rejects DEVFS paths.
static inline int rmdir(const char *path) {
    return (int)syscall(SYS_RMDIR, (uint64_t)path, 0, 0);
}

// Remove a file. NOTE: kernel rejects DEVFS paths.
static inline int unlink(const char *path) {
    return (int)syscall(SYS_UNLINK, (uint64_t)path, 0, 0);
}

#ifndef MODUOS_FS_MKFS_H
#define MODUOS_FS_MKFS_H

#include <stdint.h>

// mkfs request for SYS_VFS_MKFS.
// Strings are NUL-terminated.
typedef struct {
    char fs_name[16];        // e.g. "fat32", "ext2"
    char label[16];          // volume label (optional)

    int32_t vdrive_id;
    uint32_t start_lba;
    uint32_t sectors;        // partition length in 512-byte sectors

    uint32_t flags;          // vfs_mkfs_req_t flags

    // flags bits
    //  - allows fat32 mkfs on volumes >32GiB when auto-picking cluster size
    //    (Windows formatter typically refuses without special tooling)
    #define VFS_MKFS_FLAG_FORCE  (1u << 0)

    // FAT32-specific (0 => kernel decides default)
    uint32_t fat32_sectors_per_cluster;
} vfs_mkfs_req_t;

#endif

#ifndef MODUOS_FS_PART_H
#define MODUOS_FS_PART_H

#include <stdint.h>

// Request/response for SYS_VFS_GETPART

typedef struct {
    int32_t vdrive_id;
    int32_t part_no; // 1..4
} vfs_part_req_t;

typedef struct {
    uint32_t start_lba;
    uint32_t sectors;
    uint8_t type;
    uint8_t _pad[3];
} vfs_part_info_t;

// Request for SYS_VFS_MBRINIT: write a minimal MBR with a single primary partition.
// sectors==0 means "use disk size - start_lba".
// flags bit0: force overwrite even if a valid MBR signature exists.
typedef struct {
    int32_t vdrive_id;
    uint32_t start_lba;   // typically 2048
    uint32_t sectors;     // 0=auto
    uint8_t type;         // MBR partition type (0 => default 0x83)
    uint8_t bootable;     // 0/1
    uint16_t flags;       // bit0=force
} vfs_mbrinit_req_t;

#endif

static inline int vfs_mkfs(const vfs_mkfs_req_t *req) {
    return (int)syscall(SYS_VFS_MKFS, (uint64_t)req, 0, 0);
}

static inline int vfs_getpart(const vfs_part_req_t *req, vfs_part_info_t *out) {
    return (int)syscall(SYS_VFS_GETPART, (uint64_t)req, (uint64_t)out, 0);
}

static inline int vfs_mbrinit(const vfs_mbrinit_req_t *req) {
    return (int)syscall(SYS_VFS_MBRINIT, (uint64_t)req, 0, 0);
}

static inline int userfs_register(const userfs_user_node_t *node) {
    return (int)syscall(SYS_USERFS_REGISTER, (uint64_t)node, 0, 0);
}

static inline int userfs_register_path(const char *path, uint32_t perms) {
    userfs_user_node_t node;
    memset(&node, 0, sizeof(node));
    node.path = path;
    node.owner_id = "userland";
    node.perms = perms;
    return userfs_register(&node);
}

static inline ssize_t invoke(int fd, const void *in_buf,  size_t in_size, void *out_buf, size_t out_size) {
    long ret = syscall5(SYS_INVOKE,
                        (long)fd,
                        (long)in_buf,
                        (long)in_size,
                        (long)out_buf,
                        (long)out_size);

    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }

    return (ssize_t)ret;
}

#define MAP_FAILED ((void*)-1)

/* mmap a device fd (e.g. $/dev/mvc/mvi0 ring buffer or framebuffer).
 * hint=NULL lets the kernel choose the VA.
 * prot: 1=R 2=W 3=RW. flags: 0=normal.
 * offset: device-specific region (MVC3_OFF_RING, MVC3_OFF_FB, etc.).
 * Returns mapped user VA or MAP_FAILED.
 */
static inline void *dev_mmap(int fd, void *hint, size_t length,
                              int prot, int flags, uint64_t offset) {
    register long _num    __asm__("rax") = (long)SYS_DEV_MMAP;
    register long _fd     __asm__("rdi") = (long)fd;
    register long _hint   __asm__("rsi") = (long)hint;
    register long _length __asm__("rdx") = (long)length;
    register long _prot   __asm__("r10") = (long)prot;
    register long _flags  __asm__("r8")  = (long)flags;
    register long _offset __asm__("r9")  = (long)offset;
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "r"(_num), "r"(_fd), "r"(_hint), "r"(_length),
          "r"(_prot), "r"(_flags), "r"(_offset)
        : "rcx", "r11", "memory"
    );
    if (ret == -1L) { errno = ENOMEM; return MAP_FAILED; }
    return (void *)(uintptr_t)ret;
}

static inline int mount_drive(int vdrive_id, uint32_t partition_lba, int fs_type) {
    return (int)syscall(SYS_MOUNT, (long)vdrive_id, (long)partition_lba, (long)fs_type);
}

static inline int unmount_slot(int slot) {
    return (int)syscall(SYS_UNMOUNT, (long)slot, 0, 0);
}

static inline int list_mounts(fs_mount_info_t *buf, size_t max_entries) {
    return (int)syscall(SYS_MOUNTS, (long)buf, (long)max_entries, 0);
}

int md_main(long argc, char** argv);

#ifndef LIBC_NO_START
__attribute__((noreturn))
__attribute__((noinline))
__attribute__((used))
void _start(long argc, char** argv) {
    // ModuOS start wrapper / ABI
    int mdm = md_main(argc, argv);

    input_flush();

    if (mdm) {
        exit(mdm);
    } else {
        exit(0);
    }
}
#endif /* LIBC_NO_START */


// Simple sscanf for parsing integers (supports "%d" format only)
static inline int sscanf(const char *str, const char *format, ...) {
    int *args[16];
    int arg_count = 0;
    
    // Count %d in format string
    const char *f = format;
    while (*f) {
        if (*f == '%' && *(f+1) == 'd') {
            arg_count++;
            f += 2;
        } else {
            f++;
        }
    }
    
    // Get variadic arguments
    __builtin_va_list ap;
    __builtin_va_start(ap, format);
    for (int i = 0; i < arg_count; i++) {
        args[i] = __builtin_va_arg(ap, int*);
    }
    __builtin_va_end(ap);
    
    // Parse the string
    int parsed = 0;
    const char *s = str;
    
    for (int i = 0; i < arg_count && *s; i++) {
        while (*s == ' ' || *s == '\t') s++;
        
        int sign = 1;
        if (*s == '-') {
            sign = -1;
            s++;
        }
        
        int value = 0;
        int found = 0;
        while (*s >= '0' && *s <= '9') {
            value = value * 10 + (*s - '0');
            s++;
            found = 1;
        }
        
        if (found) {
            *args[i] = sign * value;
            parsed++;
        } else {
            break;
        }
    }
    
    return parsed;
}

/* Random number generation (LCG) */
static uint32_t __rand_seed = 1;

static inline void srand(unsigned int seed) {
    __rand_seed = seed;
}

static inline int rand(void) {
    __rand_seed = __rand_seed * 1103515245 + 12345;
    return (int)((__rand_seed / 65536) % 32768);
}

static inline void rt_sigreturn(void) {
    __asm__ volatile (
        "mov %0, %%rax\n"
        "syscall\n"
        :: "i"(SYS_RT_SIGRETURN)
        : "rax", "rcx", "r11", "memory"
    );
}