/* Generated by re2c 0.13.5 on Mon May 25 10:34:13 2009 */
#line 1 "ext/standard/var_unserializer.re"
/*
  +----------------------------------------------------------------------+
  | PHP Version 6                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2009 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Sascha Schumann <sascha@schumann.cx>                         |
  +----------------------------------------------------------------------+
*/

/* $Id: var_unserializer.c,v 1.102 2009/05/25 14:32:15 felipe Exp $ */

#include "php.h"
#include "ext/standard/php_var.h"
#include "php_incomplete_class.h"

/* {{{ reference-handling for unserializer: var_* */
#define VAR_ENTRIES_MAX 1024

typedef struct {
	zval *data[VAR_ENTRIES_MAX];
	long used_slots;
	void *next;
} var_entries;

static inline void var_push(php_unserialize_data_t *var_hashx, zval **rval)
{
	var_entries *var_hash = var_hashx->first, *prev = NULL;

	while (var_hash && var_hash->used_slots == VAR_ENTRIES_MAX) {
		prev = var_hash;
		var_hash = var_hash->next;
	}

	if (!var_hash) {
		var_hash = emalloc(sizeof(var_entries));
		var_hash->used_slots = 0;
		var_hash->next = 0;

		if (!var_hashx->first)
			var_hashx->first = var_hash;
		else
			prev->next = var_hash;
	}

	var_hash->data[var_hash->used_slots++] = *rval;
}

static inline void var_push_dtor(php_unserialize_data_t *var_hashx, zval **rval)
{
	var_entries *var_hash = var_hashx->first_dtor, *prev = NULL;

	while (var_hash && var_hash->used_slots == VAR_ENTRIES_MAX) {
		prev = var_hash;
		var_hash = var_hash->next;
	}

	if (!var_hash) {
		var_hash = emalloc(sizeof(var_entries));
		var_hash->used_slots = 0;
		var_hash->next = 0;

		if (!var_hashx->first_dtor)
			var_hashx->first_dtor = var_hash;
		else
			prev->next = var_hash;
	}

	Z_ADDREF_PP(rval);
	var_hash->data[var_hash->used_slots++] = *rval;
}

static UChar *unserialize_ustr(const unsigned char **p, int len)
{
	int i, j;
	UChar *ustr = eumalloc(len+1);

	for (i = 0; i < len; i++) {
		if (**p != '\\') {
			ustr[i] = (UChar)**p;
		} else {
			UChar ch = 0;

			for (j = 0; j < 4; j++) {
				(*p)++;
				if (**p >= '0' && **p <= '9') {
					ch = (ch << 4) + (**p -'0');
				} else if (**p >= 'a' && **p <= 'f') {
					ch = (ch << 4) + (**p -'a'+10);
				} else if (**p >= 'A' && **p <= 'F') {
					ch = (ch << 4) + (**p -'A'+10);
				} else {
					efree(ustr);
					return NULL;
				}
			}
			ustr[i] = ch;
		}
		(*p)++;
	}
	ustr[i] = 0;
	return ustr;
}

static char *unserialize_str(const unsigned char **p, size_t *len, size_t maxlen)
{
	size_t i, j;
	char *str = safe_emalloc(*len, 1, 1);
	unsigned char *end = *(unsigned char **)p+maxlen;

	if (end < *p) {
		efree(str);
		return NULL;
	}

	for (i = 0; i < *len; i++) {
		if (*p >= end) {
			efree(str);
			return NULL;
		}
		if (**p != '\\') {
			str[i] = (char)**p;
		} else {
			unsigned char ch = 0;

			for (j = 0; j < 2; j++) {
				(*p)++;
				if (**p >= '0' && **p <= '9') {
					ch = (ch << 4) + (**p -'0');
				} else if (**p >= 'a' && **p <= 'f') {
					ch = (ch << 4) + (**p -'a'+10);
				} else if (**p >= 'A' && **p <= 'F') {
					ch = (ch << 4) + (**p -'A'+10);
				} else {
					efree(str);
					return NULL;
				}
			}
			str[i] = (char)ch;
		}
		(*p)++;
	}
	str[i] = 0;
	*len = i;
	return str;
}

