#include "message.hpp"
#include <gtest/gtest.h>


const std::string eventspath = "./events";
uint8_t p[] = {0x3, 0x32, 0x34, 0x36};
event_record_t rec, *prec;
event_manager m(eventspath, 0, 0);


void build_event_record(event_record_t *rec,
                        const char* message,
                        const char* severity,
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
        char *cmd = nullptr;
        int resultAsp = 0;
        int resultSys = 0;

        resultAsp = asprintf(&cmd, "exec rm -r %s 2> /dev/null", eventspath.c_str());
        if(resultAsp == -1){
            throw std::bad_alloc();
        }
        resultSys = system(cmd);
        if(resultSys != 0){
            throw std::bad_alloc();
        }
        free(cmd);

        resultAsp = asprintf(&cmd, "exec mkdir  %s 2> /dev/null", eventspath.c_str());
        if(resultAsp == -1){
            throw std::bad_alloc();
        }
        resultSys = system(cmd);
        if(resultSys != 0){
            throw std::bad_alloc();
        }
        free(cmd);

        return;
}


/* Setting up main */
TEST(BuildingEventLog, main) {
   EXPECT_EQ(0, m.get_managed_size());
   EXPECT_EQ(0, m.next_log());
   m.next_log_refresh();
   EXPECT_EQ(0, m.next_log());
   EXPECT_EQ(0, m.latest_log_id());
   EXPECT_EQ(0, m.log_count());
}


/* Building 1st Event Log */
TEST(BuildingEventLog, one) {
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
   build_event_record(&rec, "Testing Message2", "Info", "Association", "Test", p, 4);
   EXPECT_EQ(2,   m.create(&rec));
   EXPECT_EQ(150, m.get_managed_size());
   EXPECT_EQ(2,   m.log_count());
   EXPECT_EQ(2,   m.latest_log_id());
   m.next_log_refresh();
   EXPECT_EQ(1,   m.next_log());
   EXPECT_EQ(2,   m.next_log());
}


/* Read Log 1 */
TEST(ReadLog, one) {
   string s;
   EXPECT_EQ(1, m.open(1, &prec));
   s = prec->message;
   EXPECT_EQ(0, s.compare("Testing Message1"));
   m.close(prec);
}


/* Read Log 2 */
TEST(ReadLog, two) {
   string s;
   EXPECT_EQ(2, m.open(2, &prec));
   s = prec->message;
   EXPECT_EQ(0, s.compare("Testing Message2"));
   m.close(prec);
}


/* Lets delete the earlier log, then create a new event manager
   the latest_log_id should still be 2 */
TEST(DeleteEarlierLog, one) {
   m.remove(1);
   EXPECT_EQ(75, m.get_managed_size());

   event_manager q(eventspath, 0, 0);
   EXPECT_EQ(2, q.latest_log_id());
   EXPECT_EQ(1, q.log_count());
   m.next_log_refresh();
}


/* Travese log list stuff */
TEST(TraverseLogList, one) {
   try { setup(); }
   catch(std::bad_alloc&) { std::cerr << "bad_alloc caught: bad allocation of cmd\n";}
   event_manager a(eventspath, 0, 0);
   EXPECT_EQ(0, a.next_log());

   build_event_record(&rec, "Testing list", "Info", "Association", "Test", p, 4);
   a.create(&rec);
   a.create(&rec);

   event_manager b(eventspath, 0, 0);
   EXPECT_EQ(1, b.next_log());
}


/* Testing the max limits for event logs */
TEST(TestMaxLimitLogs, one) {
   try { setup(); }
   catch(std::bad_alloc&) { std::cerr << "bad_alloc caught: bad allocation of cmd\n";}
   //setup();
   event_manager d(eventspath, 75, 0);
   build_event_record(&rec, "Testing Message1", "Info", "Association", "Test", p, 4);
   EXPECT_EQ(0, d.create(&rec));

   event_manager e(eventspath, 76, 0);
   build_event_record(&rec, "Testing Message1", "Info", "Association", "Test", p, 4);
   EXPECT_EQ(1, e.create(&rec));

   try { setup(); }
   catch(std::bad_alloc&) { std::cerr << "bad_alloc caught: bad allocation of cmd\n";}
   event_manager f(eventspath, 149, 0);
   build_event_record(&rec, "Testing Message1", "Info", "Association", "Test", p, 4);
   EXPECT_EQ(1, f.create(&rec));
   EXPECT_EQ(0, f.create(&rec));
}


