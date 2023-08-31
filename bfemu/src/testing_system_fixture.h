#ifndef TESTING_SYSTEM_FIXTURE
#define TESTING_SYSTEM_FIXTURE
#include <gtest/gtest.h>
#include "computer.h"

class FullSystem: public virtual ::testing::Test,
		  public virtual Computer
{
};


#endif //TESTING_SYSTEM_FIXTURE
