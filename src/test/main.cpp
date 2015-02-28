#include <DetourDecon.h>
#include <cstdint>
#include <cstdio>
#include <cassert>
#include "testunit.h"

TestUnit *g_testHead = NULL;

int main(int argc, char *argv[])
{
	char *bufptr = "";
	TestUnit *unit = g_testHead;
	while(unit)
	{
		fprintf(stderr, "TEST: %s ... ", unit->Name());
		if(unit->Run(&bufptr))
		{
			fprintf(stderr, "success!\n");
		} else {
			fprintf(stderr, "failed(%s)!\n", bufptr);
			return 1;
		}

		unit = unit->Next();
	}
	fprintf(stderr, "\nAll tests succeeded!\n");
	return 0;
}