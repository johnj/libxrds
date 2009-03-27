/*
+----------------------------------------------------------------------+
| See LICENSE file for further copyright information                   |
+----------------------------------------------------------------------+
| Authors: John Jawed <jawed@php.net>                                  |
|          Rasmus Lerdorf <rasmus@php.net>                             |
+----------------------------------------------------------------------+
*/

#ifndef __XRDS_PARSE_H__
#define __XRDS_PARSE_H__

#include "xrds.h"

/* libxml */
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LIBXML_TREE_ENABLED
fprintf(stderr,"tree support is not in your libxml\n");
exit(-1);
#endif /* LIBXML_TREE_ENABLED */

#define __XRDS_SET_PRIORITY_NODE(node,datastruct,priority) \
	priority = xmlGetProp(node,XRDS_PRIORITY_ATTR); \
	if(priority) \
	{ \
		if(xmlStrlen(priority)>0) \
		{ \
			if(!xmlStrcasecmp(priority,XRDS_PRIORITY_NULL_STRING)) \
			{ \
				datastruct->priority = XRDS_PRIORITY_NULL; \
			} \
			else \
			{ \
				datastruct->priority = strtoul((const char *)priority,NULL,10); \
			} \
		} \
		xmlFree(priority); \
	}

#define __XRDS_SET_PREV_NEXT(i,handle,max) \
	if(i==0) { \
		handle[0]->prev = NULL; \
	} else { \
		handle[i]->prev = handle[i-1]; \
	} \
	if(i<max) { \
		handle[i]->next = handle[i+1]; \
	} /* the following should never happen, the caller should be careful */ \
	else { \
		handle[i]->next = NULL; \
		handle[i]->prev = NULL; \
	}

xrds_return parseXRDS(xrdshandle *xrdd, xmlDoc *doc);
xrdservice *walkServiceNodes(xmlNodePtr xrdnode);
XRDSchar *findXrdsMetaValue(xmlDocPtr htmldoc);

#ifdef __cplusplus
}
#endif

#endif /* __XRDS_PARSE_H__ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

