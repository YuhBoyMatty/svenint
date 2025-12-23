// https://github.com/Number571/C/blob/master/Cryptography/GOST_28147.c
// GOST 28147-89

#ifndef SINT_GOST_28147_H
#define SINT_GOST_28147_H

#include <stdint.h>
#ifndef WIN32
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_LENGTH 32

typedef enum crypt_mode
{
	GOST_ENCRYPT = 0,
	GOST_DECRYPT
} crypt_mode_t;

size_t GOST_28147( uint8_t *to, crypt_mode_t mode, uint8_t *key256b, uint8_t *from, size_t length );

inline size_t get_encrypt_length( size_t length )
{
	return length % 8 == 0 ? length : length + ( 8 - ( length % 8 ) );
}

inline size_t encrypt_message( const char *message, const char *key, const char *output, size_t length )
{
	return GOST_28147( (uint8_t *)output, GOST_ENCRYPT, (uint8_t *)key, (uint8_t *)message, length );
}

inline size_t decrypt_message( const char *message, const char *key, const char *output, size_t length )
{
	return GOST_28147( (uint8_t *)output, GOST_DECRYPT, (uint8_t *)key, (uint8_t *)message, length );
}

#ifdef __cplusplus
}
#endif

#endif // SINT_GOST_28147_H