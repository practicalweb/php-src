/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2002 The PHP Group                                |
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
   |          Stig S�ther Bakken <ssb@fast.no>                            |
   |          David Sklar <sklar@student.net>                             |
   +----------------------------------------------------------------------+
 */
/* $Id: php_apache.c,v 1.45.4.2 2002/08/26 20:06:31 gschlossnagle Exp $ */

#include "php_apache_http.h"

#if defined(PHP_WIN32) || defined(NETWARE)
#include "zend.h"
#include "ap_compat.h"
#else
#include "build-defs.h"
#endif

#ifdef ZTS
int php_apache_info_id;
#else
php_apache_info_struct php_apache_info;
#endif

#define SECTION(name)  PUTS("<H2 align=\"center\">" name "</H2>\n")

extern module *top_module;
static int le_apachereq;
static zend_class_entry *apacherequest_class_entry;

PHP_FUNCTION(virtual);
PHP_FUNCTION(apache_request_headers);
PHP_FUNCTION(apache_response_headers);
PHP_FUNCTION(apachelog);
PHP_FUNCTION(apache_note);
PHP_FUNCTION(apache_lookup_uri);
PHP_FUNCTION(apache_child_terminate);
PHP_FUNCTION(apache_setenv);

PHP_MINFO_FUNCTION(apache);


function_entry apache_functions[] = {
	PHP_FE(virtual,									NULL)
	PHP_FE(apache_request_headers,					NULL)
	PHP_FE(apache_note,								NULL)
	PHP_FE(apache_lookup_uri,						NULL)
	PHP_FE(apache_child_terminate,					NULL)
	PHP_FE(apache_setenv,							NULL)
	PHP_FE(apache_response_headers,					NULL)
	PHP_FALIAS(getallheaders, apache_request_headers, NULL)
	{NULL, NULL, NULL}
};

/* {{{ php_apache ini entries
 */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("xbithack",			"0",				PHP_INI_ALL,		OnUpdateInt,		xbithack, php_apache_info_struct, php_apache_info)
	STD_PHP_INI_ENTRY("engine",				"1",				PHP_INI_ALL,		OnUpdateInt,		engine, php_apache_info_struct, php_apache_info)
	STD_PHP_INI_ENTRY("last_modified",		"0",				PHP_INI_ALL,		OnUpdateInt,		last_modified, php_apache_info_struct, php_apache_info)
	STD_PHP_INI_ENTRY("child_terminate",	"0",				PHP_INI_ALL,		OnUpdateInt,		terminate_child, php_apache_info_struct, php_apache_info)
	STD_PHP_INI_ENTRY("uri_handler",		NULL,				PHP_INI_ALL,		OnUpdateString,		uri_handler, php_apache_info_struct, php_apache_info)
	STD_PHP_INI_ENTRY("auth_handler",		NULL,				PHP_INI_ALL,		OnUpdateString,		auth_handler, php_apache_info_struct, php_apache_info)
	STD_PHP_INI_ENTRY("access_handler",		NULL,				PHP_INI_ALL,		OnUpdateString,		access_handler, php_apache_info_struct, php_apache_info)
	STD_PHP_INI_ENTRY("type_handler",		NULL,				PHP_INI_ALL,		OnUpdateString,		type_handler, php_apache_info_struct, php_apache_info)
	STD_PHP_INI_ENTRY("fixup_handler",		NULL,				PHP_INI_ALL,		OnUpdateString,		fixup_handler, php_apache_info_struct, php_apache_info)
	STD_PHP_INI_ENTRY("logger_handler",		NULL,				PHP_INI_ALL,		OnUpdateString,		logger_handler, php_apache_info_struct, php_apache_info)

PHP_INI_END()
/* }}} */

static void php_apache_globals_ctor(php_apache_info_struct *apache_globals TSRMLS_DC)
{
	apache_globals->in_request = 0;
}


#define APREQ_GET_THIS(ZVAL)		if (NULL == (ZVAL = getThis())) { \
										php_error(E_WARNING, "%s(): underlying ApacheRequest object missing", \
											get_active_function_name(TSRMLS_C)); \
										RETURN_FALSE; \
									}
#define APREQ_GET_REQUEST(ZVAL, R)	APREQ_GET_THIS(ZVAL); \
									R = get_apache_request(ZVAL)

static void php_apache_request_free(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	zval *z = (zval *)rsrc->ptr;
	fprintf(stderr, "%s() %p\n", __FUNCTION__, z);
	zval_ptr_dtor(&z);
}

