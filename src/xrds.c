/*
+----------------------------------------------------------------------+
| See LICENSE file for further copyright information                   |
+----------------------------------------------------------------------+
| Authors: John Jawed <jawed@php.net>                                  |
|          Rasmus Lerdorf <rasmus@php.net>                             |
+----------------------------------------------------------------------+
*/

#include "xrds_parse.h"
#include "xrds_fetch.h"
#include "xrds_sort.h"

/* this is used for curl_global_init, which isn't thread safe to begin with so there is no reason for this variable to be so */
CURLcode xrds_curl_has_inited;

void xrdsInitialize()
{
	LIBXML_TEST_VERSION
#if !defined(LIBXML_XPATH_ENABLED) || !defined(LIBXML_SAX1_ENABLED)
	fprintf(stderr,"proper XPath support is not enabled in your libxml!\n");
	exit(-1);
#endif
	xrds_curl_has_inited = 1L;
}

void xrdsDestroy()
{
	if(!xrds_curl_has_inited)
	{
		curl_global_cleanup();
	}
	xmlCleanupParser();
}

XRDS *xrdsCreate()
{
	xrdshandle *xrdsh;

	xrdsh = malloc(sizeof(xrdshandle));
	if(xrdsh)
	{
		xrdsh->xrd_elements = NULL;
		xrdsh->lastError.error = NULL;
		xrdsh->lastError.line = 0;
		xrdsh->lastResult = NULL;
		xrdsh->parser_ctxt = NULL;
	}
	return xrdsh;
}

void xrdsBuildLList(xrdshandle *xrdsh)
{
	xrdelement *xrdd = NULL;
	xrdservice *service = NULL;
	unsigned int xrdi = 0, servicesi = 0, elementsi = 0;

	for(xrdi=0;xrdi<xrdsh->xrdNr;xrdi++)
	{
		__XRDS_SET_PREV_NEXT(xrdi,xrdsh->xrd_elements,xrdsh->xrdNr)

		xrdd = xrdsh->xrd_elements[xrdi];
		for(servicesi=0;servicesi<xrdd->serviceNr;servicesi++)
		{
			__XRDS_SET_PREV_NEXT(servicesi,xrdd->services,xrdd->serviceNr)

			service = xrdd->services[servicesi];
			for(elementsi=0;elementsi<service->elementNr;elementsi++)
			{
				__XRDS_SET_PREV_NEXT(elementsi,service->elements,service->elementNr)
			}
		}
	}
}

xrds_return xrdsGetXrds(xrdshandle *xrdd, const char *path, unsigned int flags)
{
	xrds_return discret = XRDS_FAILURE;

	if(!strncasecmp("http://",path,strlen("http://")) || !strncasecmp("https://",path,strlen("https://")))
	{
		/* clean up the current xrdshandle */
		if(xrdd->xrd_elements)
		{
			xrdsFreeXrds(xrdd);
		}

		/* if we don't get protocols we will try all */
		if(flags==0)
		{
			flags = XRDS_PROTOCOL_ALL;
		}
		if(flags & XRDS_PROTOCOL_HEAD)
		{
			discret = xrdsFetchXrdsViaHEAD(xrdd,path);
		}
		/* if discovery fails we have to attempt the GET protocol and thus if only the XRDS_PROTOCOL_HEAD flag was passed it is superseded here by spec ... section 5.1.1 */
		if(discret!=XRDS_OK || flags & XRDS_PROTOCOL_GET)
		{
			discret = xrdsFetchXrdsViaGET(xrdd,path);
		}

		/* if we discovered, sort and get the elements ready to use in future calls */
		if(discret==XRDS_OK)
		{
			xrdsSortPriorities(xrdd);
		}
		if(!(flags & XRDS_NO_LLIST))
		{
			xrdsBuildLList(xrdd);
		}
	}
	else
	{
		discret = XRDS_MALFORMED_URL;
	}
	return discret;
}

