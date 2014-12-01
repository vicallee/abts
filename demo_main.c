//demo_main.c by vical@126.com
#include <stdio.h>

#include "abts.h"

extern abts_suite *test_bpool (abts_suite *suite);

static abts_suite_fntype testfns[] = {
	test_bpool,
	NULL
};

int main (int argc, char *argv[])
{
	printf("ABTEST:\n");
	return abts_main (argc, argv, testfns);
}

