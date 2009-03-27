/*
+----------------------------------------------------------------------+
| See LICENSE file for further copyright information                   |
+----------------------------------------------------------------------+
| Authors: John Jawed <jawed@php.net>                                  |
|          Rasmus Lerdorf <rasmus@php.net>                             |
+----------------------------------------------------------------------+
*/

#include "xrds_sort.h"

int _xrds_sort_xrdserviceelement_priorities(const void *a, const void *b)
{
		const xrdserviceelement *xa = *(const xrdserviceelement* const*)a;
		const xrdserviceelement *xb = *(const xrdserviceelement* const*)b;

		__XRDS_QSORT_RESULT(xa,xb)
}

int _xrds_sort_xrdservice_priorities(const void *a, const void *b)
{
		const xrdservice *xa = *(const xrdservice* const*)a;
		const xrdservice *xb = *(const xrdservice* const*)b;

		__XRDS_QSORT_RESULT(xa,xb)
}

void xrdsSortPriorities(XRDS *xrds)
{
	int numxrd = 0, numservices = 0;
	xrdelement *xrde = NULL;
	xrdservice *service = NULL;

	while(numxrd < xrds->xrdNr)
	{
		xrde = xrds->xrd_elements[numxrd++];
		qsort(xrde->services,xrde->serviceNr,sizeof(xrdelement *),_xrds_sort_xrdservice_priorities);
		numservices = 0;
		while(numservices < xrde->serviceNr)
		{
			service = xrde->services[numservices++];
			qsort(service->elements,service->elementNr,sizeof(xrdelement *),_xrds_sort_xrdserviceelement_priorities);
		}
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

