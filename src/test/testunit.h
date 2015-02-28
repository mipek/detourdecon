#ifndef _include_testunit_h_
#define _include_testunit_h_

#include <cstdio>

#define TEST_COND_IMPL(buffer, cond, msg) \
	if(!(cond)) { *testreason = msg; }

#define TEST_COND(cond, msg) \
	TEST_COND_IMPL(testreason, cond, msg);

class TestUnit;
extern TestUnit *g_testHead;

class TestUnit
{
	TestUnit *next_;
	const char *name_;
public:
	TestUnit(const char *name):
	  next_(g_testHead), name_(name)
	{
		g_testHead = this;
	}

	TestUnit *Next() const
	{
		return next_;
	}

	const char *Name() const
	{
		return name_;
	}

public:
	// Returns true to indicate success.
	virtual bool Run(char **testreason) =0;
};

#endif //_include_testunit_h_