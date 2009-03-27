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

#ifndef PHP_XRDS_H
#define PHP_XRDS_H

#include "xrds.h"

extern zend_module_entry xrds_module_entry;
#define phpext_xrds_ptr &xrds_module_entry

#define PHP_XRDS_VERSION "0.0.1"
#define XRDS_GET_THIS(o,zv) o = ((php_xrds_obj *)zend_objects_get_address(zv TSRMLS_CC))
 
#define XRDS_METHOD(f) PHP_METHOD(xrds, f)
#define XRDS_ME(f, arginfo, fls) PHP_ME(xrds, f, arginfo, fls)
#define XRDS_CONST(f) REGISTER_LONG_CONSTANT(#f,f,CONST_CS | CONST_PERSISTENT)

#ifdef PHP_XRDS_DEBUG
short unsigned int DEBUG = 1L;
#else
short unsigned int DEBUG = 0L;
#endif

typedef struct __php_xrds {
	zend_object zoxrds;
	XRDS *xrds;
} php_xrds_obj;

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(xrds);
PHP_MINFO_FUNCTION(xrds);

#endif  /* PHP_XRDS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
