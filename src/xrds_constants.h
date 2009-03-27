/*
+----------------------------------------------------------------------+
| See LICENSE file for further copyright information                   |
+----------------------------------------------------------------------+
| Authors: John Jawed <jawed@php.net>                                  |
|          Rasmus Lerdorf <rasmus@php.net>                             |
+----------------------------------------------------------------------+
*/

#ifndef __XRDS_CONSTANTS_H__
#define __XRDS_CONSTANTS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* by default "null" priorities (omitted or otherwise) are the lowest, to keep this sane it's internally represented by the largest possible value of xs:NonNegativeInteger ... see section 6.4 */
#define XRDS_PRIORITY_NULL 18446744073709551615ULL

#define XRDS_MAX_XRD_PER_XRDS 4096L
#define XRDS_MAX_SERVICES_PER_XRD 4096L
#define XRDS_MAX_ELEMENTS_PER_SERVICE 4096L
#define XRDS_MAX_HEADER_LEN 4096L
#define XRDS_MAX_URI_LENGTH XRDS_MAX_HEADER_LEN

/* max bytes (- 1) that we'll fill up the response buffer up to, mainly when searching for the xrds <meta> in a html doc (32768 = 32KB) */
/** reasoning behind the default 32KB size was mostly influenced by the data here: http://www.optimizationweek.com/reviews/average-web-page/
 * since the average HTML size is 25KB it's not entirely unreasonable to infer that (DOM)html->head->meta is somewhere in the first 32KB of the payload...
 * ...this may not hold true for 100% of pages out there but will hold true for the majority ... of course it becomes a problem it should be increased */
#define XRDS_MAX_BODY_LEN 32768L

#define XRDS_MAX_REDIRS 32L

#define XRDS_XRD_SIMPLE_TYPE (const xmlChar *)"xri://$xrds*simple"

#define XRDS_SERVICE_NODE (const xmlChar *)"Service"
#define XRDS_TYPE_NODE (const xmlChar *)"Type"
#define XRDS_EXPIRES_NODE (const xmlChar *)"Expires"
#define XRDS_MEDIATYPE_NODE (const xmlChar *)"MediaType"
#define XRDS_URI_NODE (const xmlChar *)"URI"
#define XRDS_LOCALID_NODE (const XRDSchar *)"LocalID"
#define XRDS_MUSTSUPPORT_NODE (const XRDSchar *)"simple:MustSupport"
#define XRDS_HTTP_HEADER_BREAK "\r\n"
#define XRDS_HTTP_HEADER_XRDS "X-XRDS-Location"
#define XRDS_HTTP_HEADER_CONTENT_TYPE "Content-Type"
#define XRDS_HTTP_HEADER_CONTENT_TYPE_VAL "application/xrds+xml"
#define XRDS_USER_AGENT "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.0.3) Gecko/2008092510 Ubuntu/8.04 (hardy) Firefox/3.0.3"
#define XRDS_XML_NS (const XRDSchar *)"http://www.w3.org/XML/1998/namespace"

/* wanted to use XML_XML_ID here but ran into weirdness with the libxml parser and fixed attributes */
#define XRDS_ID_ATTR (const XRDSchar *)"id"
#define XRDS_PRIORITY_NULL_STRING (const XRDSchar *)"null"

/* html parsing relevant constants */
#define XRDS_HTML_HEAD_NODE (const XRDSchar *)"head"
#define XRDS_HTML_META_NODE (const XRDSchar *)"meta"
#define XRDS_META_VAL_ATTR (const XRDSchar *)"http-equiv"
#define XRDS_META_XRDS_VAL (const XRDSchar *)"X-XRDS-Location"
#define XRDS_META_CONTENT_ATTR (const XRDSchar *)"content"
#define XRDS_PRIORITY_ATTR (const XRDSchar *)"priority"

#define XRDS_PROTOCOL_HEAD (1<<0L)
#define XRDS_PROTOCOL_GET (1<<1L)
#define XRDS_NO_LLIST (1<<2L)
#define XRDS_PROTOCOL_ALL (XRDS_PROTOCOL_HEAD | XRDS_PROTOCOL_GET)

typedef enum {
	XRDS_OK,
	XRDS_FAILURE,
	XRDS_MALFORMED_URL,
	XRDS_TOO_MANY_REDIRECTS,
	XRDS_DISCOVERY_FAILED,
	XRDS_XML_FAILURE,
	XRDS_LOCATION_IS_URI
} xrds_return;

#ifdef __cplusplus
}
#endif

#endif /* __XRDS_CONSTANTS_H__ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

