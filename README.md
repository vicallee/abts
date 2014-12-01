abts
====

A modified version of apache C test suite

abts is the most concise testing framework, and now I will take it out from apache, and then to make some changes use my past experience:<p>
1. Further simplified can make it easier to understand and use, test_suite corresponding unit, test_case corresponding interface.<p>
2. Only the abts.h and abts.c can integrated into various c or cpp project easily.<p>
3. Increase the -f parameter output to a file directly.<p>


How to use:
In the test case file test_xx.c<p>
<pre>
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
</pre>

In the main function:<p>
<pre>
extern abts_suite * test_xx (abts_suite * suite);

static abts_suite_fntype testfns [] = {
	test_xx,
	NULL
};

int main (int argc, char * argv [])
{
	return abts_main (argc, argv, testfns);
}
</pre>
