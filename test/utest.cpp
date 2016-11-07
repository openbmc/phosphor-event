#include "sample.h"
#include "message.hpp"
#include <gtest/gtest.h>

TEST(FactorialTest, Zero) {
    EXPECT_EQ(1, Factorial(0));
}

std::string eventspath = "./events";

void build_event_record(event_record_t *rec,
                        const char* message,
                        const char*severity,
                        const char* association,
                        const char* reportedby,
                        const uint8_t* p,
                        size_t n)
{
        rec->message     = (char*) message;
        rec->severity    = (char*) severity;
        rec->association = (char*) association;
        rec->reportedby  = (char*) reportedby;
        rec->p           = (uint8_t*) p;
        rec->n           = n;

        return;
}

void setup(void)
{
        char* cmd = nullptr;
	int resultAsp = 0;
	int resultSys = 0;

        resultAsp = asprintf(&cmd, "exec rm -r %s 2> /dev/null", eventspath.c_str());
        resultSys = system(cmd);
	if(resultAsp == -1){
		printf("The system return value is %d\nMemory allocation wasn't possible for cmd.\n", resultSys);
		throw std::bad_alloc();
	}
        free(cmd);

        resultAsp = asprintf(&cmd, "exec mkdir  %s 2> /dev/null", eventspath.c_str());
        resultSys = system(cmd);
	if(resultAsp == -1){
		printf("The system return value is %d\nMemory allocation wasn't possible for cmd.\n", resultSys);
		throw std::bad_alloc();
	}
        free(cmd);

        return;
}

//Run all the tests that were declared with TEST()
int main(int argc, char **argv)
{
   //uint8_t p[] = {0x3, 0x32, 0x34, 0x36};
   //event_record_t rec, *prec;
   
   setup(); //Needed to clear events for the next test
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
