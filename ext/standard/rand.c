/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2001 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Zeev Suraski <zeev@zend.com>                                |
   |          Pedro Melo <melo@ip.pt>                                     |
   +----------------------------------------------------------------------+
 */
/* $Id: rand.c,v 1.31.4.4 2001/09/02 19:42:29 jeroen Exp $ */

#include <stdlib.h>

#include "php.h"
#include "php_math.h"
#include "php_rand.h"
#include "php_ini.h"

#include "zend_execute.h"

#include "basic_functions.h"

/* See php_rand.h for information about layout */

#if 0
#define RANDGEN_ENTRY(intval, lower, upper, has_seed) \
	php_randgen_entry[intval] = { \
		(has_seed) ? php_rand_##lower : NULL, \
		php_rand_##lower, \
		PHP_RANDMAX_##upper, \
		"lower" \
	};
#endif

php_randgen_entry (*php_randgen_entries)[PHP_RAND_NUMRANDS];

/* TODO: make sure this will be called */
PHP_MINIT_FUNCTION(rand)
{
	/* call: rand_sys, rand_mt, etc */
}

/* TODO: check that this function is called on the start of each script
 * execution: not more often, not less often.
 *
 * Note that system rand is inherently thread-unsafe: A different thread can
 * always eat up some rand()'s, and thus nuking your expected sequence.
 * Another reason to use MT...
 */
PHP_RINIT_FUNCTION(rand)
{
	register int i;

	/* seed all number-generators */
	/* FIXME: or seed relevant numgen on init/update ini-entry? */
	for (i = 0 ; i < PHP_RAND_NUMRANDS ; i++) {
		if (PHP_HAS_SRAND(i)) {
#define SRAND_A_RANDOM_SEED (time(0) * getpid() * (php_combined_lcg(TSRMLS_C) * 10000.0)) /* something with microtime? */
			PHP_SRAND(i,SRAND_A_RANDOM_SEED);
		}
	}
}

/* INI */
static int randgen_str_to_int(char *str, int strlen)
{
	register int i;

	for (i = 0 ; i < PHP_RAND_NUMRANDS ; i++) {
		if (!strcasecmp(str, PHP_RAND_INISTR(i)))
			return i;
	}
	return -1;
}
	
/* FIXME: check that this is called on initial ini-parsing too */
/* FIXME: what if no ini-entry was present? */
static PHP_INI_MH(OnUpdateRandGen)
{
	/* Set BG(rand_generator) to the correct integer value indicating
	 * ini-setting */
	BG(rand_generator) = randgen_str_to_int(new_value, new_value_length);
	if (BG(rand_generator) == -1) {
		/* FIXME: is this possible? What happens if this occurs during
		 * ini-parsing at startup? */
		php_error(E_WARNING,"Invalid value for random_number_generator: \"%s\"", new_value);
		/* Fallback: */
		BG(rand_generator) = PHP_RAND_DEFAULT;
	}
#ifdef DEBUG_RAND
	printf("\nRAND-INI updated: %d\n",BG(rand_generator));
#endif
	return SUCCESS;
}

PHP_INI_BEGIN()
	/* FIXME: default is hardcoded here, this is the second place */
	PHP_INI_ENTRY("random_number_generator", "mt", PHP_INI_ALL, OnUpdateRandGen)
PHP_INI_END()

/* srand */

/* {{{ PHPAPI void php_srand(void) */
PHPAPI void php_srand(void)
{
	BG(rand_generator_current) = BG(rand_generator);
	PHP_SRAND(BG(rand_generator), SRAND_A_RANDOM_SEED);
}
/* }}} */

/* {{{ [mt_]srand common */
#define pim_srand_common(name,type) 				\
PHP_FUNCTION(name)							\
{											\
	zval **arg;								\
											\
	if (ZEND_NUM_ARGS() != 1) {				\
		WRONG_PARAM_COUNT;					\
	}										\
	zend_get_parameters_ex(1, &arg);		\
	convert_to_long_ex(arg);				\
											\
	BG(rand_generator_current) = type;		\
	PHP_SRAND(type, Z_LVAL_PP(arg)); 		\
}
/* }}} */

/* {{{ proto void srand(int seed)
   Seeds random number generator */
pim_srand_common(srand,PHP_RAND_SYS)
/* }}} */

/* {{{ proto void mt_srand(int seed)
   Seeds random number generator */
