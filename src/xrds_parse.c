/*
+----------------------------------------------------------------------+
| See LICENSE file for further copyright information                   |
+----------------------------------------------------------------------+
| Authors: John Jawed <jawed@php.net>                                  |
|          Rasmus Lerdorf <rasmus@php.net>                             |
+----------------------------------------------------------------------+
*/

#include "xrds_parse.h"

xrdelement *walkXRDNodes(xmlNodePtr xrdnode)
{
	xmlAttrPtr attrs;
    xmlNodePtr xrdchildnode = NULL;
	xmlChar *nodeval = NULL;
	xrdelement *ret_xrd = NULL;
	unsigned int numservices = 0, servicememsize;
	xrdservice *services[XRDS_MAX_SERVICES_PER_XRD + 1];

	ret_xrd = malloc(sizeof(xrdelement));
	if(!ret_xrd)
	{
		return NULL;
	}
	memset(services, 0, sizeof(xrdservice *) * (XRDS_MAX_SERVICES_PER_XRD + 1));
	ret_xrd->simpleXRDS = FALSE;
	ret_xrd->expires = NULL;
	ret_xrd->fid = NULL;
	xrdchildnode = xrdnode;

	/* walk across the XRD nodes */
	while(xrdchildnode &&
			(numservices < XRDS_MAX_SERVICES_PER_XRD))
	{
		if(xrdchildnode->type==XML_ELEMENT_NODE)
		{
			attrs = xrdchildnode->properties;
			if(xmlStrEqual(xrdchildnode->name,XRDS_TYPE_NODE))
			{
				nodeval = xmlNodeGetContent(xrdchildnode);
				if(xmlStrEqual(nodeval,XRDS_XRD_SIMPLE_TYPE))
				{
					ret_xrd->simpleXRDS = TRUE;
				}
				xmlFree(nodeval);
			}
			else if(xmlStrEqual(xrdchildnode->name,XRDS_SERVICE_NODE))
			{
				services[numservices++] = walkServiceNodes(xrdchildnode);
			}
			else if(xmlStrEqual(xrdchildnode->name,XRDS_EXPIRES_NODE))
			{
				 ret_xrd->expires = xmlNodeGetContent(xrdchildnode);
			}
		}
		xrdchildnode = xrdchildnode->next;
    }

	/* populate the services we've got */
	servicememsize = sizeof(xrdservice *) * (numservices + 1 /* NULL terminated */);
	ret_xrd->services = malloc(servicememsize);
	if(!ret_xrd->services)
	{
		return NULL;
	}
	memmove(ret_xrd->services,services,servicememsize);
	ret_xrd->services[numservices] = NULL;
	ret_xrd->serviceNr = numservices;
	return ret_xrd;
}

xrdservice *walkServiceNodes(xmlNodePtr servicenode)
{
    xmlNodePtr servicechildnode = NULL;
	xrdserviceelement *element = NULL, *serviceelements[XRDS_MAX_ELEMENTS_PER_SERVICE + 1];
	xrdservice *service;
	xmlChar *priority = NULL;
	unsigned int numelements = 0, elememsize;

	servicechildnode = servicenode->children->next;
	service = malloc(sizeof(xrdservice));
	if(!service)
	{
		return NULL;
	}
	service->priority = XRDS_PRIORITY_NULL;
	service->elements = NULL;
	memset(serviceelements, 0, sizeof(xrdserviceelement *) * (XRDS_MAX_ELEMENTS_PER_SERVICE + 1));

	while(servicechildnode &&
			(numelements < XRDS_MAX_ELEMENTS_PER_SERVICE))
	{
		if(servicechildnode->type==XML_ELEMENT_NODE)
		{
			element = malloc(sizeof(xrdserviceelement));
			if(!element)
			{
				return NULL;
			}
			element->nodeName = xmlStrdup(servicechildnode->name);
			element->nodeValue = xmlNodeGetContent(servicechildnode);
			element->attrs = servicechildnode->properties;
			element->priority = XRDS_PRIORITY_NULL;

			/* the priority attribute on the service child node */
			__XRDS_SET_PRIORITY_NODE(servicechildnode,element,priority)

			serviceelements[numelements++] = element;
		}
		servicechildnode = servicechildnode->next;
    }
	elememsize = (sizeof(xrdserviceelement *) * (numelements + 1) /* NULL terminated */);
	service->elements = malloc(elememsize);
	if(!service->elements)
	{
		return NULL;
	}
	memset(service->elements, 0, elememsize);
	memcpy(service->elements, serviceelements, elememsize);
	service->elements[numelements] = NULL;
	service->elementNr = numelements;

	/* finally the priority attribute on the service node */
	if(servicenode->type==XML_ELEMENT_NODE)
	{
		__XRDS_SET_PRIORITY_NODE(servicenode,service,priority)
	}

	return service;
}

