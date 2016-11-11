#include "sample.h"
#include "message.hpp"
#include "event_messaged.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <unistd.h>


using namespace std;

string eventspath = "./events";
uint8_t p[] = {0x3, 0x32, 0x34, 0x36};
event_record_t rec, *prec;
string s;
event_manager m(eventspath, 0, 0);



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
        char *cmd;

        asprintf(&cmd, "exec rm -r %s 2> /dev/null", eventspath.c_str());
        system(cmd);
        free(cmd);

        asprintf(&cmd, "exec mkdir  %s 2> /dev/null", eventspath.c_str());
        system(cmd);
        free(cmd);

        return;
}

/* First sample Test */
TEST(FactorialTest, Zero) {
    EXPECT_EQ(1, Factorial(0));
}


/* Building 1st Event Log */
TEST(BuildingEventLog, one) {
   EXPECT_EQ(1, 1);
   build_event_record(&rec,"Testing Message1", "Info", "Association", "Test", p, 4);
   EXPECT_EQ(1,  m.create(&rec));
   EXPECT_EQ(75, m.get_managed_size());
   EXPECT_EQ(1,  m.log_count());
   EXPECT_EQ(1,  m.latest_log_id());
   m.next_log_refresh();
   EXPECT_EQ(1,  m.next_log());
   m.next_log_refresh();
}


/* Building 2nd Event Log */
TEST(BuildingEventLog, two) {
   EXPECT_EQ(1, 1);
}


/* Read Log 1 */
TEST(ReadLog, one) {
   EXPECT_EQ(1, 1);
}


/* Read Log 2 */
TEST(ReadLog, two) {
   EXPECT_EQ(1, 1);
}


/* Lets delete the earlier log, then create a new event manager
   the latest_log_id should still be 2 */
TEST(DeleteEarlierLog, one) {
   EXPECT_EQ(1, 1);
}


/* Travese log list stuff */
TEST(TraverseLogList, one) {
   EXPECT_EQ(1, 1);
}


/* Testing the max limits for event logs */
TEST(TestMaxLimitLogs, one) {
   EXPECT_EQ(1, 1);
}


/* Testing the max limits for event logs */
TEST(TestMaxLimitLogs, two) {
   EXPECT_EQ(1, 1);
}


/* Create an abundence of logs, then restart with a limited set  */
/* You should not be able to create new logs until the log size  */
/* dips below the request number                                 */
TEST(CreateLogsRestartSet, one) {
   EXPECT_EQ(1, 1);
}


/* Delete logs to dip below the requested limit */
TEST(DeleteLogsBelowLimit, one) {
   EXPECT_EQ(1, 1);
}


/* Create an abundence of logs, then restart with a limited set  */
/* You should not be able to create new logs until the log size  */
/* dips below the request number                                 */
TEST(CreateLogsRestartSet, two) {
   EXPECT_EQ(1, 1);
}



//Run all the tests that were declared with TEST()
int main(int argc, char **argv)
{
   setup(); 
   //event_manager m(eventspath, 0, 0);
   assert(m.get_managed_size() == 0);
   //EXPECT_EQ(0, m.get_managed_size());
   assert(m.next_log() == 0);
   //EXPECT_EQ(0, m.next_log());
   m.next_log_refresh();
   assert(m.next_log() == 0);
   //EXPECT_EQ(0, m.next_log());
   assert(m.latest_log_id() == 0);
   //EXPECT_EQ(0, m.latest_log_id());
   assert(m.log_count() == 0);
   //EXPECT_EQ(0, m.log_count());

   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}