/* Testing the max limits for event logs */
TEST(TestMaxLimitLogs, two) {
   try { setup(); }
   catch(std::bad_alloc&) { std::cerr << "bad_alloc caught: bad allocation of cmd\n";}
   event_manager g(eventspath, 300, 1);
   build_event_record(&rec, "Testing Message1", "Info", "Association", "Test", p, 4);
   EXPECT_EQ(1, g.create(&rec));
   EXPECT_EQ(0, g.create(&rec));
   EXPECT_EQ(1, g.log_count());

   try { setup(); }
   catch(std::bad_alloc&) { std::cerr << "bad_alloc caught: bad allocation of cmd\n";}
   event_manager h(eventspath, 600, 3);
   build_event_record(&rec, "Testing Message1", "Info", "Association", "Test", p, 4);
   EXPECT_EQ(1, h.create(&rec));
   EXPECT_EQ(2, h.create(&rec));
   EXPECT_EQ(3, h.create(&rec));
   EXPECT_EQ(0, h.create(&rec));
   EXPECT_EQ(3, h.log_count());
}


/* Create an abundence of logs, then restart with a limited set  */
/* You should not be able to create new logs until the log size  */
/* dips below the request number                                 */
TEST(CreateLogsRestartSet, one) {
   try { setup(); }
   catch(std::bad_alloc&) { std::cerr << "bad_alloc caught: bad allocation of cmd\n";}
   event_manager i(eventspath, 600, 3);
   build_event_record(&rec, "Testing Message1", "Info", "Association", "Test", p, 4);
   EXPECT_EQ(1, i.create(&rec));
   EXPECT_EQ(2, i.create(&rec));
   EXPECT_EQ(3, i.create(&rec));
   EXPECT_EQ(0, i.create(&rec));
   EXPECT_EQ(3, i.log_count());
   event_manager j(eventspath, 600, 1);
   EXPECT_EQ(3, j.log_count());
   EXPECT_EQ(0, j.create(&rec));
   EXPECT_EQ(3, j.log_count());

   /* Delete logs to dip below the requested limit */
   EXPECT_EQ(0, j.remove(3));
   EXPECT_EQ(2, j.log_count());
   EXPECT_EQ(0, j.create(&rec));
   EXPECT_EQ(0, j.remove(2));
   EXPECT_EQ(1, j.log_count());
   EXPECT_EQ(0, j.create(&rec));
   EXPECT_EQ(0, j.remove(1));
   EXPECT_EQ(0, j.log_count());
   EXPECT_EQ(7, j.create(&rec));
}


/* Create an abundence of logs, then restart with a limited set  */
/* You should not be able to create new logs until the log size  */
/* dips below the request number                                 */
TEST(CreateLogsRestartSet, two) {
   try { setup(); }
   catch(std::bad_alloc&) { std::cerr << "bad_alloc caught: bad allocation of cmd\n";}
   event_manager k(eventspath, 600, 100);
   build_event_record(&rec, "Testing Message1", "Info", "Association", "Test", p, 4);
   EXPECT_EQ(1, k.create(&rec));
   EXPECT_EQ(2, k.create(&rec));
   /* Now we have consumed 150 bytes */
   event_manager l(eventspath, 151, 100);
   EXPECT_EQ(0, l.create(&rec));
   EXPECT_EQ(0, l.remove(2));
   EXPECT_EQ(4, l.create(&rec));
}



//Run all the tests that were declared with TEST()
int main(int argc, char **argv)
{
   try { setup(); }
   catch(std::bad_alloc&) { std::cerr << "bad_alloc caught: bad allocation of cmd\n";}
   testing::InitGoogleTest(&argc, argv);
   int capture = RUN_ALL_TESTS();
   //Needed to clear events for the next test
   try { setup(); }
   catch(std::bad_alloc&) { std::cerr << "bad_alloc caught: bad allocation of cmd\n";}
   return capture;
}
