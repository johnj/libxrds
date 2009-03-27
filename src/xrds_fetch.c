/*
+----------------------------------------------------------------------+
| See LICENSE file for further copyright information                   |
+----------------------------------------------------------------------+
| Authors: John Jawed <jawed@php.net>                                  |
|          Rasmus Lerdorf <rasmus@php.net>                             |
+----------------------------------------------------------------------+
*/

#include "xrds_fetch.h"
#include "xrds_parse.h"

/* xpath and html parser only used here */
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/HTMLparser.h>

/* just for envs which won't optimize strlen(constant), using strlen in this scope is incorrect since strlen is evaluated at runtime not compile time, so sizeof - 1 is used */
static const int sizeXrdsHeader = sizeof(XRDS_HTTP_HEADER_XRDS) - 1;
static const int sizeXrdsBreak = sizeof(XRDS_HTTP_HEADER_BREAK) - 1;

extern CURLcode xrds_curl_has_inited;

BOOL xrdsURIEqualWithoutFragment(const char *uri1, const char *uri2)
{
	char uri1part[XRDS_MAX_URI_LENGTH] = "", uri2part[XRDS_MAX_URI_LENGTH] = "";
	unsigned int ci = 0;

	XRDS_URI_WITHOUT_FRAGMENT(uri1,uri1part,ci);
	XRDS_URI_WITHOUT_FRAGMENT(uri2,uri2part,ci);
	/* at this point uri1part and uri2part should contain strings without uri fragments */

	return !strcmp(uri1part,uri2part); /* uri comparison is case sensitive */
}

static size_t _xrds_read_body(void *ptr, size_t size, size_t nmemb, void *ctx)
{
	size_t body_len,current_body_len;
	long bytes_to_copy;
	char *body, *pbody;

	body = (char *)ctx;
	pbody = (char *)ptr;
	body_len = nmemb * size;
	current_body_len = strlen(body);
	bytes_to_copy = XRDS_MAX_BODY_LEN - current_body_len - body_len - 1;

	if(pbody[body_len]!='\0')
	{
		pbody[body_len] = '\0';
	}
	if(bytes_to_copy<1)
	{
		if((current_body_len+1)<XRDS_MAX_BODY_LEN) /* find out how much we can accomodate */
		{
			bytes_to_copy = bytes_to_copy + body_len;
		}
		else /* no vacancy */
		{
			bytes_to_copy = -1;
		}
	}
	if(bytes_to_copy>0)
	{
		strncat(body,ptr,bytes_to_copy);
	}

	return body_len;
}

static size_t _xrds_read_header(void *ptr, size_t size, size_t nmemb, void *ctx)
{
	char *header;
	xrdsprotoheaders *xhead;
	size_t hlen;
	unsigned int xhead_clen = 0;

	header = (char *)ptr;
	xhead = (xrdsprotoheaders *)ctx;
	hlen = nmemb * size;

	/* we are now following redirects, so we should reset the headers before respecting the current set of headers */
	if(xhead->req_finished)
	{
		XRDS_RESET_PHEADER(xhead);
	}

	if(header[hlen]!='\0')
	{
		header[hlen] = '\0';
	}

	/* handle X-XRDS-Location header */
	if(hlen > sizeXrdsHeader && !strncasecmp(header,XRDS_HTTP_HEADER_XRDS,sizeXrdsHeader))
	{
		header += sizeXrdsHeader + 1 /*:*/;
		xhead_clen += sizeXrdsHeader;
		while(*header==' ' && xhead_clen<(XRDS_MAX_HEADER_LEN))
		{
			header++;
			++xhead_clen;
		}
		strncpy(xhead->x_xrds_location,header,hlen - xhead_clen - 3 /*\r\n\0*/);
	}
	/* the last header callback will be a straight forward CRLF (of a valid http response), confirmed via various http servers and netcat tests */
	else if(hlen >= sizeXrdsBreak && !strncasecmp(header,XRDS_HTTP_HEADER_BREAK,sizeXrdsBreak))
	{
		xhead->req_finished = 1L;
	}
	return hlen;
}

static inline void xrdsPrepCurlHandle(CURL *curl, xrdsprotoheaders *parsed_headers, BOOL headReq)
{
	if(headReq) {
		curl_easy_setopt(curl,CURLOPT_NOBODY,1L);
	}
	curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,_xrds_read_header);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,_xrds_read_body);
	curl_easy_setopt(curl,CURLOPT_WRITEHEADER,parsed_headers);
	curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1L);
	curl_easy_setopt(curl,CURLOPT_MAXREDIRS,XRDS_MAX_REDIRS);
	curl_easy_setopt(curl,CURLOPT_USERAGENT,XRDS_USER_AGENT);
}

xrds_return xrdsFetchXrdsViaHEAD(xrdshandle *xrdd, const char *uri)
{
	return xrdsFetchXrds(xrdd,uri,TRUE);
}

xrds_return xrdsFetchXrdsViaGET(xrdshandle *xrdd, const char *uri)
{
	return xrdsFetchXrds(xrdd,uri,FALSE);
}

static void _xrds_xml_error(void *ctx, const char *msg ATTRIBUTE_UNUSED, ...)
{
	/* just a phantom functon, 	don't want xml errors spewing out to stderr */
}