char *xrdsFindURIWithNodeValue(XRDS *xrdd, const XRDSchar *nodeName, const XRDSchar *nodeValue, const XRDSchar *fragment)
{
	xrdelement *xrde;
	xrdservice *service;
	xrdserviceelement *element;
	char *uri = NULL;
	XRDSresult *result = NULL;
	unsigned int numxrde = 0, numservice = 0, numelements = 0, isFound = 0;

	xrde = xrdd->xrd_elements[numxrde];
	while(xrde && numxrde<xrdd->xrdNr)
	{
		if(fragment)
		{
			if(!xmlStrEqual(xrde->fid,fragment))
			{
				/* xml:id!=the fragment we were looking for */
				xrde = xrdd->xrd_elements[++numxrde];
				continue;
			}
		}
		service = xrde->services[numservice];
		while(service && numservice<xrde->serviceNr)
		{
			element = service->elements[numelements];
			isFound = 0;

			if(xrdd->lastResult==NULL)
			{
				result = malloc(sizeof(xrdresult));
				if(result)
				{
					memset(result,0,sizeof(xrdresult));
				}
				else
				{
					return NULL;
				}
			}
			else
			{
				result = xrdd->lastResult;
			}

			while(element && numelements<service->elementNr)
			{
				if(!nodeName)
				{
					if(xmlStrEqual(nodeValue,element->nodeValue))
					{
						isFound = 1;
						uri = (char *)nodeValue;
					}
				}
				else
				{
					if(xmlStrEqual(XRDS_URI_NODE,element->nodeName))
					{
						uri = (char *)element->nodeValue;
					}
					else
					{
						if(xmlStrEqual(nodeName,element->nodeName))
						{
							if(xmlStrEqual(nodeValue,element->nodeValue))
							{
								isFound = 1;
							}
						}
					}
				}
				result->other_nodes[numelements] = service->elements[numelements];
				element = service->elements[++numelements];
			}
			result->other_nodes[numelements] = '\0';
			if(isFound)
			{
				result->priority = service->priority;
				result->nodeNr = numelements;
				xrdd->lastResult = result;
				return uri;
			}
			service = xrde->services[++numservice];
			numelements = 0;
		}
		numservice = 0;
		xrde = xrdd->xrd_elements[++numxrde];
	}
	return NULL;
}

char *xrdsGetURIForNodes(XRDS *xrdd, const unsigned char *lookup_uri, const XRDSchar *nodeName)
{
	unsigned char *fragment = NULL, uri[XRDS_MAX_URI_LENGTH] = "", *xid = NULL;
	XRDSchar *fragment_loc = NULL;
	char *ret;
	unsigned int urii = 0;

	/* check to see if there is a fragment */
	fragment_loc = (XRDSchar *)xmlStrchr(lookup_uri,'#');
	if(fragment_loc)
	{
		fragment = fragment_loc + 1;
		xid = xmlStrdup(fragment);
		XRDS_URI_WITHOUT_FRAGMENT(lookup_uri,uri,urii);
	}

	ret = xrdsFindURIWithNodeValue(xrdd,nodeName,fragment ? uri : lookup_uri,xid);

	if(xid)
	{
		free(xid);
	}
	return ret;
}

char *xrdsGetURIForType(XRDS *xrdd, const unsigned char *type_uri)
{
	return xrdsGetURIForNodes(xrdd,type_uri,XRDS_TYPE_NODE);
}

char *xrdsLookupURI(XRDS *xrdd, const unsigned char *uri)
{
	return xrdsGetURIForNodes(xrdd,uri,NULL);
}

void xrdsFreeXrds(xrdshandle *xrdsh)
{
	xrdelement *xrdd = NULL, **xrd_s = NULL;
	xrdservice *service = NULL;
	xrdserviceelement *service_e = NULL;
	unsigned int xrdi = 0, servicesi = 0, elementsi = 0;

	if(xrdsh)
	{
		xrd_s = xrdsh->xrd_elements;
		if(xrd_s)
		{
			/* clean up xrd nodes */
			do
			{
				xrdd = xrd_s[xrdi];
				if(xrdd)
				{
					if(xrdd->services)
					{
						servicesi = 0;
						/* clean up service nodes */
						do
						{
							service = xrdd->services[servicesi];
							if(service)
							{
								elementsi = 0;
								do
								{
									service_e = service->elements[elementsi];
									if(service_e)
									{
										if(service_e->nodeName)
										{
											free(service_e->nodeName);
										}
										if(service_e->nodeValue)
										{
											free(service_e->nodeValue);
										}
										free(service_e);
									}
									++elementsi;
								} while(service_e);
								if(service->elements)
								{
									free(service->elements);
								}
								free(service);
							}
							++servicesi;
						} while(service);
						free(xrdd->services);
					}
					if(xrdd->expires)
					{
						xmlFree(xrdd->expires);
					}
					if(xrdd->fid)
					{
						xmlFree(xrdd->fid);
					}
					free(xrdd);
				}
				++xrdi;
			} while(xrdd);
			free(xrd_s);
		}
		if(xrdsh->lastResult)
		{
			free(xrdsh->lastResult);
		}
		if(xrdsh->lastError.error)
		{
			free(xrdsh->lastError.error);
		}
		free(xrdsh);
	}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