PHPAPI void var_replace(php_unserialize_data_t *var_hashx, zval *ozval, zval **nzval)
{
	long i;
	var_entries *var_hash = var_hashx->first;

	while (var_hash) {
		for (i = 0; i < var_hash->used_slots; i++) {
			if (var_hash->data[i] == ozval) {
				var_hash->data[i] = *nzval;
				/* do not break here */
			}
		}
		var_hash = var_hash->next;
	}
}

static int var_access(php_unserialize_data_t *var_hashx, long id, zval ***store)
{
	var_entries *var_hash = var_hashx->first;

	while (id >= VAR_ENTRIES_MAX && var_hash && var_hash->used_slots == VAR_ENTRIES_MAX) {
		var_hash = var_hash->next;
		id -= VAR_ENTRIES_MAX;
	}

	if (!var_hash) return !SUCCESS;

	if (id < 0 || id >= var_hash->used_slots) return !SUCCESS;

	*store = &var_hash->data[id];

	return SUCCESS;
}

PHPAPI void var_destroy(php_unserialize_data_t *var_hashx)
{
	void *next;
	long i;
	var_entries *var_hash = var_hashx->first;

	while (var_hash) {
		next = var_hash->next;
		efree(var_hash);
		var_hash = next;
	}

	var_hash = var_hashx->first_dtor;

	while (var_hash) {
		for (i = 0; i < var_hash->used_slots; i++) {
			zval_ptr_dtor(&var_hash->data[i]);
		}
		next = var_hash->next;
		efree(var_hash);
		var_hash = next;
	}
}

/* }}} */

#define YYFILL(n) do { } while (0)
#define YYCTYPE unsigned char
#define YYCURSOR cursor
#define YYLIMIT limit
#define YYMARKER marker


#line 230 "ext/standard/var_unserializer.re"




static inline long parse_iv2(const unsigned char *p, const unsigned char **q)
{
	char cursor;
	long result = 0;
	int neg = 0;

	switch (*p) {
		case '-':
			neg++;
			/* fall-through */
		case '+':
			p++;
	}

	while (1) {
		cursor = (char)*p;
		if (cursor >= '0' && cursor <= '9') {
			result = result * 10 + cursor - '0';
		} else {
			break;
		}
		p++;
	}
	if (q) *q = p;
	if (neg) return -result;
	return result;
}

static inline long parse_iv(const unsigned char *p)
{
	return parse_iv2(p, NULL);
}

/* no need to check for length - re2c already did */
static inline size_t parse_uiv(const unsigned char *p)
{
	unsigned char cursor;
	size_t result = 0;

	if (*p == '+') {
		p++;
	}

	while (1) {
		cursor = *p;
		if (cursor >= '0' && cursor <= '9') {
			result = result * 10 + (size_t)(cursor - (unsigned char)'0');
		} else {
			break;
		}
		p++;
	}
	return result;
}

#define UNSERIALIZE_PARAMETER zval **rval, const unsigned char **p, const unsigned char *max, php_unserialize_data_t *var_hash TSRMLS_DC
#define UNSERIALIZE_PASSTHRU rval, p, max, var_hash TSRMLS_CC

