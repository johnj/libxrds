/*
+----------------------------------------------------------------------+
| See LICENSE file for further copyright information                   |
+----------------------------------------------------------------------+
| Authors: John Jawed <jawed@php.net>                                  |
|          Rasmus Lerdorf <rasmus@php.net>                             |
+----------------------------------------------------------------------+
*/

#ifndef __XRDS_SORT_H__
#define __XRDS_SORT_H__

#include "xrds.h"

#ifdef __cplusplus
extern "C" {
#endif

#define __XRDS_QSORT_RESULT(xa,xb) \
	if(xa->priority < xb->priority) { return -1; } \
	else if(xa->priority > xb->priority) { return 1; } \
	else { return 0; }

void xrdsSortPriorities(XRDS *xrds);

#ifdef __cplusplus
}
#endif

#endif /* __XRDS_SORT_H__ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

