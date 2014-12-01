abts
====

A modified version of apache C test suite

abts been the most concise testing framework, and now I will take it out from apache, and then use past experience to make some changes:<p>
1. further simplified to make it easier to understand and use, test_suite corresponding unit, test_case corresponding interface.<p>
2. The only abts.h and abts.c, can easily be integrated into various c or cpp project.<p>
3. Increase the -f parameter directly output to a file.<p>


How to use:
In the test case file test_xx.c

static void test_xx_func (abts_case * tc, void * arg)<p>
{<p>
	ABTS_ASSERT (1, "Condition is false");<p>
}<p>
abts_suite * test_xx (abts_suite * suite)<p>
{<p>
	suite = abts_add_suite (suite, "bpool");<p>

	abts_run_test (suite, test_xx_func, NULL);<p>
	// add other test case function<p>

	return suite;<p>
}<p>

In the main function:

extern abts_suite * test_xx (abts_suite * suite);

static abts_suite_fntype testfns [] = {<p>
	test_xx,<p>
	NULL<p>
};<p>

int main (int argc, char * argv [])<p>
{<p>
	return abts_main (argc, argv, testfns);<p>
}<p>