static inline int process_nested_data(UNSERIALIZE_PARAMETER, HashTable *ht, long elements)
{
	while (elements-- > 0) {
		zval *key, *data, **old_data;

		ALLOC_INIT_ZVAL(key);

		if (!php_var_unserialize(&key, p, max, NULL TSRMLS_CC)) {
			zval_dtor(key);
			FREE_ZVAL(key);
			return 0;
		}

		if (Z_TYPE_P(key) != IS_LONG &&
			Z_TYPE_P(key) != IS_STRING &&
			Z_TYPE_P(key) != IS_UNICODE
		) {
			zval_dtor(key);
			FREE_ZVAL(key);
			return 0;
		}

		ALLOC_INIT_ZVAL(data);

		if (!php_var_unserialize(&data, p, max, var_hash TSRMLS_CC)) {
			zval_dtor(key);
			FREE_ZVAL(key);
			zval_dtor(data);
			FREE_ZVAL(data);
			return 0;
		}

		switch (Z_TYPE_P(key)) {
			case IS_LONG:
				if (zend_hash_index_find(ht, Z_LVAL_P(key), (void **)&old_data)==SUCCESS) {
					var_push_dtor(var_hash, old_data);
				}
				zend_hash_index_update(ht, Z_LVAL_P(key), &data, sizeof(data), NULL);
				break;
			case IS_STRING:
			case IS_UNICODE:
				if (zend_u_symtable_find(ht, Z_TYPE_P(key), Z_UNIVAL_P(key), Z_UNILEN_P(key) + 1, (void **)&old_data)==SUCCESS) {
					var_push_dtor(var_hash, old_data);
				}
				zend_u_symtable_update(ht, Z_TYPE_P(key), Z_UNIVAL_P(key), Z_UNILEN_P(key) + 1, &data, sizeof(data), NULL);
				break;
		}

		zval_dtor(key);
		FREE_ZVAL(key);

		if (elements && *(*p-1) != ';' && *(*p-1) != '}') {
			(*p)--;
			return 0;
		}
	}

	return 1;
}

static inline int finish_nested_data(UNSERIALIZE_PARAMETER)
{
	if (*((*p)++) == '}')
		return 1;

#if SOMETHING_NEW_MIGHT_LEAD_TO_CRASH_ENABLE_IF_YOU_ARE_BRAVE
	zval_ptr_dtor(rval);
#endif
	return 0;
}

static inline int object_custom(UNSERIALIZE_PARAMETER, zend_class_entry *ce)
{
	long datalen;
	int type;
	zstr buf;
	size_t buf_len;

	datalen = parse_iv2((*p) + 2, p);

	switch((*p)[1]) {
	case 'U':
		type = IS_UNICODE;
		(*p) += 4;
		break;
	case 'N':
		(*p) += 2;
		return finish_nested_data(UNSERIALIZE_PASSTHRU);
	case '{':
		type = IS_STRING;
		(*p) += 2;
		break;
	default:
		zend_error(E_WARNING, "Illegal data for unserializing");
		return 0;
	}

	if (datalen < 0 || (*p) + datalen >= max) {
		zend_error(E_WARNING, "Insufficient data for unserializing - %ld required, %ld present", datalen, (long)(max - (*p)));
		return 0;
	}

	if (type == IS_UNICODE) {
		buf.u = unserialize_ustr(p, datalen);
		buf_len = u_strlen(buf.u);
	} else {
		buf.s = (char*)*p;
		buf_len = datalen;
		(*p) += datalen;
	}
	if (ce->unserialize == NULL) {
		zend_error(E_WARNING, "Class %v has no unserializer", ce->name);
		object_init_ex(*rval, ce);
	} else if (ce->unserialize(rval, ce, type, buf, buf_len, (zend_unserialize_data *)var_hash TSRMLS_CC) != SUCCESS) {
		if (type == IS_UNICODE) {
			efree(buf.v);
		}
		return 0;
	}
	if (type == IS_UNICODE) {
		efree(buf.v);
	}

	return finish_nested_data(UNSERIALIZE_PASSTHRU);
}

static inline long object_common1(UNSERIALIZE_PARAMETER, zend_class_entry *ce)
{
	long elements;

	elements = parse_iv2((*p) + 2, p);

	(*p) += 2;

	object_init_ex(*rval, ce);
	return elements;
}

