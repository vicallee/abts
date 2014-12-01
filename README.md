abts
====

A modified version of apache C test suite

abts been the most concise testing framework, and now I will take it out from apache, and then use past experience to make some changes:
1. further simplified to make it easier to understand and use, test_suite corresponding unit, test_case corresponding interface.
2. The only abts.h and abts.c, can easily be integrated into various c or cpp project.
3. Increase the -f parameter directly output to a file.


How to use:
In the test case file test_xx.c

static void test_xx_func (abts_case * tc, void * arg)
{
	ABTS_ASSERT (1, "Condition is false");
}
abts_suite * test_xx (abts_suite * suite)
{
	suite = abts_add_suite (suite, "bpool");

	abts_run_test (suite, test_xx_func, NULL);
	// add other test case function

	return suite;
}

In the main function:

extern abts_suite * test_xx (abts_suite * suite);

static abts_suite_fntype testfns [] = {
	test_xx,
	NULL
};

int main (int argc, char * argv [])
{
	return abts_main (argc, argv, testfns);
}
