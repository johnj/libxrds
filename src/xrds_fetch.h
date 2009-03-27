/*
+----------------------------------------------------------------------+
| See LICENSE file for further copyright information                   |
+----------------------------------------------------------------------+
| Authors: John Jawed <jawed@php.net>                                  |
|          Rasmus Lerdorf <rasmus@php.net>                             |
+----------------------------------------------------------------------+
*/

#ifndef __XRDS_FETCH_H__
#define __XRDS_FETCH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xrds.h"

#define XRDS_RESET_PHEADER(x) \
	memset(x->x_xrds_location, 0L, XRDS_MAX_HEADER_LEN); \
	x->req_finished = 0L;

#define XRDS_URI_WITHOUT_FRAGMENT(x,xx,i) \
	i = 0; \
	while((*x!='\0' && *x!='#') && i<XRDS_MAX_URI_LENGTH) \
	{ \
		xx[i] = *x++; \
		i++; \
	} \
	xx[i] = '\0';

#define XRDS_ACCEPT_HEADER "Accept: application/xrds+xml,text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"

/* libcurl */
#include <curl/curl.h>

xrds_return xrdsFetchXrdsViaHEAD(xrdshandle *xrdd, const char *uri);
xrds_return xrdsFetchXrdsViaGET(xrdshandle *xrdd, const char *uri);
xrds_return xrdsFetchXrds(xrdshandle *xrdd, const char *uri,BOOL head_protocol);
BOOL xrdsURIEqualWithoutFragment(const char *uri1, const char *uri2);

#ifdef __cplusplus
}
#endif

#endif /* __XRDS_FETCH_H__ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