static inline int object_common2(UNSERIALIZE_PARAMETER, long elements)
{
	zval *retval_ptr = NULL;
	zval fname;

	if (!process_nested_data(UNSERIALIZE_PASSTHRU, Z_OBJPROP_PP(rval), elements)) {
		return 0;
	}

	if (Z_OBJCE_PP(rval) != PHP_IC_ENTRY &&
		zend_hash_exists(&Z_OBJCE_PP(rval)->function_table, "__wakeup", sizeof("__wakeup"))) {
		INIT_PZVAL(&fname);
		ZVAL_ASCII_STRINGL(&fname, "__wakeup", sizeof("__wakeup") - 1, 1);
		call_user_function_ex(CG(function_table), rval, &fname, &retval_ptr, 0, 0, 1, NULL TSRMLS_CC);
		zval_dtor(&fname);
	}

	if (retval_ptr)
		zval_ptr_dtor(&retval_ptr);

	return finish_nested_data(UNSERIALIZE_PASSTHRU);

}

PHPAPI int php_var_unserialize(UNSERIALIZE_PARAMETER)
{
	const unsigned char *cursor, *limit, *marker, *start;
	zval **rval_ref;

	limit = cursor = *p;

	if (var_hash && cursor[0] != 'R') {
		var_push(var_hash, rval);
	}

	start = cursor;


#line 465 "ext/standard/var_unserializer.c"
{
	YYCTYPE yych;
	static const unsigned char yybm[] = {
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
	};

	if ((YYLIMIT - YYCURSOR) < 7) YYFILL(7);
	yych = *YYCURSOR;
	switch (yych) {
	case 'C':
	case 'O':	goto yy14;
	case 'N':	goto yy5;
	case 'R':	goto yy2;
	case 'S':	goto yy10;
	case 'U':	goto yy11;
	case 'a':	goto yy12;
	case 'b':	goto yy6;
	case 'd':	goto yy8;
	case 'i':	goto yy7;
	case 'o':	goto yy13;
	case 'r':	goto yy4;
	case 's':	goto yy9;
	case '}':	goto yy15;
	default:	goto yy17;
	}
yy2:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy103;
yy3:
#line 807 "ext/standard/var_unserializer.re"
	{ return 0; }
#line 528 "ext/standard/var_unserializer.c"
yy4:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy97;
	goto yy3;
yy5:
	yych = *++YYCURSOR;
	if (yych == ';') goto yy95;
	goto yy3;
yy6:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy91;
	goto yy3;
yy7:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy85;
	goto yy3;
yy8:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy61;
	goto yy3;
yy9:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy54;
	goto yy3;
yy10:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy47;
	goto yy3;
yy11:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy40;
	goto yy3;
yy12:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy33;
	goto yy3;
yy13:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy26;
	goto yy3;
yy14:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy18;
	goto yy3;
yy15:
	++YYCURSOR;
#line 801 "ext/standard/var_unserializer.re"
	{
	/* this is the case where we have less data than planned */
	php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Unexpected end of serialized data");
	return 0; /* not sure if it should be 0 or 1 here? */
}
#line 581 "ext/standard/var_unserializer.c"
yy17:
	yych = *++YYCURSOR;
	goto yy3;
yy18:
	yych = *++YYCURSOR;
	if (yybm[0+yych] & 128) {
		goto yy21;
	}
	if (yych == '+') goto yy20;
yy19:
	YYCURSOR = YYMARKER;
	goto yy3;
yy20:
	yych = *++YYCURSOR;
	if (yybm[0+yych] & 128) {
		goto yy21;
	}
	goto yy19;
yy21:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yybm[0+yych] & 128) {
		goto yy21;
	}
	if (yych != ':') goto yy19;
	yych = *++YYCURSOR;
	if (yych != '"') goto yy19;
	++YYCURSOR;
#line 693 "ext/standard/var_unserializer.re"
	{
	size_t len, len2, maxlen;
	long elements;
	zstr class_name;
	zend_class_entry *ce;
	zend_class_entry **pce;
	int incomplete_class = 0;

	int custom_object = 0;

	zval *user_func;
	zval *retval_ptr;
	zval **args[1];
	zval *arg_func_name;

	if (*start == 'C') {
		custom_object = 1;
	}

	INIT_PZVAL(*rval);
	len2 = len = parse_uiv(start + 2);
	maxlen = max - YYCURSOR;
	if (maxlen < len || len == 0) {
		*p = start + 2;
		return 0;
	}

	class_name.u = unserialize_ustr(&YYCURSOR, len);

	if (*(YYCURSOR) != '"') {
		efree(class_name.v);
		*p = YYCURSOR;
		return 0;
	}
	if (*(YYCURSOR+1) != ':') {
		efree(class_name.v);
		*p = YYCURSOR+1;
		return 0;
	}

	do {
		/* Try to find class directly */
		if (zend_u_lookup_class(IS_UNICODE, class_name, len2, &pce TSRMLS_CC) == SUCCESS) {
			ce = *pce;
			break;
		}

		/* Check for unserialize callback */
		if ((PG(unserialize_callback_func) == NULL) || (PG(unserialize_callback_func)[0] == '\0')) {
			incomplete_class = 1;
			ce = PHP_IC_ENTRY;
			break;
		}

		/* Call unserialize callback */
		MAKE_STD_ZVAL(user_func);
		ZVAL_STRING(user_func, PG(unserialize_callback_func), 1);
		args[0] = &arg_func_name;
		MAKE_STD_ZVAL(arg_func_name);
		ZVAL_UNICODE(arg_func_name, class_name.u, 1);
		if (call_user_function_ex(CG(function_table), NULL, user_func, &retval_ptr, 1, args, 0, NULL TSRMLS_CC) != SUCCESS) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "defined (%s) but not found", user_func->value.str.val);
			incomplete_class = 1;
			ce = PHP_IC_ENTRY;
			zval_ptr_dtor(&user_func);
			zval_ptr_dtor(&arg_func_name);
			break;
		}
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}

		/* The callback function may have defined the class */
		if (zend_u_lookup_class(IS_UNICODE, class_name, len2, &pce TSRMLS_CC) == SUCCESS) {
			ce = *pce;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Function %s() hasn't defined the class it was called for", user_func->value.str.val);
			incomplete_class = 1;
			ce = PHP_IC_ENTRY;
		}

		zval_ptr_dtor(&user_func);
		zval_ptr_dtor(&arg_func_name);
		break;
	} while (1);

	*p = YYCURSOR;

	if (custom_object) {
		int ret = object_custom(UNSERIALIZE_PASSTHRU, ce);

		if (ret && incomplete_class) {
			php_store_class_name(*rval, class_name, len2);
		}
		efree(class_name.v);
		return ret;
	}

	elements = object_common1(UNSERIALIZE_PASSTHRU, ce);

	if (incomplete_class) {
		php_store_class_name(*rval, class_name, len2);
	}
	efree(class_name.v);

	return object_common2(UNSERIALIZE_PASSTHRU, elements);
}
#line 719 "ext/standard/var_unserializer.c"
yy26:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy19;
	} else {
		if (yych <= '-') goto yy27;
		if (yych <= '/') goto yy19;
		if (yych <= '9') goto yy28;
		goto yy19;
	}