pim_srand_common(mt_srand,PHP_RAND_MT)
/* }}} */

/* rand */

/* {{{ PHPAPI long php_rand(void) */
PHPAPI long php_rand(void)
{
	return PHP_RAND(BG(rand_generator_current));
}
/* }}} */

/* {{{ macro: PHP_RAND_RANGE */
#define PHP_RAND_RANGE(which,min,max,result) {  \
    /*
     * A bit of tricky math here.  We want to avoid using a modulus because
     * that simply tosses the high-order bits and might skew the distribution
     * of random values over the range.  Instead we map the range directly.
     *
     * We need to map the range from 0...M evenly to the range a...b
	 * Expressed in real numbers, this becomes:
	 *
	 * 				[0,M+1[ mapped to [a,b+1[
	 * 
     * Let n = the random number and n' = the mapped random number
	 * So the formula needs to be:
	 *
     *               n' = a + n((b+1)-a)/(m+1) 
     *
	 *  This isn't perfect, because n only takes integer values. So when a..b
	 *  spans a significant portion of 0..M, some numbers have nearly twice as
	 *  much chance. But since twice a very small chance is still a very small
	 *  chance, it's ignored.
	 *
	 *  --Rasmus and Jeroen
     */								\
	(result) = PHP_RAND(which);		\
	if ((max) < (min)) {			\
		php_error(E_WARNING, "%s():  Invalid range:  %ld..%ld (minimum can't be larger than maximum)", \
			get_active_function_name(TSRMLS_C), (min), (max)); \
	} else if ( (max) - (min) > PHP_RANDMAX(which) ) { \
		/* TODO: this can done better, get two numbers and combine... */ \
		php_error(E_WARNING, "%s():  Invalid range:  %ld..%ld (can't give that much randomness)",  \
			get_active_function_name(TSRMLS_C), (min), (max)); \
	} \
	(result) = (min) + (long) ((double)((max)-(min)+1) * (result)/(PHP_RANDMAX(which)+1.0)); \
}
/* }}} */

/* {{{ PHPAPI long php_rand_range(long min, long max) */
PHPAPI long php_rand_range(long min, long max)
{
	register long result;
	PHP_RAND_RANGE(BG(rand_generator_current), min, max, result);
	return result;
}
/* }}} */

/* {{{ [mt_]rand common */
#define PHP_FUNCTION_RAND(name,which)								\
PHP_FUNCTION(name)													\
{																	\
	zval **min, **max;												\
																	\
	switch (ZEND_NUM_ARGS()) {										\
		case 0:														\
			RETURN_LONG(PHP_RAND(which));							\
		case 2:														\
			if (zend_get_parameters_ex(2, &min, &max)==FAILURE) {	\
				RETURN_FALSE;										\
			}														\
			convert_to_long_ex(min);								\
			convert_to_long_ex(max);								\
			Z_TYPE_P(return_value) = IS_LONG;						\
			PHP_RAND_RANGE(which, Z_LVAL_PP(min), 					\
					Z_LVAL_PP(max), Z_LVAL_P(return_value));		\
			return;													\
		default:													\
			WRONG_PARAM_COUNT;										\
			break;													\
	}																\
}
/* }}} */

/* {{{ proto int rand([int min, int max]) 
   Returns a random number */
PHP_FUNCTION_RAND(rand,PHP_RAND_SYS)
/* }}} */

/* {{{ proto int mt_rand([int min, int max]) 
   Returns a random number by means of Mersenne Twister */
PHP_FUNCTION_RAND(mt_rand,PHP_RAND_MT)
/* }}} */

/* getrandmax */

/* {{{ PHPAPI long php_randmax(void)
   Returns the maximum value a random number can have */
PHPAPI long php_randmax(void)
{
	return PHP_RANDMAX(BG(rand_generator_current));
}
/* }}} */

/* {{{ proto int getrandmax(void)
   Returns the maximum value a random number can have */
PHP_FUNCTION(getrandmax)
{
	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	RETURN_LONG( PHP_RANDMAX(PHP_RAND_SYS));
}
/* }}} */

/* {{{ proto int mt_getrandmax(void)
   Returns the maximum value a random number can have */
PHP_FUNCTION(mt_getrandmax)
{
	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	RETURN_LONG( PHP_RANDMAX(PHP_RAND_MT));
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 tw=78 fdm=marker
 * vim: sw=4 ts=4 tw=78
 */