static request_rec *get_apache_request(zval *z)
{
	request_rec *r;
	zval **addr;

	if (NULL == z) {
		php_error(E_WARNING, "get_apache_request() invalid wrapper passed");
		return NULL;
	}

	if (Z_TYPE_P(z) != IS_OBJECT) {
		php_error(E_WARNING, "%s(): wrapper is not an object", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	if (zend_hash_index_find(Z_OBJPROP_P(z), 0, (void **)&addr) == FAILURE) {
		php_error(E_WARNING, "%s(): underlying object missing", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	r = (request_rec *)Z_LVAL_PP(addr);
	if (!r) {
		php_error(E_WARNING, "%s(): request_rec invalid", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	return r;
}

/* {{{
 */
PHPAPI zval *php_apache_request_new(request_rec *r)
{
	zval *req;
	zval *addr;

	MAKE_STD_ZVAL(addr);
	Z_TYPE_P(addr) = IS_LONG;
	Z_LVAL_P(addr) = (int) r;

	MAKE_STD_ZVAL(req);
	object_init_ex(req, apacherequest_class_entry);
	zend_hash_index_update(Z_OBJPROP_P(req), 0, &addr, sizeof(zval *), NULL);

	return req;
}
/* }}} */

/* {{{ apache_request_read_string_slot()
 */
static void apache_request_read_string_slot(int offset, INTERNAL_FUNCTION_PARAMETERS)
{
	zval *id, **new_value;
	request_rec *r;
	char *s;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	s = *(char **)((char*)r + offset);

	if (s)
		RETURN_STRING(s, 1);

	RETURN_EMPTY_STRING();
}
/* }}} */


/* {{{ apache_request_string_slot()
 */
static void apache_request_string_slot(int offset, INTERNAL_FUNCTION_PARAMETERS)
{
	zval *id, **new_value;
	request_rec *r;
	char *old_value;
	char **target;

	APREQ_GET_REQUEST(id, r);

	target = (char **)((char*)r + offset);
	old_value = *target;

	switch (ZEND_NUM_ARGS()) {
		case 0:
			break;
		case 1:
			if (zend_get_parameters_ex(1, &new_value) == FAILURE) {
				RETURN_FALSE;
			}
			convert_to_string_ex(new_value);
			*target = ap_pstrdup(r->pool, Z_STRVAL_PP(new_value));
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}

	if (old_value)
		RETURN_STRING(old_value, 1);

	RETURN_EMPTY_STRING();
}
/* }}} */

/* {{{ apache_request_read_int_slot()
 */
static void apache_request_read_int_slot(int offset, INTERNAL_FUNCTION_PARAMETERS)
{
	zval *id;
	request_rec *r;
	long l;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	l = *(long *)((char*)r + offset);

	RETURN_LONG(l);
}
/* }}} */

/* {{{ apache_request_int_slot()
 */
static void apache_request_int_slot(int offset, INTERNAL_FUNCTION_PARAMETERS)
{
	zval *id, **new_value;
	request_rec *r;
	long old_value;
	long *target;

	APREQ_GET_REQUEST(id, r);

	target = (long *)((char*)r + offset);
	old_value = *target;

	switch (ZEND_NUM_ARGS()) {
		case 0:
			break;
		case 1:
			if (zend_get_parameters_ex(1, &new_value) == FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long_ex(new_value);
			*target = Z_LVAL_PP(new_value);
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}

	RETURN_LONG(old_value);
}
/* }}} */


/* {{{ access string slots of request rec
 */

/* {{{ proto string $request->filename([string new_filename])
 */
PHP_FUNCTION(apache_request_filename)
{
	apache_request_string_slot(offsetof(request_rec, filename), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->uri([string new_uri])
 */
PHP_FUNCTION(apache_request_uri)
{
	apache_request_string_slot(offsetof(request_rec, uri), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->unparsed_uri([string new_unparsed_uri])
 */
PHP_FUNCTION(apache_request_unparsed_uri)
{
	apache_request_string_slot(offsetof(request_rec, unparsed_uri), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->path_info([string new_path_info])
 */
PHP_FUNCTION(apache_request_path_info)
{
	apache_request_string_slot(offsetof(request_rec, path_info), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->args([string new_args])
 */
PHP_FUNCTION(apache_request_args)
{
	apache_request_string_slot(offsetof(request_rec, args), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->boundary()
 */
PHP_FUNCTION(apache_request_boundary)
{
	apache_request_read_string_slot(offsetof(request_rec, boundary), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */


/* {{{ proto string $request->content_type([string new_type])
 */
PHP_FUNCTION(apache_request_content_type)
{
	apache_request_string_slot(offsetof(request_rec, content_type), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->content_encoding([string new_encoding])
 */
PHP_FUNCTION(apache_request_content_encoding)
{
	apache_request_string_slot(offsetof(request_rec, content_encoding), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->handler([string new_handler])
 */
PHP_FUNCTION(apache_request_handler)
{
	apache_request_string_slot(offsetof(request_rec, handler), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->the_request()
 */
PHP_FUNCTION(apache_request_the_request)
{
	apache_request_read_string_slot(offsetof(request_rec, the_request), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->protocol()
 */
PHP_FUNCTION(apache_request_protocol)
{
	apache_request_read_string_slot(offsetof(request_rec, protocol), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->hostname()
 */
PHP_FUNCTION(apache_request_hostname)
{
	apache_request_read_string_slot(offsetof(request_rec, hostname), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->status_line([string new_status_line])
 */
PHP_FUNCTION(apache_request_status_line)
{
	apache_request_string_slot(offsetof(request_rec, status_line), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto string $request->method()
 */
PHP_FUNCTION(apache_request_method)
{
	apache_request_read_string_slot(offsetof(request_rec, method), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* }}} access string slots of request rec */

/* {{{ access int slots of request_rec
 */

/* {{{ proto int $request->proto_num()
 */
PHP_FUNCTION(apache_request_proto_num)
{
	apache_request_read_int_slot(offsetof(request_rec, proto_num), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->assbackwards()
 */
PHP_FUNCTION(apache_request_assbackwards)
{
	apache_request_read_int_slot(offsetof(request_rec, assbackwards), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */


/* {{{ proto int $request->proxyreq([int new_proxyreq])
 */
PHP_FUNCTION(apache_request_proxyreq)
{
	apache_request_int_slot(offsetof(request_rec, proxyreq), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->chunked()
 */
PHP_FUNCTION(apache_request_chunked)
{
	apache_request_read_int_slot(offsetof(request_rec, chunked), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */


/* {{{ proto int $request->header_only()
 */
PHP_FUNCTION(apache_request_header_only)
{
	apache_request_read_int_slot(offsetof(request_rec, header_only), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->request_time()
 */
PHP_FUNCTION(apache_request_request_time)
{
	apache_request_read_int_slot(offsetof(request_rec, request_time), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->status([int new_status])
 */
PHP_FUNCTION(apache_request_status)
{
	apache_request_int_slot(offsetof(request_rec, status), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->method_number([int method_number])
 */
PHP_FUNCTION(apache_request_method_number)
{
	apache_request_read_int_slot(offsetof(request_rec, method_number), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->allowed([int allowed])
 */
PHP_FUNCTION(apache_request_allowed)
{
	apache_request_int_slot(offsetof(request_rec, allowed), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->bytes_sent()
 */
PHP_FUNCTION(apache_request_bytes_sent)
{
	apache_request_read_int_slot(offsetof(request_rec, bytes_sent), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->mtime()
 */
PHP_FUNCTION(apache_request_mtime)
{
	apache_request_read_int_slot(offsetof(request_rec, mtime), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->content_length([int new_content_length])
 */
PHP_FUNCTION(apache_request_content_length)
{
	zval *id, **zlen;
	request_rec *r;

	if (ZEND_NUM_ARGS() == 0) {
		apache_request_read_int_slot(offsetof(request_rec, clength), INTERNAL_FUNCTION_PARAM_PASSTHRU);
	}
	else if (ZEND_NUM_ARGS() > 1) {
		WRONG_PARAM_COUNT;
	}
	else {
		if (zend_get_parameters_ex(1, &zlen) == FAILURE) {
			RETURN_FALSE;
		}

		APREQ_GET_REQUEST(id, r);

		convert_to_long_ex(zlen);
		(void)ap_set_content_length(r, Z_LVAL_PP(zlen));
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto int $request->remaining()
 */
PHP_FUNCTION(apache_request_remaining)
{
	apache_request_read_int_slot(offsetof(request_rec, remaining), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->no_cache()
 */
PHP_FUNCTION(apache_request_no_cache)
{
	apache_request_int_slot(offsetof(request_rec, no_cache), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->no_local_copy()
 */
PHP_FUNCTION(apache_request_no_local_copy)
{
	apache_request_int_slot(offsetof(request_rec, no_local_copy), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int $request->read_body()
 */
PHP_FUNCTION(apache_request_read_body)
{
	apache_request_int_slot(offsetof(request_rec, read_body), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */


/* }}} access int slots of request_rec */

/* {{{ proxy functions for the ap_* functions family
 */

/* {{{ proto int apache_request_server_port()
 */
PHP_FUNCTION(apache_request_server_port)
{
	zval *id;
	request_rec *r;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	RETURN_LONG(ap_get_server_port(r));
}
/* }}} */

/* {{{ proto int apache_request_remote_host([int type])
 */
PHP_FUNCTION(apache_request_remote_host)
{
	zval *id, **ztype;
	request_rec *r;
	char *res;
	int type = REMOTE_NAME;

	switch (ZEND_NUM_ARGS()) {
		case 0:
			break;
		case 1:
			if (zend_get_parameters_ex(1, &ztype) == FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long_ex(ztype);
			type = Z_LVAL_PP(ztype);
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}


	APREQ_GET_REQUEST(id, r);

	res = (char *)ap_get_remote_host(r->connection, r->per_dir_config, type);
	if (res)
		RETURN_STRING(res, 1);

	RETURN_EMPTY_STRING();
}
/* }}} */

/* {{{ proto long apache_request_update_mtime([int dependency_mtime])
 */
PHP_FUNCTION(apache_request_update_mtime)
{
	zval *id, **zmtime;
	request_rec *r;
	int mtime = 0;

	switch (ZEND_NUM_ARGS()) {
		case 0:
			break;
		case 1:
			if (zend_get_parameters_ex(1, &zmtime) == FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long_ex(zmtime);
			mtime = Z_LVAL_PP(zmtime);
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}


	APREQ_GET_REQUEST(id, r);

	RETURN_LONG(ap_update_mtime(r, mtime));
}
/* }}} */


/* {{{ proto void apache_request_set_etag()
 */
PHP_FUNCTION(apache_request_set_etag)
{
	zval *id;
	request_rec *r;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	ap_set_etag(r);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void apache_request_set_last_modified()
 */
PHP_FUNCTION(apache_request_set_last_modified)
{
	zval *id;
	request_rec *r;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	ap_set_last_modified(r);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long apache_request_meets_conditions()
 */
PHP_FUNCTION(apache_request_meets_conditions)
{
	zval *id;
	request_rec *r;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	RETURN_LONG(ap_meets_conditions(r));
}
/* }}} */

/* {{{ proto long apache_request_discard_request_body()
 */
PHP_FUNCTION(apache_request_discard_request_body)
{
	zval *id;
	request_rec *r;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	RETURN_LONG(ap_discard_request_body(r));
}
/* }}} */

/* {{{ proto long apache_request_satisfies()
 */
PHP_FUNCTION(apache_request_satisfies)
{
	zval *id;
	request_rec *r;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	RETURN_LONG(ap_satisfies(r));
}
/* }}} */


/* {{{ proto bool apache_request_is_initial_req()
 */
PHP_FUNCTION(apache_request_is_initial_req)
{
	zval *id;
	request_rec *r;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	RETURN_BOOL(ap_is_initial_req(r));
}
/* }}} */

/* {{{ proto bool apache_request_some_auth_required()
 */
PHP_FUNCTION(apache_request_some_auth_required)
{
	zval *id;
	request_rec *r;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	RETURN_BOOL(ap_some_auth_required(r));
}
/* }}} */

/* {{{ proto string apache_request_auth_type()
 */
PHP_FUNCTION(apache_request_auth_type)
{
	zval *id;
	request_rec *r;
	char *t;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	t = (char *)ap_auth_type(r);
	if (!t)
		RETURN_NULL();

	RETURN_STRING(t, 1);
}
/* }}} */

/* {{{ proto string apache_request_auth_name()
 */
PHP_FUNCTION(apache_request_auth_name)
{
	zval *id;
	request_rec *r;
	char *t;

	if (ZEND_NUM_ARGS() > 0) {
		WRONG_PARAM_COUNT;
	}

	APREQ_GET_REQUEST(id, r);

	t = (char *)ap_auth_name(r);
	if (!t)
		RETURN_NULL();

	RETURN_STRING(t, 1);
}
/* }}} */

/* {{{ proto apache_request_basic_auth_pw()
 */
PHP_FUNCTION(apache_request_basic_auth_pw)
{
	zval *id, *zpw;
	request_rec *r;
	const char *pw;
	long status;

	if (ZEND_NUM_ARGS() != 1) {
		WRONG_PARAM_COUNT;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zpw) == FAILURE) {
	    RETURN_NULL();
	}

	if (!PZVAL_IS_REF(zpw)) {
	    zend_error(E_WARNING, "Parameter wasn't passed by reference");
	    RETURN_NULL();
	}


	APREQ_GET_REQUEST(id, r);

	pw = NULL;
	status = ap_get_basic_auth_pw(r, &pw);
	if (status == OK && pw) {
		ZVAL_STRING(zpw, (char *)pw, 1);
	}
	else
		ZVAL_NULL(zpw);
	RETURN_LONG(status);
}
/* }}} */




/* }}} */

/* {{{ php_apache_request_class_functions
 */
static function_entry php_apache_request_class_functions[] = {
	/* string slots */
	PHP_FALIAS(args,						apache_request_args,				NULL)
	PHP_FALIAS(boundary,					apache_request_boundary,			NULL)
	PHP_FALIAS(content_encoding,			apache_request_content_encoding,	NULL)
	PHP_FALIAS(content_type,				apache_request_content_type,		NULL)
	PHP_FALIAS(filename,					apache_request_filename,			NULL)
	PHP_FALIAS(handler,						apache_request_handler,				NULL)
	PHP_FALIAS(hostname,					apache_request_hostname,			NULL)
	PHP_FALIAS(method,						apache_request_method,				NULL)
	PHP_FALIAS(path_info,					apache_request_path_info,			NULL)
	PHP_FALIAS(protocol,					apache_request_protocol,			NULL)
	PHP_FALIAS(status_line,					apache_request_status_line,			NULL)
	PHP_FALIAS(the_request,					apache_request_the_request,			NULL)
	PHP_FALIAS(unparsed_uri,				apache_request_unparsed_uri,		NULL)
	PHP_FALIAS(uri,							apache_request_uri,					NULL)

	/* int slots */
	PHP_FALIAS(allowed,						apache_request_allowed,				NULL)
	PHP_FALIAS(bytes_sent,					apache_request_bytes_sent,			NULL)
	PHP_FALIAS(chunked,						apache_request_chunked,				NULL)
	PHP_FALIAS(content_length,				apache_request_content_length,		NULL)
	PHP_FALIAS(header_only,					apache_request_header_only,			NULL)
	PHP_FALIAS(method_number,				apache_request_method_number,		NULL)
	PHP_FALIAS(mtime,						apache_request_mtime,				NULL)
	PHP_FALIAS(no_cache,					apache_request_no_cache,			NULL)
	PHP_FALIAS(no_local_copy,				apache_request_no_local_copy,		NULL)
	PHP_FALIAS(proto_num,					apache_request_proto_num,			NULL)
	PHP_FALIAS(proxyreq,					apache_request_proxyreq,			NULL)
	PHP_FALIAS(read_body,					apache_request_read_body,			NULL)
	PHP_FALIAS(remaining,					apache_request_remaining,			NULL)
	PHP_FALIAS(request_time,				apache_request_request_time,		NULL)
	PHP_FALIAS(status,						apache_request_status,				NULL)

	/* proxy functions for the ap_* functions family */
#undef auth_name
#undef auth_type
#undef discard_request_body
#undef is_initial_req
#undef meets_conditions
#undef satisfies
#undef set_etag
#undef set_last_modified
#undef some_auth_required
#undef update_mtime
	PHP_FALIAS(auth_name,					apache_request_auth_name,				NULL)
	PHP_FALIAS(auth_type,					apache_request_auth_type,				NULL)
	PHP_FALIAS(basic_auth_pw,				apache_request_basic_auth_pw,			NULL)
	PHP_FALIAS(discard_request_body,		apache_request_discard_request_body,	NULL)
	PHP_FALIAS(is_initial_req,				apache_request_is_initial_req,			NULL)
	PHP_FALIAS(meets_conditions,			apache_request_meets_conditions,		NULL)
	PHP_FALIAS(remote_host,					apache_request_remote_host,				NULL)
	PHP_FALIAS(satisfies,					apache_request_satisfies,				NULL)
	PHP_FALIAS(server_port,					apache_request_server_port,				NULL)
	PHP_FALIAS(set_etag,					apache_request_set_etag,				NULL)
	PHP_FALIAS(set_last_modified,			apache_request_set_last_modified,		NULL)
	PHP_FALIAS(some_auth_required,			apache_request_some_auth_required,		NULL)
	PHP_FALIAS(update_mtime,				apache_request_update_mtime,			NULL)

	{ NULL, NULL, NULL }
};
/* }}} */


static PHP_MINIT_FUNCTION(apache)
{
	zend_class_entry ce;

#ifdef ZTS
	ts_allocate_id(&php_apache_info_id, sizeof(php_apache_info_struct), (ts_allocate_ctor) php_apache_globals_ctor, NULL);
#else
	php_apache_globals_ctor(&php_apache_info TSRMLS_CC);
#endif
	REGISTER_INI_ENTRIES();


	le_apachereq = zend_register_list_destructors_ex(php_apache_request_free, NULL, "ApacheRequest", module_number);
	INIT_OVERLOADED_CLASS_ENTRY(ce, "ApacheRequest", php_apache_request_class_functions, NULL, NULL, NULL);
	apacherequest_class_entry = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	REGISTER_LONG_CONSTANT("OK",				OK,					CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DECLINED",			DECLINED,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FORBIDDEN",			FORBIDDEN,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AUTH_REQUIRED",		AUTH_REQUIRED,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DONE",				DONE,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SERVER_ERROR",		SERVER_ERROR,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("REDIRECT",			REDIRECT,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BAD_REQUEST",		BAD_REQUEST,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NOT_FOUND",			NOT_FOUND,			CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("M_GET",				M_GET,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_PUT",				M_PUT,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_POST",			M_POST,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_DELETE",			M_DELETE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_CONNECT",			M_CONNECT,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_OPTIONS",			M_OPTIONS,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_TRACE",			M_TRACE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_PATCH",			M_PATCH,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_PROPFIND",		M_PROPFIND,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_PROPPATCH",		M_PROPPATCH,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_MKCOL",			M_MKCOL,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_COPY",			M_COPY,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_MOVE",			M_MOVE,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_LOCK",			M_LOCK,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_UNLOCK",			M_UNLOCK,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("M_INVALID",			M_INVALID,			CONST_CS | CONST_PERSISTENT);

	/* Possible values for request_rec.read_body (set by handling module):
	 *    REQUEST_NO_BODY          Send 413 error if message has any body
	 *    REQUEST_CHUNKED_ERROR    Send 411 error if body without Content-Length
	 *    REQUEST_CHUNKED_DECHUNK  If chunked, remove the chunks for me.
	 *    REQUEST_CHUNKED_PASS     Pass the chunks to me without removal.
	 */
	REGISTER_LONG_CONSTANT("REQUEST_NO_BODY",			REQUEST_NO_BODY,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("REQUEST_CHUNKED_ERROR", 	REQUEST_CHUNKED_ERROR,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("REQUEST_CHUNKED_DECHUNK",	REQUEST_CHUNKED_DECHUNK,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("REQUEST_CHUNKED_PASS",		REQUEST_CHUNKED_PASS,		CONST_CS | CONST_PERSISTENT);
	
	/* resolve types for remote_host() */
	REGISTER_LONG_CONSTANT("REMOTE_HOST",			REMOTE_HOST,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("REMOTE_NAME", 			REMOTE_NAME,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("REMOTE_NOLOOKUP",		REMOTE_NOLOOKUP,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("REMOTE_DOUBLE_REV",		REMOTE_DOUBLE_REV,		CONST_CS | CONST_PERSISTENT);
	
	return SUCCESS;
}


static PHP_MSHUTDOWN_FUNCTION(apache)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

zend_module_entry apache_module_entry = {
	STANDARD_MODULE_HEADER,
	"apache", 
	apache_functions, 
	PHP_MINIT(apache), 
	PHP_MSHUTDOWN(apache), 
	NULL, 
	NULL, 
	PHP_MINFO(apache), 
	NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};

/* {{{ proto bool apache_child_terminate(void)
   Terminate apache process after this request */
PHP_FUNCTION(apache_child_terminate)
{
#ifndef MULTITHREAD
	if (AP(terminate_child)) {
		ap_child_terminate( ((request_rec *)SG(server_context)) );
		RETURN_TRUE;
	} else { /* tell them to get lost! */
		php_error(E_WARNING, "apache.child_terminate is disabled");
		RETURN_FALSE;
	}
#else
		php_error(E_WARNING, "apache_child_terminate() is not supported in this build");
		RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ proto string apache_note(string note_name [, string note_value])
   Get and set Apache request notes */
PHP_FUNCTION(apache_note)
{
	pval **arg_name, **arg_val;
	char *note_val;
	int arg_count = ARG_COUNT(ht);

	if (arg_count<1 || arg_count>2 ||
		zend_get_parameters_ex(arg_count, &arg_name, &arg_val) ==FAILURE ) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_string_ex(arg_name);
	note_val = (char *) table_get(((request_rec *)SG(server_context))->notes, (*arg_name)->value.str.val);
	
	if (arg_count == 2) {
		convert_to_string_ex(arg_val);
		table_set(((request_rec *)SG(server_context))->notes, (*arg_name)->value.str.val, (*arg_val)->value.str.val);
	}

	if (note_val) {
		RETURN_STRING(note_val, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(apache)
{
	module *modp = NULL;
	char output_buf[128];
#if !defined(WIN32) && !defined(WINNT)
	char name[64];
	char modulenames[1024];
	char *p;
#endif
	server_rec *serv;
	extern char server_root[MAX_STRING_LEN];
	extern uid_t user_id;
	extern char *user_name;
	extern gid_t group_id;
	extern int max_requests_per_child;

	serv = ((request_rec *) SG(server_context))->server;


	php_info_print_table_start();

#ifdef PHP_WIN32
	php_info_print_table_row(1, "Apache for Windows 95/NT");
	php_info_print_table_end();
	php_info_print_table_start();
#elif defined(NETWARE)
	php_info_print_table_row(1, "Apache for NetWare");
	php_info_print_table_end();
	php_info_print_table_start();
#else
	php_info_print_table_row(2, "APACHE_INCLUDE", PHP_APACHE_INCLUDE);
	php_info_print_table_row(2, "APACHE_TARGET", PHP_APACHE_TARGET);
#endif

	php_info_print_table_row(2, "Apache Version", SERVER_VERSION);

#ifdef APACHE_RELEASE
	sprintf(output_buf, "%d", APACHE_RELEASE);
	php_info_print_table_row(2, "Apache Release", output_buf);
#endif
	sprintf(output_buf, "%d", MODULE_MAGIC_NUMBER);
	php_info_print_table_row(2, "Apache API Version", output_buf);
	sprintf(output_buf, "%s:%u", serv->server_hostname, serv->port);
	php_info_print_table_row(2, "Hostname:Port", output_buf);
#if !defined(WIN32) && !defined(WINNT)
	sprintf(output_buf, "%s(%d)/%d", user_name, (int)user_id, (int)group_id);
	php_info_print_table_row(2, "User/Group", output_buf);
	sprintf(output_buf, "Per Child: %d - Keep Alive: %s - Max Per Connection: %d", max_requests_per_child, serv->keep_alive ? "on":"off", serv->keep_alive_max);
	php_info_print_table_row(2, "Max Requests", output_buf);
#endif
	sprintf(output_buf, "Connection: %d - Keep-Alive: %d", serv->timeout, serv->keep_alive_timeout);
	php_info_print_table_row(2, "Timeouts", output_buf);
#if !defined(WIN32) && !defined(WINNT)
/*
	This block seems to be working on NetWare; But it seems to be showing
	all modules instead of just the loaded ones
*/
	php_info_print_table_row(2, "Server Root", server_root);

	strcpy(modulenames, "");
	for(modp = top_module; modp; modp = modp->next) {
		strlcpy(name, modp->name, sizeof(name));
		if ((p = strrchr(name, '.'))) {
			*p='\0'; /* Cut off ugly .c extensions on module names */
		}
		strcat(modulenames, name);
		if (modp->next) {
			strcat(modulenames, ", ");
		}
	}
	php_info_print_table_row(2, "Loaded Modules", modulenames);
#endif

	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();

	{
		register int i;
		array_header *arr;
		table_entry *elts;
		request_rec *r;

		r = ((request_rec *) SG(server_context));
		arr = table_elts(r->subprocess_env);
		elts = (table_entry *)arr->elts;
		
		SECTION("Apache Environment");
		php_info_print_table_start();	
		php_info_print_table_header(2, "Variable", "Value");
		for (i=0; i < arr->nelts; i++) {
			php_info_print_table_row(2, elts[i].key, elts[i].val);
		}
		php_info_print_table_end();	
	}

	{
		array_header *env_arr;
		table_entry *env;
		int i;
		request_rec *r;
		
		r = ((request_rec *) SG(server_context));
		SECTION("HTTP Headers Information");
		php_info_print_table_start();
		php_info_print_table_colspan_header(2, "HTTP Request Headers");
		php_info_print_table_row(2, "HTTP Request", r->the_request);
		env_arr = table_elts(r->headers_in);
		env = (table_entry *)env_arr->elts;
		for (i = 0; i < env_arr->nelts; ++i) {
			if (env[i].key && (!PG(safe_mode) || (PG(safe_mode) && strncasecmp(env[i].key, "authorization", 13)))) {
				php_info_print_table_row(2, env[i].key, env[i].val);
			}
		}
		php_info_print_table_colspan_header(2, "HTTP Response Headers");
		env_arr = table_elts(r->headers_out);
		env = (table_entry *)env_arr->elts;
		for(i = 0; i < env_arr->nelts; ++i) {
			if (env[i].key) {
				php_info_print_table_row(2, env[i].key, env[i].val);
			}
		}
		php_info_print_table_end();
	}
}
/* }}} */

/* {{{ proto bool virtual(string filename)
   Perform an Apache sub-request */
/* This function is equivalent to <!--#include virtual...-->
 * in mod_include. It does an Apache sub-request. It is useful
 * for including CGI scripts or .shtml files, or anything else
 * that you'd parse through Apache (for .phtml files, you'd probably
 * want to use <?Include>. This only works when PHP is compiled
 * as an Apache module, since it uses the Apache API for doing
 * sub requests.
 */
PHP_FUNCTION(virtual)
{
	pval **filename;
	request_rec *rr = NULL;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &filename) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string_ex(filename);
	
	if (!(rr = sub_req_lookup_uri ((*filename)->value.str.val, ((request_rec *) SG(server_context))))) {
		php_error(E_WARNING, "Unable to include '%s' - URI lookup failed", (*filename)->value.str.val);
		if (rr) destroy_sub_req (rr);
		RETURN_FALSE;
	}

	if (rr->status != 200) {
		php_error(E_WARNING, "Unable to include '%s' - error finding URI", (*filename)->value.str.val);
		if (rr) destroy_sub_req (rr);
		RETURN_FALSE;
	}

	php_end_ob_buffers(1 TSRMLS_CC);
	php_header();

	if (run_sub_req(rr)) {
		php_error(E_WARNING, "Unable to include '%s' - request execution failed", (*filename)->value.str.val);
		if (rr) destroy_sub_req (rr);
		RETURN_FALSE;
	} else {
		if (rr) destroy_sub_req (rr);
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto array getallheaders(void)
   Alias for apache_request_headers() */
/* }}} */
/* {{{ proto array apache_request_headers(void)
   Fetch all HTTP request headers */
PHP_FUNCTION(apache_request_headers)
{
    array_header *env_arr;
    table_entry *tenv;
    int i;
	
    if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
    }
    env_arr = table_elts(((request_rec *) SG(server_context))->headers_in);
    tenv = (table_entry *)env_arr->elts;
    for (i = 0; i < env_arr->nelts; ++i) {
		if (!tenv[i].key ||
			(PG(safe_mode) &&
			 !strncasecmp(tenv[i].key, "authorization", 13))) {
			continue;
		}
		if (add_assoc_string(return_value, tenv[i].key, (tenv[i].val==NULL) ? "" : tenv[i].val, 1)==FAILURE) {
			RETURN_FALSE;
		}
    }
}
/* }}} */

/* {{{ proto array apache_response_headers(void)
   Fetch all HTTP response headers */
PHP_FUNCTION(apache_response_headers)
{
    array_header *env_arr;
    table_entry *tenv;
    int i;

    if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
    }
    env_arr = table_elts(((request_rec *) SG(server_context))->headers_out);
    tenv = (table_entry *)env_arr->elts;
    for (i = 0; i < env_arr->nelts; ++i) {
		if (!tenv[i].key) continue;
		if (add_assoc_string(return_value, tenv[i].key, (tenv[i].val==NULL) ? "" : tenv[i].val, 1)==FAILURE) {
			RETURN_FALSE;
		}
	}
}
/* }}} */

/* {{{ proto bool apache_setenv(string variable, string value [, bool walk_to_top])
   Set an Apache subprocess_env variable */
PHP_FUNCTION(apache_setenv)
{
	int var_len, val_len, top=0;
	char *var = NULL, *val = NULL;
	request_rec *r = (request_rec *) SG(server_context);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|b", &var, &var_len, &val, &val_len, &top) == FAILURE) {
        RETURN_FALSE;
	}

	while(top) {
		if(r->prev) r = r->prev;
		else break;
	}

	ap_table_setn(r->subprocess_env, ap_pstrndup(r->pool, var, var_len), ap_pstrndup(r->pool, val, val_len));
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto object apache_lookup_uri(string URI)
   Perform a partial request of the given URI to obtain information about it */
PHP_FUNCTION(apache_lookup_uri)
{
	pval **filename;
	request_rec *rr=NULL;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &filename) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string_ex(filename);

	if(!(rr = sub_req_lookup_uri((*filename)->value.str.val, ((request_rec *) SG(server_context))))) {
		php_error(E_WARNING, "URI lookup failed", (*filename)->value.str.val);
		RETURN_FALSE;
	}
	object_init(return_value);
	add_property_long(return_value,"status", rr->status);
	if (rr->the_request) {
		add_property_string(return_value,"the_request", rr->the_request, 1);
	}
	if (rr->status_line) {
		add_property_string(return_value,"status_line", (char *)rr->status_line, 1);		
	}
	if (rr->method) {
		add_property_string(return_value,"method", (char *)rr->method, 1);		
	}
	if (rr->content_type) {
		add_property_string(return_value,"content_type", (char *)rr->content_type, 1);
	}
	if (rr->handler) {
		add_property_string(return_value,"handler", (char *)rr->handler, 1);		
	}
	if (rr->uri) {
		add_property_string(return_value,"uri", rr->uri, 1);
	}
	if (rr->filename) {
		add_property_string(return_value,"filename", rr->filename, 1);
	}
	if (rr->path_info) {
		add_property_string(return_value,"path_info", rr->path_info, 1);
	}
	if (rr->args) {
		add_property_string(return_value,"args", rr->args, 1);
	}
	if (rr->boundary) {
		add_property_string(return_value,"boundary", rr->boundary, 1);
	}
	add_property_long(return_value,"no_cache", rr->no_cache);
	add_property_long(return_value,"no_local_copy", rr->no_local_copy);
	add_property_long(return_value,"allowed", rr->allowed);
	add_property_long(return_value,"sent_bodyct", rr->sent_bodyct);
	add_property_long(return_value,"bytes_sent", rr->bytes_sent);
	add_property_long(return_value,"byterange", rr->byterange);
	add_property_long(return_value,"clength", rr->clength);

#if MODULE_MAGIC_NUMBER >= 19980324
	if (rr->unparsed_uri) {
		add_property_string(return_value,"unparsed_uri", rr->unparsed_uri, 1);
	}
	if(rr->mtime) {
		add_property_long(return_value,"mtime", rr->mtime);
	}
#endif
	if(rr->request_time) {
		add_property_long(return_value,"request_time", rr->request_time);
	}

	destroy_sub_req(rr);
}
/* }}} */


#if 0
This function is most likely a bad idea.  Just playing with it for now.

PHP_FUNCTION(apache_exec_uri)
{
	pval **filename;
	request_rec *rr=NULL;
	TSRMLS_FETCH();

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &filename) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string_ex(filename);

	if(!(rr = ap_sub_req_lookup_uri((*filename)->value.str.val, ((request_rec *) SG(server_context))))) {
		php_error(E_WARNING, "URI lookup failed", (*filename)->value.str.val);
		RETURN_FALSE;
	}
	RETVAL_LONG(ap_run_sub_req(rr));
	ap_destroy_sub_req(rr);
}
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