yy27:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych >= ':') goto yy19;
yy28:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy28;
	if (yych >= ';') goto yy19;
	yych = *++YYCURSOR;
	if (yych != '"') goto yy19;
	++YYCURSOR;
#line 685 "ext/standard/var_unserializer.re"
	{

	INIT_PZVAL(*rval);

	return object_common2(UNSERIALIZE_PASSTHRU,
			object_common1(UNSERIALIZE_PASSTHRU, ZEND_STANDARD_CLASS_DEF_PTR));
}
#line 752 "ext/standard/var_unserializer.c"
yy33:
	yych = *++YYCURSOR;
	if (yych == '+') goto yy34;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy35;
	goto yy19;
yy34:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych >= ':') goto yy19;
yy35:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy35;
	if (yych >= ';') goto yy19;
	yych = *++YYCURSOR;
	if (yych != '{') goto yy19;
	++YYCURSOR;
#line 665 "ext/standard/var_unserializer.re"
	{
	long elements = parse_iv(start + 2);
	/* use iv() not uiv() in order to check data range */
	*p = YYCURSOR;

	if (elements < 0) {
		return 0;
	}

	INIT_PZVAL(*rval);

	array_init_size(*rval, elements);

	if (!process_nested_data(UNSERIALIZE_PASSTHRU, Z_ARRVAL_PP(rval), elements)) {
		return 0;
	}

	return finish_nested_data(UNSERIALIZE_PASSTHRU);
}
#line 793 "ext/standard/var_unserializer.c"
yy40:
	yych = *++YYCURSOR;
	if (yych == '+') goto yy41;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy42;
	goto yy19;
