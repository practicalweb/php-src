/* $Id: crypt_freesec.h,v 1.1 2008/07/28 11:34:53 pajoye Exp $ */
#ifndef _CRYPT_FREESEC_H
#define _CRYPT_FREESEC_H

#if PHP_WIN32
# include "win32/php_stdint.h"
#else
# include <stdint.h>
#endif

#define MD5_HASH_MAX_LEN 120

struct php_crypt_extended_data {
	int initialized;
	u_int32_t saltbits;
	u_int32_t old_salt;
	u_int32_t en_keysl[16], en_keysr[16];
	u_int32_t de_keysl[16], de_keysr[16];
	u_int32_t old_rawkey0, old_rawkey1;
	char output[21];
};

/*
 * _crypt_extended_init() must be called explicitly before first use of
 * _crypt_extended_r().
 */

void _crypt_extended_init(void);

char *_crypt_extended_r(const char *key, const char *setting,
	struct php_crypt_extended_data *data);

#endif