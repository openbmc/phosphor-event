#include "sample.h"
#include "message.hpp"
#include <gtest/gtest.h>

TEST(FactorialTest, Zero) {
    EXPECT_EQ(1, Factorial(0));
}

string eventspath = "./events";

void build_event_record(event_record_t *rec,
                        const char *message,
                        const char *severity,
                        const char *association,
                        const char *reportedby,
                        const uint8_t *p,
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
        char *cmd = NULL;

        asprintf(&cmd, "exec rm -r %s 2> /dev/null", eventspath.c_str());
        system(cmd);
        free(cmd);

        asprintf(&cmd, "exec mkdir  %s 2> /dev/null", eventspath.c_str());
        system(cmd);
        free(cmd);

        return;
}

//Run all the tests that were declared with TEST()
int main(int argc, char **argv)
{
   //uint8_t p[] = {0x3, 0x32, 0x34, 0x36};
   //event_record_t rec, *prec;
   string s;

   setup(); //Needed to clear events for the next test
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