yy41:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych >= ':') goto yy19;
yy42:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy42;
	if (yych >= ';') goto yy19;
	yych = *++YYCURSOR;
	if (yych != '"') goto yy19;
	++YYCURSOR;
#line 636 "ext/standard/var_unserializer.re"
	{
	size_t len, maxlen;
	UChar *ustr;

	len = parse_uiv(start + 2);
	maxlen = max - YYCURSOR;
	if (maxlen < len) {
		*p = start + 2;
		return 0;
	}

	if ((ustr = unserialize_ustr(&YYCURSOR, len)) == NULL) {
		return 0;
	}

	if (*(YYCURSOR) != '"') {
		efree(ustr);
		*p = YYCURSOR;
		return 0;
	}

	YYCURSOR += 2;
	*p = YYCURSOR;

	INIT_PZVAL(*rval);
	ZVAL_UNICODEL(*rval, ustr, len, 0);
	return 1;
}
#line 843 "ext/standard/var_unserializer.c"
yy47:
	yych = *++YYCURSOR;
	if (yych == '+') goto yy48;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy49;
	goto yy19;
yy48:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych >= ':') goto yy19;
yy49:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy49;
	if (yych >= ';') goto yy19;
	yych = *++YYCURSOR;
	if (yych != '"') goto yy19;
	++YYCURSOR;
#line 607 "ext/standard/var_unserializer.re"
	{
	size_t len, maxlen;
	char *str;

	len = parse_uiv(start + 2);
	maxlen = max - YYCURSOR;
	if (maxlen < len) {
		*p = start + 2;
		return 0;
	}

	if ((str = unserialize_str(&YYCURSOR, &len, maxlen)) == NULL) {
		return 0;
	}

	if (*(YYCURSOR) != '"') {
		efree(str);
		*p = YYCURSOR;
		return 0;
	}

	YYCURSOR += 2;
	*p = YYCURSOR;

	INIT_PZVAL(*rval);
	ZVAL_STRINGL(*rval, str, len, 0);
	return 1;
}
#line 893 "ext/standard/var_unserializer.c"
yy54:
	yych = *++YYCURSOR;
	if (yych == '+') goto yy55;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy56;
	goto yy19;
yy55:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych >= ':') goto yy19;
yy56:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy56;
	if (yych >= ';') goto yy19;
	yych = *++YYCURSOR;
	if (yych != '"') goto yy19;
	++YYCURSOR;
#line 579 "ext/standard/var_unserializer.re"
	{
	size_t len, maxlen;
	char *str;

	len = parse_uiv(start + 2);
	maxlen = max - YYCURSOR;
	if (maxlen < len) {
		*p = start + 2;
		return 0;
	}

	str = (char*)YYCURSOR;

	YYCURSOR += len;

	if (*(YYCURSOR) != '"') {
		*p = YYCURSOR;
		return 0;
	}

	YYCURSOR += 2;
	*p = YYCURSOR;

	INIT_PZVAL(*rval);
	ZVAL_STRINGL(*rval, str, len, 1);
	return 1;
}
#line 942 "ext/standard/var_unserializer.c"
yy61:
	yych = *++YYCURSOR;
	if (yych <= '/') {
		if (yych <= ',') {
			if (yych == '+') goto yy65;
			goto yy19;
		} else {
			if (yych <= '-') goto yy63;
			if (yych <= '.') goto yy68;
			goto yy19;
		}
	} else {
		if (yych <= 'I') {
			if (yych <= '9') goto yy66;
			if (yych <= 'H') goto yy19;
			goto yy64;
		} else {
			if (yych != 'N') goto yy19;
		}
	}
	yych = *++YYCURSOR;
	if (yych == 'A') goto yy84;
	goto yy19;