xrds_return parseXRDS(xrdshandle *xrd, xmlDocPtr xmldoc)
{
    xmlNodePtr xrdnode = NULL, xrdsnode = NULL;
	xrdelement **xrdd_s, *xrdd, *vlaxrd[XRDS_MAX_XRD_PER_XRDS + 1];
	unsigned int xrdmemsize, numxrds = 0;

	xrdsnode = xmlDocGetRootElement(xmldoc);
	xrdnode = xrdsnode->children;

	/* going through XRD nodes */
	while(xrdnode && numxrds<XRDS_MAX_XRD_PER_XRDS)
	{
		if(xrdnode->type==XML_ELEMENT_NODE)
		{
			xrdd = walkXRDNodes(xrdnode->children);
			if(xrdd)
			{
				xrdd->fid = xmlGetNsProp(xrdnode,XRDS_ID_ATTR,XRDS_XML_NS);
				vlaxrd[numxrds++] = xrdd;
			}
		}
		xrdnode = xrdnode->next;
	}
	xrdmemsize = sizeof(xrdelement) * (numxrds + 1);
	xrdd_s = malloc(xrdmemsize);
	if(!xrdd_s)
	{
		return XRDS_FAILURE;
	}
	memset(xrdd_s, 0, xrdmemsize);
	memcpy(xrdd_s, vlaxrd, xrdmemsize);
	xrdd_s[numxrds] = NULL;
	xrd->xrd_elements = xrdd_s;
	xrd->xrdNr = numxrds;
	return XRDS_OK;
}

/* caller must free non-null return value */
XRDSchar *findXrdsMetaValue(xmlDocPtr htmldoc)
{
	xmlNodePtr htmlnode, headnode, metanode;
	xmlChar *contentprop, *valprop;

	htmlnode = xmlDocGetRootElement(htmldoc);
	headnode = htmlnode->children;

	/* all these constants are used in a case insensitive manner even though they may be defined otherwise */
	while(headnode)
	{
		if(!xmlStrcasecmp(XRDS_HTML_HEAD_NODE,headnode->name))
		{
			metanode = headnode->children;
			while(metanode)
			{
				if(!xmlStrcasecmp(XRDS_HTML_META_NODE,metanode->name))
				{
					/* first check: make sure this meta tag has an http-equiv attribute */
					valprop = xmlGetProp(metanode,XRDS_META_VAL_ATTR);
					if(valprop)
					{
						/* second check: make sure the http-equiv value is what we are looking for */
						if(!xmlStrcasecmp(XRDS_META_XRDS_VAL,valprop))
						{
							/* grab the content attribute */
							contentprop = xmlGetProp(metanode,XRDS_META_VAL_ATTR);
							if(contentprop)
							{
								return contentprop;
							}
						}
						xmlFree(valprop);
					}
				}
				metanode = metanode->next;

			}
		}
		headnode = headnode->next;
	}
	return NULL;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

