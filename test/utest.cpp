#include "message.hpp"
#include <gtest/gtest.h>


constexpr auto eventspath = "./events";
uint8_t p[] ={0x3, 0x32, 0x34, 0x36};
event_record_t rec, *prec;
event_manager eventMain(eventspath, 0, 0);


void build_event_record(event_record_t *rec,
                        const char* message,
                        const char* severity,
                        const char* association,
                        const char* reportedby,
                        const uint8_t* p,
                        size_t n)
{
        rec->message     = const_cast<char*> (message);
        rec->severity    = const_cast<char*> (severity);
        rec->association = const_cast<char*> (association);
        rec->reportedby  = const_cast<char*> (reportedby);
        rec->p           = const_cast<uint8_t*> (p);
        rec->n           = n;

        return;
}

void setup(void)
{
        char *cmd = nullptr;
        int resultAsp = 0;
        int resultSys = 0;

        resultAsp = asprintf(&cmd, "exec rm -r %s 2> /dev/null", eventspath);
        if(resultAsp == -1){
            throw std::bad_alloc();
        }
        resultSys = system(cmd);
        if(resultSys != 0){
            throw std::system_error();
        }
        free(cmd);

        resultAsp = asprintf(&cmd, "exec mkdir  %s 2> /dev/null", eventspath);
        if(resultAsp == -1){
            throw std::bad_alloc();
        }
        resultSys = system(cmd);
        if(resultSys != 0){
            throw std::system_error();
        }
        free(cmd);

        return;
}

void tryCatchSetup(void)
{
   try { setup(); }
   catch(std::bad_alloc&) {
      std::cerr << "bad_alloc caught: bad allocation of cmd\n";
      EXPECT_EQ(0, 1); //Fail the test
   } catch(std::system_error&) {
      std::cerr << "system_error caught: system did not execute properly\n";
      EXPECT_EQ(0, 1); //Fail the test
   }
}

/* Setting up main */
TEST(BuildingEventLog, main) {
   EXPECT_EQ(0, eventMain.get_managed_size());
   EXPECT_EQ(0, eventMain.next_log());
   eventMain.next_log_refresh();
   EXPECT_EQ(0, eventMain.next_log());
   EXPECT_EQ(0, eventMain.latest_log_id());
   EXPECT_EQ(0, eventMain.log_count());
}


/* Building 1st Event Log */
TEST(BuildingEventLog, one) {
   build_event_record(&rec,"Testing Message1", "Info",
                           "Association", "Test", p, 4);
   EXPECT_EQ(1,  eventMain.create(&rec));
   EXPECT_EQ(75, eventMain.get_managed_size());
   EXPECT_EQ(1,  eventMain.log_count());
   EXPECT_EQ(1,  eventMain.latest_log_id());
   eventMain.next_log_refresh();
   EXPECT_EQ(1,  eventMain.next_log());
   eventMain.next_log_refresh();
}


/* Building 2nd Event Log */
TEST(BuildingEventLog, two) {
   build_event_record(&rec, "Testing Message2", "Info",
                            "Association", "Test", p, 4);
   EXPECT_EQ(2,   eventMain.create(&rec));
   EXPECT_EQ(150, eventMain.get_managed_size());
   EXPECT_EQ(2,   eventMain.log_count());
   EXPECT_EQ(2,   eventMain.latest_log_id());
   eventMain.next_log_refresh();
   EXPECT_EQ(1,   eventMain.next_log());
   EXPECT_EQ(2,   eventMain.next_log());
}


/* Read Log 1 */
TEST(ReadLog, one) {
   string s;
   EXPECT_EQ(1, eventMain.open(1, &prec));
   s = prec->message;
   EXPECT_EQ(0, s.compare("Testing Message1"));
   eventMain.close(prec);
}


/* Read Log 2 */
TEST(ReadLog, two) {
   std::string s;
   EXPECT_EQ(2, eventMain.open(2, &prec));
   s = prec->message;
   EXPECT_EQ(0, s.compare("Testing Message2"));
   eventMain.close(prec);
}


/* Lets delete the earlier log, then create a new event manager
   the latest_log_id should still be 2 */
TEST(DeleteEarlierLog, one) {
   eventMain.remove(1);
   EXPECT_EQ(75, eventMain.get_managed_size());

   event_manager eventq(eventspath, 0, 0);
   EXPECT_EQ(2, eventq.latest_log_id());
   EXPECT_EQ(1, eventq.log_count());
   eventMain.next_log_refresh();
}


