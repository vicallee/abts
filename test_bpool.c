//test_bpool.c by vical@126.com

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "abts.h"
#include "bpool.h"

static int not_pass = 0;
static bpool_t pool;

static void test_bpool_perf (abts_case *tc, void *arg)
{
	unsigned i, j, k, n, ts, testn = (unsigned)(unsigned long)arg;
	void **pblock;

	not_pass = 1;

	n = testn;
	k = (n+4)/5;
	pblock = (void**)calloc (1, sizeof(void*) * (n+k));
	if (!pblock) 
		return;
	
	bpool_init (&pool, SIZE_AUTO_EXPAND, 31);

	//add
	ts = abts_tick ();
	for (j=0; j<n; j++) {
		pblock[j] = bpool_alloc_block (&pool);
		if (!pblock[j]) {
			n = j;
			k = (n+4)/5;
			break;
		}
	}
	ABTS_LOG ("bpool add %d items, msec=%d", n, abts_tick () - ts);
	
	ABTS_TRUE (pool.nallblock-pool.nfreeblock == n);
			
	//Partial deletion
	ts = abts_tick ();
	for (j=0; j<n; j++) {			
		if (j%5 == 0) {
			bpool_free_block (&pool, pblock[j]);
			pblock[j] = NULL;
		}
	}
	
	ABTS_LOG ("bpool free %d items, msec=%d", k, abts_tick () - ts);
	
	ABTS_TRUE (pool.nallblock-pool.nfreeblock == n - k
			&& pool.nfreeblock > 0);

	//Increase was partially deleted again
	for (j=n; j<n+k; j++) {
		pblock[j] = bpool_alloc_block (&pool);
		if (!pblock[j]) {
			ABTS_TRUE (pblock[j]!=NULL);
		}
	}
	ABTS_TRUE (pool.nallblock-pool.nfreeblock==n);
	
	//remove all
	ts = abts_tick ();
	for (j=0; j<n+k; j++) {
		if (pblock[j]) {
			bpool_free_block (&pool, pblock[j]);
			pblock[j] = NULL;
		}
	}
	ABTS_LOG ("bpool cleanup %d items, msec=%d", n, abts_tick () - ts);

	//cleanup
	ts = abts_tick ();
	bpool_cleanup (&pool);
	ABTS_LOG ("bpool cleanup , msec=%d", abts_tick () - ts);

	free (pblock);
	
	not_pass = n==testn ? 0 : 1;
}

static void test_bpool_free (abts_case *tc, void *arg)
{
	void *block;

	//pool
	bpool_init (&pool, 10, 32);
	
	//Normal operation
	block = bpool_alloc_block (&pool);
	bpool_free_block (&pool, block);
	ABTS_TRUE (pool.nallblock - pool.nfreeblock == 0
			&& pool.nfreeblock > 0);
	
	//Overrun operation
	//bpool_free_block (&pool, block);
	//ABTS_TRUE (pool.nallblock - pool.nfreeblock == 0
	//		&& pool.nfreeblock > 0);
	
	bpool_cleanup (&pool);
}

static void test_bpool_alloc (abts_case *tc, void *arg)
{
	void *block;
	
	//pool
	bpool_init (&pool, 2, 32);
	
	//Normal operation
	block = bpool_alloc_block (&pool);
	ABTS_TRUE (block && pool.nallblock - pool.nfreeblock == 1);
	block = bpool_alloc_block (&pool);
	ABTS_TRUE (block && pool.nallblock - pool.nfreeblock == 2);
	
	//Overrun operation
	block = bpool_alloc_block (&pool);
	ABTS_TRUE (!block && pool.nfreeblock == 0);
	
	bpool_cleanup (&pool);
}

static void test_bpool_init (abts_case *tc, void *arg)
{
	//pool
	bpool_init (&pool, 10, 32);
	ABTS_TRUE (pool.maxcount==10 
			&& pool.nallblock - pool.nfreeblock == 0);
	bpool_cleanup (&pool);
}

abts_suite *test_bpool (abts_suite *suite)
{
	int i;
	suite = abts_add_suite(suite, "bpool");

	abts_run_test (suite, test_bpool_init, NULL);
	abts_run_test (suite, test_bpool_alloc, NULL);
	abts_run_test (suite, test_bpool_free, NULL);

	
	unsigned perfs[] = {10*1000, 100*1000, 1000*1000, 10*1000*1000, /*100*1000*1000, 1000*1000*1000*/};
	unsigned nperfs = sizeof(perfs)/sizeof(perfs[0]);
	
	for (i=0, not_pass=0; i<nperfs && not_pass==0; i++) {
		abts_run_test (suite, test_bpool_perf, (void*)(unsigned long)perfs[i]);
	}
	
	return suite;
}