xrds_return xrdsParseResponse(xrdshandle *xrdd, const char *body)
{
	/* try to parse out the XML or try to find a (DOM)html->head->meta element */
	/* find in html */

	xmlDocPtr doc;
	xmlParserCtxtPtr xctxt;
	xmlErrorPtr lastError;
	htmlParserCtxtPtr hctxt;
	XRDSchar *x_xrds_location;
	XRDSreturn ret = XRDS_FAILURE;

	/* first create an xdrs-specific context */
	xctxt = xmlNewParserCtxt();
	xctxt->sax->error = _xrds_xml_error;

	/* strict parsing first to figure out if this is really an xrds document */
	doc = xmlCtxtReadMemory(xctxt,body,strlen(body),NULL,NULL,0);
	if(doc)
	{
		ret = parseXRDS(xrdd,doc);
		xmlFreeDoc(doc);
	}
	/* wait! there's still hope...in parsing html? eeeek */
	else
	{
		lastError = xmlCtxtGetLastError(xctxt);
		xrdd->lastError.error = strdup(lastError->message);

		/* using htmlCreateMemoryParserCtxt instead of htmlNewParserCtxt, the later is not as portable */
		hctxt = htmlCreateMemoryParserCtxt(body,strlen(body));
		hctxt->sax->error = _xrds_xml_error;
		doc = htmlCtxtReadMemory(hctxt,body,strlen(body),NULL,NULL,HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);

		if(doc)
		{
			/* really wish I could have used xpath here but alas, libxml may have unexpected behavior with bad html and xpath queries */
			x_xrds_location = findXrdsMetaValue(doc);
			if(x_xrds_location)
			{
				/* const BAD_CAST = why? because we know that the URL must be a valid URI val */
				ret = xrdsFetchXrds(xrdd,(const char *)x_xrds_location,FALSE);
			}
			xmlFreeDoc(doc);
		}
		/* this will not happen unless parsing fails horribly */
		else
		{
			ret = XRDS_XML_FAILURE;
		}
		htmlFreeParserCtxt(hctxt);
	}
	xmlFreeParserCtxt(xctxt);
	return ret;
}

xrds_return xrdsFetchXrds(xrdshandle *xrdd, const char *uri,BOOL head_protocol)
{
	xrdsprotoheaders parsed_headers;
	struct curl_slist *client_headers = NULL;
	CURL *curl;
	CURLcode resp;
	char response_body[XRDS_MAX_BODY_LEN] = "";
	BOOL foundValidContentType = FALSE; /* for HEAD protocol only */
	xrds_return ret = XRDS_FAILURE;

	XRDS_RESET_PHEADER((&parsed_headers));

	if(xrds_curl_has_inited)
	{
		xrds_curl_has_inited = curl_global_init(CURL_GLOBAL_ALL);
	}

	curl = curl_easy_init();
	client_headers = curl_slist_append(client_headers,XRDS_ACCEPT_HEADER);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,response_body);

	xrdsPrepCurlHandle(curl,&parsed_headers,head_protocol);
	curl_easy_setopt(curl,CURLOPT_URL,uri);
	curl_easy_setopt(curl,CURLOPT_HTTPHEADER,client_headers);

	resp = curl_easy_perform(curl);

	if(resp==CURLE_OK)
	{
		if(!strlen(parsed_headers.x_xrds_location)) /* there is no xrds location header, let's see if the actual uri is an XRDS doc */
		{
			if(curl_easy_getinfo(curl,CURLINFO_CONTENT_TYPE,&parsed_headers.content_type)==CURLE_OK)
			{
				/* the service provider may or may not set the XRDS content type */
				if(!strncasecmp(parsed_headers.content_type,XRDS_HTTP_HEADER_CONTENT_TYPE_VAL,strlen(XRDS_HTTP_HEADER_CONTENT_TYPE_VAL)))
				{
					foundValidContentType = TRUE;
				}
			}
			if(head_protocol)
			{
				/* the HEAD protocol requires that either a valid Content-Type or X-XRDS-Location be set */
				if(!foundValidContentType)
				{
					ret = XRDS_DISCOVERY_FAILED;
				}
			}
			else /* non-HEAD protocol (which is only GET) */
			{
				/* ok didn't have the content-type header we were looking for, that's fine it didn't have to, let's try to parse this ... */
				ret = xrdsParseResponse(xrdd,response_body);
			}
		}
		/* we have a X-XRDS-Location header OR the X-XRDS-Location == this URI, both cases it is a GET protocol to get the XRDS */
		else if(strlen(parsed_headers.x_xrds_location))
		{
			if(xrdsURIEqualWithoutFragment(uri,parsed_headers.x_xrds_location))
			{
				ret = XRDS_LOCATION_IS_URI;
			}
			else
			{
				ret = xrdsFetchXrds(xrdd,parsed_headers.x_xrds_location,FALSE);
			}
		}
		else
		{
			ret = XRDS_DISCOVERY_FAILED;
		}
	}
	else
	{
		if(resp==CURLE_TOO_MANY_REDIRECTS)
		{
			ret = XRDS_TOO_MANY_REDIRECTS;
		}
	}

	curl_slist_free_all(client_headers);
	curl_easy_cleanup(curl);
	return ret;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

