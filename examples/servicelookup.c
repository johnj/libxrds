/*
+----------------------------------------------------------------------+
| See LICENSE file for further copyright information                   |
+----------------------------------------------------------------------+
| Authors: John Jawed <jawed@php.net>                                  |
+----------------------------------------------------------------------+
*/

#include "xrds.h"
#include <assert.h>
#include <libxml/globals.h>
#include <libxml/xmlmemory.h>
#include <libxml/threads.h>

static void usage(const char *name) {
	assert(name);
	fprintf(stderr, "Usage: %s <url-to-discover> <service-type>\n", name);
	fprintf(stderr, "where <url-to-discover> is a protected resource or XRDS location\n");
	fprintf(stderr, "<service-type> is a URL describing the service (e.g., http://oauth.net/core/1.0/endpoint/request)\n");
}

int main(int argc, char **argv)
{
	XRDSreturn xrds_ret;
	XRDS *xrds;
	char *res = "";
	unsigned char *url_to_discover;
	unsigned char *type;
	int i = 0;

	if((argc < 3)) {
		fprintf(stderr, "Error: wrong number of arguments.\n");
		usage(argv[0]);
		return(-1);
	}

	url_to_discover = (unsigned char*)argv[1];
	type = (unsigned char*)argv[2];

	xrdsInitialize();

	xrds = xrdsCreate();
	xrds_ret = xrdsGetXrds(xrds,(const char*)url_to_discover,XRDS_PROTOCOL_HEAD);

	if(xrds_ret!=XRDS_OK)
	{
		fprintf(stderr,"Something went wrong\n");
		if(xrds->lastError.error)
		{
			fprintf(stderr,"error info: %s\n",xrds->lastError.error);
		}
	}
	else
	{
		res = xrdsGetURIForType(xrds,type);
		if(res)
		{
			fprintf(stderr,"Discovered %s for %s at %s\n",res,type,url_to_discover);
		}
		else
		{
			fprintf(stderr,"No discovery info found for '%s'\n",url_to_discover);
		}
	}
	xrdsFreeXrds(xrds);
	xrdsDestroy();
	return 0;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

