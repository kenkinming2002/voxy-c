#ifndef LIBSERDE_SERIALIZER_H
#define LIBSERDE_SERIALIZER_H

#include <stddef.h>

typedef struct libserde_serializer *libserde_serializer_t;

/// Create a serializer that writes to file specified by path.
///
/// Returns NULL on failure.
libserde_serializer_t libserde_serializer_create(const char *path);

/// Create a serializer that writes data to memory.
///
/// The allocated buffer is returned via buf and len arguments which will be
/// updated once the serializer is destroyed.
///
/// Returns NULL on failure.
libserde_serializer_t libserde_serializer_create_mem(char **buf, size_t *len);

/// Create a serializer that writes to file specified by path, without
/// clobbering existing file.
///
/// Returns NULL on failure, where exist will be set to a non-zero value if it
/// is due to the file already existing.
libserde_serializer_t libserde_serializer_create_exclusive(const char *path, int *exist);

/// Destroy the serializer.
void libserde_serializer_destroy(libserde_serializer_t serializer);

/// Serialize data.
///
/// TODO: Consider having an API for serializing data structures if possible.
///
/// Returns 0 on success.
int libserde_serializer_write(libserde_serializer_t serializer, const void *data, size_t length);

/// Serialize a checksum.
///
/// The checksum is computed from data written starting from the previous call
/// of this function or when serializer is created if this is the first time
/// this function is being called.
///
/// This is meant to catch incompatible serialization and deserialization code
/// for example because a data field is added/removed.
///
/// Returns 0 on success.
int libserde_serializer_write_checksum(libserde_serializer_t serializer);

/// Helper macros.
#define libserde_serializer_try_write(serializer, v, label) do { if(libserde_serializer_write((serializer), &(v), sizeof (v)) != 0) goto label; } while(0)
#define libserde_serializer_try_write_checksum(serializer, label) do { if(libserde_serializer_write_checksum((serializer)) != 0) goto label; } while(0)

#endif // LIBSERDE_SERIALIZER_H
