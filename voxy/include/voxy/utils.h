#ifndef VOXY_UTILS_H
#define VOXY_UTILS_H

// https://stackoverflow.com/questions/15832301/understanding-container-of-macro-in-the-linux-kernel
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})


#define SWAP(a, b) do { typeof(a) tmp = a; a = b; b = tmp; } while(0)

#endif // VOXY_UTILS_H