/* Travese log list stuff */
TEST(TraverseLogList, one) {
   tryCatchSetup();
   event_manager eventa(eventspath, 0, 0);
   EXPECT_EQ(0, eventa.next_log());

   build_event_record(&rec, "Testing list", "Info",
                            "Association", "Test", p, 4);
   eventa.create(&rec);
   eventa.create(&rec);

   event_manager eventb(eventspath, 0, 0);
   EXPECT_EQ(1, eventb.next_log());
}


/* Testing the max limits for event logs */
TEST(TestMaxLimitLogs, one) {
   tryCatchSetup();
   event_manager eventd(eventspath, 75, 0);
   build_event_record(&rec, "Testing Message1", "Info",
                            "Association", "Test", p, 4);
   EXPECT_EQ(0, eventd.create(&rec));

   event_manager evente(eventspath, 76, 0);
   build_event_record(&rec, "Testing Message1", "Info",
                            "Association", "Test", p, 4);
   EXPECT_EQ(1, evente.create(&rec));

   tryCatchSetup();
   event_manager eventf(eventspath, 149, 0);
   build_event_record(&rec, "Testing Message1", "Info",
                            "Association", "Test", p, 4);
   EXPECT_EQ(1, eventf.create(&rec));
   EXPECT_EQ(0, eventf.create(&rec));
}


/* Testing the max limits for event logs */
TEST(TestMaxLimitLogs, two) {
   tryCatchSetup();
   event_manager eventg(eventspath, 300, 1);
   build_event_record(&rec, "Testing Message1", "Info",
                            "Association", "Test", p, 4);
   EXPECT_EQ(1, eventg.create(&rec));
   EXPECT_EQ(0, eventg.create(&rec));
   EXPECT_EQ(1, eventg.log_count());

   tryCatchSetup();
   event_manager eventh(eventspath, 600, 3);
   build_event_record(&rec, "Testing Message1", "Info",
                            "Association", "Test", p, 4);
   EXPECT_EQ(1, eventh.create(&rec));
   EXPECT_EQ(2, eventh.create(&rec));
   EXPECT_EQ(3, eventh.create(&rec));
   EXPECT_EQ(0, eventh.create(&rec));
   EXPECT_EQ(3, eventh.log_count());
}


/* Create an abundence of logs, then restart with a limited set  */
/* You should not be able to create new logs until the log size  */
/* dips below the request number                                 */
TEST(CreateLogsRestartSet, one) {
   tryCatchSetup();
   event_manager eventi(eventspath, 600, 3);
   build_event_record(&rec, "Testing Message1", "Info",
                            "Association", "Test", p, 4);
   EXPECT_EQ(1, eventi.create(&rec));
   EXPECT_EQ(2, eventi.create(&rec));
   EXPECT_EQ(3, eventi.create(&rec));
   EXPECT_EQ(0, eventi.create(&rec));
   EXPECT_EQ(3, eventi.log_count());
   event_manager eventj(eventspath, 600, 1);
   EXPECT_EQ(3, eventj.log_count());
   EXPECT_EQ(0, eventj.create(&rec));
   EXPECT_EQ(3, eventj.log_count());

   /* Delete logs to dip below the requested limit */
   EXPECT_EQ(0, eventj.remove(3));
   EXPECT_EQ(2, eventj.log_count());
   EXPECT_EQ(0, eventj.create(&rec));
   EXPECT_EQ(0, eventj.remove(2));
   EXPECT_EQ(1, eventj.log_count());
   EXPECT_EQ(0, eventj.create(&rec));
   EXPECT_EQ(0, eventj.remove(1));
   EXPECT_EQ(0, eventj.log_count());
   EXPECT_EQ(7, eventj.create(&rec));
}


/* Create an abundence of logs, then restart with a limited set  */
/* You should not be able to create new logs until the log size  */
/* dips below the request number                                 */
TEST(CreateLogsRestartSet, two) {
   tryCatchSetup();
   event_manager eventk(eventspath, 600, 100);
   build_event_record(&rec, "Testing Message1", "Info",
                            "Association", "Test", p, 4);
   EXPECT_EQ(1, eventk.create(&rec));
   EXPECT_EQ(2, eventk.create(&rec));
   /* Now we have consumed 150 bytes */
   event_manager eventl(eventspath, 151, 100);
   EXPECT_EQ(0, eventl.create(&rec));
   EXPECT_EQ(0, eventl.remove(2));
   EXPECT_EQ(4, eventl.create(&rec));
}



//Run all the tests that were declared with TEST()
int main(int argc, char **argv)
{
   tryCatchSetup();
   testing::InitGoogleTest(&argc, argv);
   int capture = RUN_ALL_TESTS();
   //Needed to clear events for the next test
   tryCatchSetup();
   return capture;
}
