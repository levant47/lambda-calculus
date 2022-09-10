// TODO: disabling struct padding doesn't work as of right now, we have to fix that,
// and also disable padding for mystd (or maybe even globally for anyone who includes mystd)

typedef s32 Descriptor;

const Descriptor STDIN = 0;
const Descriptor STDOUT = 1;
const Descriptor STDERR = 2;

static inline s64 read
(
    Descriptor file_descriptor,
    void* buffer,
    u64 buffer_size
)
{
    s64 result;
    asm volatile
    (
        "syscall"
        : "=a"(result)
        : "a"(0), "D"(file_descriptor), "S"(buffer), "d"(buffer_size)
        : "rcx", "r11", "memory"
    );
    return result;
}

static inline s64 write
(
    Descriptor file_descriptor,
    const void* buffer,
    u64 buffer_size
)
{
    s64 result;
    asm volatile
    (
        "syscall"
        : "=a"(result)
        : "a"(1), "D"(file_descriptor), "S"(buffer), "d"(buffer_size)
        : "rcx", "r11", "memory"
    );
    return result;
}

enum OpenFlag
{
    OpenFlagReadOnly = 0,
    OpenFalgWriteOnly = 1,
    OpenFlagReadWrite = 2,
    OpenFlagCreate = 0x40,
    OpenFlagExclusive = 0x80,
    OpenFlagNoTty = 0x100,
    OpenFlagTruncate = 0x200,
    OpenFlagDirectory = 0x10000,
    OpenFlagNoFollow = 0x20000,
    OpenFlagTemporary = 0x410000,
    OpenFlagCloseOnExecute = 0x80000,
};

static inline OpenFlag operator|(OpenFlag left, OpenFlag right)
{
    return (OpenFlag)((s32)left | (s32)right);
}

static inline Descriptor open
(
    const char* path,
    OpenFlag flags,
    s32 mode = 0
)
{
    s32 result;
    asm volatile
    (
        "syscall"
        : "=a"(result)
        : "a"(2), "D"(path), "S"(flags), "d"(mode)
        : "rcx", "r11", "memory"
    );
    return result;
}

static inline s32 close(Descriptor file_descriptor)
{
    s32 result;
    asm volatile
    (
        "syscall"
        : "=a"(result)
        : "a"(3), "D"(file_descriptor)
        : "rcx", "r11", "memory"
    );
    return result;
}

// TODO: this structure is wrong (except for the size field)
struct StatResult
{
    u64 dev;
    u64 ino;
    u32 mode;
    u64 nlink;
    u32 uid;
    u32 gid;
    u64 rdev;
    s64 size;
    s64 atime;
    s64 mtime;
    s64 ctime;
    s64 blksize;
    s64 blocks;
    u32 attr;
    byte zero[40];
};

static inline s32 stat(const char* path, StatResult* stat_result_address)
{
    s32 result;
    asm volatile
    (
        "syscall"
        : "=a"(result)
        :
            "a"(4),
            "D"(path),
            "S"(stat_result_address)
        : "rcx", "r11", "memory"
    );
    return result;
}

enum PollEvent : u16
{
    PollEventDataAvailable = 1,
    PollEventHangUp = 0x10, // cannot be requested, can only be returned
};

static inline PollEvent operator|(PollEvent left, PollEvent right)
{
    return (PollEvent)((u16)left | (u16)right);
}

#pragma pack(push, 1)
struct PollParameter
{
    Descriptor descriptor;
    PollEvent requested_events;
    PollEvent returned_events;
};
#pragma pack(pop)

s64 POLL_TIMEOUT_RETURN_IMMEDIATELY = 0;

static inline s32 poll(PollParameter* poll_parameters, u64 poll_parameters_count, s64 timeout)
{
    s32 result;
    asm volatile
    (
        "syscall"
        : "=a"(result)
        :
            "a"(7),
            "D"(poll_parameters),
            "S"(poll_parameters_count),
            "d"(timeout)
        : "rcx", "r11", "memory"
    );
    return result;
}

static inline s32 mprotect(void* start, u64 length, u64 protection)
{
    s32 result;
    asm volatile
    (
        "syscall"
        : "=a"(result)
        :
            "a"(10),
            "D"(start),
            "S"(length),
            "d"(protection)
        : "rcx", "r11", "memory"
    );
    return result;
}

static inline void* brk(void* new_break)
{
    void* result;
    asm volatile
    (
        "syscall"
        : "=a"(result)
        : "a"(12), "D"(new_break)
        : "rcx", "r11", "memory"
    );
    return result;
}

struct SleepTime
{
    s64 seconds;
    s64 nanoseconds;
};

static inline s32 nanosleep(SleepTime* requested, SleepTime* remaining = nullptr)
{
    s32 result;
    asm volatile
    (
        "syscall"
        : "=a"(result)
        : "a"(35), "D"(requested), "S"(remaining)
        : "rcx", "r11", "memory"
    );
    return result;
}

enum SocketDomain : u16
{
    SocketDomainUnix = 1,
    SocketDomainInternet = 2,
};

enum SocketType
{
    SocketTypeTcp = 1,
    SocketTypeUdp = 2,
};

static inline Descriptor socket(SocketDomain domain, SocketType type, s32 protocol = 0 /* unused */)
{
    Descriptor result;
    asm volatile
    (
        "syscall"
        : "=a"(result)
        : "a"(41), "D"(domain), "S"(type), "d"(protocol)
        : "rcx", "r11", "memory"
    );
    return result;
}

#define IP(first, second, third, fourth) fourth << 24 \
    | third << 16 \
    | second << 8 \
    | first

struct InternetSocketAddress
{
    SocketDomain family;
    u16 port;
    u16 address;
    byte zero[10];

};

struct UnixSocketAddress
{
    SocketDomain family;
    char path[108];
};

static inline s32 connect
(
    Descriptor socket_descriptor,
    void* socket_address,
    s32 socket_address_size
)
{
    s32 result;
    asm volatile
    (
        "syscall"
        : "=a"(result)
        :
            "a"(42),
            "D"(socket_descriptor),
            "S"(socket_address),
            "d"(socket_address_size)
        : "rcx", "r11", "memory"
    );
    return result;
}

static inline void exit(s64 exit_code)
{
    asm volatile
    (
        "syscall"
        :
        : "a"(60), "D"(exit_code)
        :
    );
}
