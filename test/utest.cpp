#include "sample.h"
#include <gtest/gtest.h>


/* First sample Test */
TEST(FactorialTest, Zero) {
    EXPECT_EQ(1, Factorial(0));
}


/* Building 1st Event Log */
TEST(BuildingEventLog, one) {
   EXPECT_EQ(1, 1);
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
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}



