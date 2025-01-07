#ifndef LIBCORE_UTILS_H
#define LIBCORE_UTILS_H

// https://stackoverflow.com/questions/15832301/understanding-container-of-macro-in-the-linux-kernel
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})


#define SWAP(a, b) do { typeof(a) tmp = a; a = b; b = tmp; } while(0)

#define MAX(a, b) ({ typeof(a) _a = (a); typeof(a) _b = (b); _a > _b ? _a : _b; })
#define MIN(a, b) ({ typeof(a) _a = (a); typeof(a) _b = (b); _a < _b ? _a : _b; })

#endif // LIBCORE_UTILS_H
