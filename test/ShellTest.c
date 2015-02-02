#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CuTest.h"
#include "../bin/shell/shell_functions.h"

CuSuite* CuStringGetSuite(void) {
	CuSuite* suite = CuSuiteNew();
	return suite;
}
void TestParse(CuTest* tc) {
	char input[]="hello johnny";
	char **tokens = advance_tokenize(input, ' ', '"');
	take_action(tokens);
	CuAssert(tc, "match", strcmp(tokens[0],"hello")==0);
	CuAssert(tc, "match", strcmp(tokens[1],"johnny")==0);
}

CuSuite* CuGetSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, TestParse);

	return suite;
}