yy63:
	yych = *++YYCURSOR;
	if (yych <= '/') {
		if (yych == '.') goto yy68;
		goto yy19;
	} else {
		if (yych <= '9') goto yy66;
		if (yych != 'I') goto yy19;
	}
yy64:
	yych = *++YYCURSOR;
	if (yych == 'N') goto yy80;
	goto yy19;
yy65:
	yych = *++YYCURSOR;
	if (yych == '.') goto yy68;
	if (yych <= '/') goto yy19;
	if (yych >= ':') goto yy19;
yy66:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *YYCURSOR;
	if (yych <= ':') {
		if (yych <= '.') {
			if (yych <= '-') goto yy19;
			goto yy78;
		} else {
			if (yych <= '/') goto yy19;
			if (yych <= '9') goto yy66;
			goto yy19;
		}
	} else {
		if (yych <= 'E') {
			if (yych <= ';') goto yy71;
			if (yych <= 'D') goto yy19;
			goto yy73;
		} else {
			if (yych == 'e') goto yy73;
			goto yy19;
		}
	}
yy68:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych >= ':') goto yy19;
yy69:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *YYCURSOR;
	if (yych <= ';') {
		if (yych <= '/') goto yy19;
		if (yych <= '9') goto yy69;
		if (yych <= ':') goto yy19;
	} else {
		if (yych <= 'E') {
			if (yych <= 'D') goto yy19;
			goto yy73;
		} else {
			if (yych == 'e') goto yy73;
			goto yy19;
		}
	}
yy71:
	++YYCURSOR;
#line 569 "ext/standard/var_unserializer.re"
	{
#if SIZEOF_LONG == 4
use_double:
#endif
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_DOUBLE(*rval, zend_strtod((const char *)start + 2, NULL));
	return 1;
}
#line 1040 "ext/standard/var_unserializer.c"
yy73:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy19;
	} else {
		if (yych <= '-') goto yy74;
		if (yych <= '/') goto yy19;
		if (yych <= '9') goto yy75;
		goto yy19;
	}
yy74:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych == '+') goto yy77;
		goto yy19;
	} else {
		if (yych <= '-') goto yy77;
		if (yych <= '/') goto yy19;
		if (yych >= ':') goto yy19;
	}
yy75:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy75;
	if (yych == ';') goto yy71;
	goto yy19;
yy77:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy75;
	goto yy19;
yy78:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *YYCURSOR;
	if (yych <= ';') {
		if (yych <= '/') goto yy19;
		if (yych <= '9') goto yy78;
		if (yych <= ':') goto yy19;
		goto yy71;
	} else {
		if (yych <= 'E') {
			if (yych <= 'D') goto yy19;
			goto yy73;
		} else {
			if (yych == 'e') goto yy73;
			goto yy19;
		}
	}
yy80:
	yych = *++YYCURSOR;
	if (yych != 'F') goto yy19;
yy81:
	yych = *++YYCURSOR;
	if (yych != ';') goto yy19;
	++YYCURSOR;
#line 554 "ext/standard/var_unserializer.re"
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);

	if (!strncmp((char*)start + 2, "NAN", 3)) {
		ZVAL_DOUBLE(*rval, php_get_nan());
	} else if (!strncmp((char*)start + 2, "INF", 3)) {
		ZVAL_DOUBLE(*rval, php_get_inf());
	} else if (!strncmp((char*)start + 2, "-INF", 4)) {
		ZVAL_DOUBLE(*rval, -php_get_inf());
	}

	return 1;
}
#line 1114 "ext/standard/var_unserializer.c"
yy84:
	yych = *++YYCURSOR;
	if (yych == 'N') goto yy81;
	goto yy19;
