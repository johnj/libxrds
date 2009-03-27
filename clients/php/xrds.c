/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2008 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: John Jawed <jawed@php.net>                                  |
  |          Rasmus Lerdorf <rasmus@php.net>                             |
  +----------------------------------------------------------------------+
*/

/* $Id: $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/file.h"
#include "zend_operators.h"

#include "php_xrds.h"
#include <php/Zend/zend_API.h>

/* Zend class entry */
static zend_class_entry *ce_xrds;

/* handlers */
static zend_object_handlers php_xrds_handlers;

#ifdef COMPILE_DL_XRDS
ZEND_GET_MODULE(xrds)
#endif

static void php_xrds_dtor(void *ze TSRMLS_DC) /* {{{ */
{
	php_xrds_obj *o = (php_xrds_obj *)ze;
	xrdsFreeXrds(o->xrds);
	zend_object_std_dtor(&o->zoxrds TSRMLS_CC);
	efree(o);
}
/* }}} */

static zend_object_value php_xrds_ctor(zend_class_entry *ze TSRMLS_DC) /* {{{ */
{
	php_xrds_obj *o;
	zend_object_value ret;

	o = (php_xrds_obj *)ecalloc(1,sizeof(*o));
	zend_object_std_init(&o->zoxrds, ze TSRMLS_CC);

	ret.handle = zend_objects_store_put(o, (zend_objects_store_dtor_t)zend_objects_destroy_object, php_xrds_dtor, NULL TSRMLS_CC);
	ret.handlers = &php_xrds_handlers;
	return ret;
}
/* }}} */

/* {{{ proto void xrds::__construct() */
XRDS_METHOD(__construct)
{
	php_xrds_obj *o;
	zval *obj = NULL;

	if(zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &obj, ce_xrds) == FAILURE)
	{
		RETURN_FALSE;
	}

	XRDS_GET_THIS(o,obj);

	o->xrds = xrdsCreate();
}
/* }}} */

/* {{{ proto void xrds::discover(string uri) */
XRDS_METHOD(discover)
{
	XRDSreturn xrds_ret;
	php_xrds_obj *o;
	zval *obj = NULL;
	char *uri;
	int uri_len = 0;

	if(zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &obj, ce_xrds, &uri, &uri_len) == FAILURE)
	{
		return;
	}

	XRDS_GET_THIS(o,obj);

	if(uri_len<1)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "URI to discover cannot be empty");
		RETURN_FALSE;
	}

	xrds_ret = xrdsGetXrds(o->xrds,uri,XRDS_PROTOCOL_HEAD);

	if(xrds_ret==XRDS_OK)
	{
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/* {{{ proto string xrds::ServiceURIForType(string type) */
XRDS_METHOD(ServiceURIForType)
{
	php_xrds_obj *o;
	zval *obj = NULL;
	char *uri = NULL, *type = NULL;
	int type_len = 0;

	if(zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &obj, ce_xrds, &type, &type_len) == FAILURE)
	{
		return;
	}

	XRDS_GET_THIS(o,obj);

	uri = xrdsGetURIForType(o->xrds,(unsigned char *)type);

	if(uri)
	{
		RETURN_STRING(uri,1);
	}
	RETURN_NULL();
}
/* }}} */

/* {{{ proto array xrds::childElements(string uri [, bool all_priorities ]) */
XRDS_METHOD(childElements)
{
	php_xrds_obj *o;
	zval *obj = NULL, *attrs = NULL;
	char *uri = NULL;
	zend_bool all_priorities = 0;
	unsigned int uri_len = 0, numelement = 0, numresults = 0;
	char *res = NULL;
	XRDSelement *element = NULL;

	if(zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|b", &obj, ce_xrds, &uri, &uri_len, &all_priorities) == FAILURE)
	{
		return;
	}

	XRDS_GET_THIS(o,obj);

	res = xrdsLookupURI(o->xrds,(unsigned char *)uri);

	if(res)
	{
		array_init(return_value);
		numresults = all_priorities ? o->xrds->lastResult->nodeNr : 1;
		element = o->xrds->lastResult->other_nodes[numelement];
		while(numelement<numresults && element)
		{
			MAKE_STD_ZVAL(attrs);
			array_init(attrs);
			add_assoc_long_ex(attrs, (const char *)XRDS_PRIORITY_ATTR, strlen((const char *)XRDS_PRIORITY_ATTR) + 1, element->priority==XRDS_PRIORITY_NULL ? LONG_MAX : (long)element->priority);
			add_assoc_string_ex(attrs, "node", sizeof("node"), (char *)element->nodeName, 1);
			add_assoc_string_ex(attrs, "value", sizeof("value"), (char *)element->nodeValue, 1);
			add_next_index_zval(return_value,attrs);
			element = o->xrds->lastResult->other_nodes[++numelement];
		}
	}
	else
	{
		RETURN_NULL();
	}
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_xrds__no_params, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xrds__discover, 0, 0, 1)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xrds__serviceurifortype, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xrds__childelements, 0, 0, 1)
	ZEND_ARG_INFO(0, uri)
	ZEND_ARG_INFO(0, all_priorities)
ZEND_END_ARG_INFO()

static zend_function_entry xrds_obj_methods[] = { /* {{{ */
    XRDS_ME(__construct,arginfo_xrds__no_params, ZEND_ACC_PUBLIC)
    XRDS_ME(discover,arginfo_xrds__discover, ZEND_ACC_PUBLIC)
    XRDS_ME(ServiceURIForType,arginfo_xrds__serviceurifortype, ZEND_ACC_PUBLIC)
    XRDS_ME(childElements,arginfo_xrds__childelements, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(xrds)
{
    zend_class_entry ce;

    memcpy(&php_xrds_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    INIT_CLASS_ENTRY(ce, "xrds", xrds_obj_methods);
    ce_xrds = zend_register_internal_class(&ce TSRMLS_CC);
    ce_xrds->create_object = php_xrds_ctor;

    xrdsInitialize();

    return SUCCESS;
}

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(xrds)
{
    xrdsDestroy();

    return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(xrds)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Revision", "$Revision: $");
    php_info_print_table_header(2, "Version", PHP_XRDS_VERSION);
    php_info_print_table_end();
}
/* }}} */

static zend_function_entry xrds_functions[] = { /* {{{ */
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ xrds_module_entry
*/
zend_module_entry xrds_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "xrds",
    xrds_functions,
    PHP_MINIT(xrds),
    PHP_MSHUTDOWN(xrds),
    NULL,
    NULL,
    PHP_MINFO(xrds),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_XRDS_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
