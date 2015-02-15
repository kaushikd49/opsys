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
	char input[] = "hello johnny";
	char **tokens = advance_tokenize(input, ' ', '"');
	print_tokens(tokens);
	CuAssert(tc, "match", strcmp(tokens[0], "hello") == 0);
	CuAssert(tc, "match", strcmp(tokens[1], "johnny") == 0);
}


void TestParse2(CuTest* tc) {
	char input[] = "ls";
	char **tokens = advance_tokenize(input, ' ', '"');
	contains_pipe(tokens);
}


void TestScriptRead(CuTest* tc) {
	char str[1000];
	int flag = 1;
	int fileHandle = open("../script1.sh", 0);
	do {
		flag = read_line(str, fileHandle);
		printf("%s\n", str);
	} while (flag == 1);
	close(fileHandle);
	//CuAssert(tc, "match", strlen(str)>=7);
}

CuSuite* CuGetSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, TestParse2);
//	SUITE_ADD_TEST(suite, TestScriptRead);

	return suite;
}

