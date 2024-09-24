#ifndef LIBSERDE_DESERIALIZER_H
#define LIBSERDE_DESERIALIZER_H

#include <stddef.h>

typedef struct libserde_deserializer *libserde_deserializer_t;

/// Create a deserializer that writes to file specified by path.
///
/// Returns NULL on failure.
libserde_deserializer_t libserde_deserializer_create(const char *path);

/// Destroy the deserializer.
void libserde_deserializer_destroy(libserde_deserializer_t deserializer);

/// Deserialize data.
///
/// TODO: Consider having an API for serializing data structures if possible.
///
/// Returns 0 on success.
int libserde_deserializer_read(libserde_deserializer_t deserializer, void *data, size_t length);

/// Deserialize a checksum.
///
/// The checksum is computed from data read starting from the previous call of
/// this function or when deserializer is created if this is the first time this
/// function is being called.
///
/// This is meant to catch incompatible serialization and deserialization code
/// for example because a data field is added/removed.
///
/// FIXME: Figure out a way to distinguish general error and checksum error.
///
/// Returns 0 on success.
int libserde_deserializer_read_checksum(libserde_deserializer_t deserializer);

/// Helper macros.
#define libserde_deserializer_try_read(deserializer, data, length) do { if(libserde_deserializer_read(deserializer, data, length) != 0) return -1; } while(0)
#define libserde_deserializer_try_read_checksum(deserializer) do { if(libserde_deserializer_read_checksum(deserializer) != 0) return -1; } while(0)


#endif // LIBSERDE_DESERIALIZER_H