yy85:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy19;
	} else {
		if (yych <= '-') goto yy86;
		if (yych <= '/') goto yy19;
		if (yych <= '9') goto yy87;
		goto yy19;
	}
yy86:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych >= ':') goto yy19;
yy87:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy87;
	if (yych != ';') goto yy19;
	++YYCURSOR;
#line 527 "ext/standard/var_unserializer.re"
	{
#if SIZEOF_LONG == 4
	int digits = YYCURSOR - start - 3;

	if (start[2] == '-' || start[2] == '+') {
		digits--;
	}

	/* Use double for large long values that were serialized on a 64-bit system */
	if (digits >= MAX_LENGTH_OF_LONG - 1) {
		if (digits == MAX_LENGTH_OF_LONG - 1) {
			int cmp = strncmp(YYCURSOR - MAX_LENGTH_OF_LONG, long_min_digits, MAX_LENGTH_OF_LONG - 1);

			if (!(cmp < 0 || (cmp == 0 && start[2] == '-'))) {
				goto use_double;
			}
		} else {
			goto use_double;
		}
	}
#endif
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_LONG(*rval, parse_iv(start + 2));
	return 1;
}
#line 1168 "ext/standard/var_unserializer.c"
yy91:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych >= '2') goto yy19;
	yych = *++YYCURSOR;
	if (yych != ';') goto yy19;
	++YYCURSOR;
#line 520 "ext/standard/var_unserializer.re"
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_BOOL(*rval, parse_iv(start + 2));
	return 1;
}
#line 1183 "ext/standard/var_unserializer.c"
yy95:
	++YYCURSOR;
#line 513 "ext/standard/var_unserializer.re"
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_NULL(*rval);
	return 1;
}
#line 1193 "ext/standard/var_unserializer.c"
yy97:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy19;
	} else {
		if (yych <= '-') goto yy98;
		if (yych <= '/') goto yy19;
		if (yych <= '9') goto yy99;
		goto yy19;
	}
yy98:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych >= ':') goto yy19;
yy99:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy99;
	if (yych != ';') goto yy19;
	++YYCURSOR;
#line 490 "ext/standard/var_unserializer.re"
	{
	long id;

 	*p = YYCURSOR;
	if (!var_hash) return 0;

	id = parse_iv(start + 2) - 1;
	if (id == -1 || var_access(var_hash, id, &rval_ref) != SUCCESS) {
		return 0;
	}

	if (*rval == *rval_ref) return 0;

	if (*rval != NULL) {
		zval_ptr_dtor(rval);
	}
	*rval = *rval_ref;
	Z_ADDREF_PP(rval);
	Z_UNSET_ISREF_PP(rval);

	return 1;
}
#line 1239 "ext/standard/var_unserializer.c"
yy103:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy19;
	} else {
		if (yych <= '-') goto yy104;
		if (yych <= '/') goto yy19;
		if (yych <= '9') goto yy105;
		goto yy19;
	}
yy104:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych >= ':') goto yy19;
yy105:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') goto yy19;
	if (yych <= '9') goto yy105;
	if (yych != ';') goto yy19;
	++YYCURSOR;
#line 469 "ext/standard/var_unserializer.re"
	{
	long id;

 	*p = YYCURSOR;
	if (!var_hash) return 0;

	id = parse_iv(start + 2) - 1;
	if (id == -1 || var_access(var_hash, id, &rval_ref) != SUCCESS) {
		return 0;
	}

	if (*rval != NULL) {
		zval_ptr_dtor(rval);
	}
	*rval = *rval_ref;
	Z_ADDREF_PP(rval);
	Z_SET_ISREF_PP(rval);

	return 1;
}
#line 1283 "ext/standard/var_unserializer.c"
}
#line 809 "ext/standard/var_unserializer.re"


	return 0;
}
