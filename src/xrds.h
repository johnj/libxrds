/*
+----------------------------------------------------------------------+
| See LICENSE file for further copyright information                   |
+----------------------------------------------------------------------+
| Authors: John Jawed <jawed@php.net>                                  |
|          Rasmus Lerdorf <rasmus@php.net>                             |
+----------------------------------------------------------------------+
*/

#ifndef __XRDS_H__
#define __XRDS_H__

#include "xrds_headers.h"
#include "xrds_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int BOOL;

#ifndef TRUE
#define TRUE 1L
#define FALSE (!TRUE)
#endif

/* libxml */
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#ifndef LIBXML_TREE_ENABLED
fprintf(stderr,"tree support is not in your libxml, cannot continue.\n");
exit(-1);
#endif /* LIBXML_TREE_ENABLED */

typedef xmlChar XRDSchar;

typedef struct _xrdsprotoheaders
{
	char			x_xrds_location[XRDS_MAX_HEADER_LEN];
	char			*content_type;
	unsigned int	req_finished; /* this is used internally to audit header processing redirects with libcurl */
} xrdsprotoheaders;

typedef struct _xrdserviceselement
{
	/* simpleType xs:NonNegativeInteger */
	unsigned long long	priority;
	/* node name, such as URI, Type, MediaType, LocalID, etc */
	XRDSchar			*nodeName;
	/* node value */
	XRDSchar			*nodeValue;
	/* node attrs */
	xmlAttrPtr			attrs;
	/* llist */
	struct _xrdserviceselement	*next;
	struct _xrdserviceselement	*prev;
} xrdserviceelement;

typedef struct _xrdservice
{
	/* simpleType xs:NonNegativeInteger */
	unsigned long long	priority;
	xrdserviceelement	**elements;
	unsigned int		elementNr;
	/* llist */
	struct _xrdservice	*next;
	struct _xrdservice	*prev;
} xrdservice;

typedef struct _xrdelement
{
	BOOL				simpleXRDS;
	/* optional xml:id attribute */
	XRDSchar			*fid;
	/* required (6.2) */
	XRDSchar			*expires;
	XRDSchar			*version;
	xrdservice			**services;
	unsigned int		serviceNr;
	/* llist */
	struct _xrdelement	*next;
	struct _xrdelement	*prev;
} xrdelement;

typedef struct _xrdresult
{
	unsigned long long	priority;
	XRDSchar			*uri;
	unsigned int		nodeNr;
	xrdserviceelement	*other_nodes[XRDS_MAX_ELEMENTS_PER_SERVICE];
} xrdresult;

typedef struct _xrderror
{
	char	*error;
	int		line;
} xrderror;

typedef xrderror	XRDSerror;
typedef xrdresult	XRDSresult;

typedef struct _xrdshandle
{
	xrdelement 		**xrd_elements;
	unsigned int	xrdNr;
	XRDSerror		lastError;
	XRDSresult		*lastResult;
	xmlParserCtxt	*parser_ctxt;
} xrdshandle;

typedef xrdshandle XRDS;
typedef xrds_return XRDSreturn;
typedef xrdserviceelement XRDSelement;

/* functions that assist in creating and releasing various xrds handles */
void xrdsInitialize();
XRDS *xrdsCreate();
xrds_return xrdsGetXrds(xrdshandle *xrdd, const char *path, unsigned int protocols);
void xrdsFreeXrds(xrdshandle *xrdsh);
void xrdsDestroy();

/* functions most APIs will want to use for looking up stuff */
char *xrdsGetURIForType(XRDS *xrdd, const unsigned char *type);
char *xrdsLookupServiceURI(XRDS *xrdd, const unsigned char *service_uri);
char *xrdsLookupURI(XRDS *xrdd, const unsigned char *uri);

/* advanced functions */
char *xrdsFindURIWithNodeValue(XRDS *xrdd, const XRDSchar *nodeName, const XRDSchar *nodeValue, const XRDSchar *fragment);
char *xrdsGetURIForNodes(XRDS *xrdd, const unsigned char *lookup_uri, const XRDSchar *nodeName);

#ifdef __cplusplus
}
#endif

#endif /* __XRDS_H__ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

