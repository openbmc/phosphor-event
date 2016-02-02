#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>

#include "message.H"

using namespace std;

string eventspath = "./events";

#define ASSERT(a,b,c)  if(b!=c) {cout << "ASSERT ERROR: line " << __LINE__ << endl; exit(1);} else { cout << "Passed: " << a << endl;}

void build_event_record(event_record_t *rec,
						const char *message,
                        const char *severity,
                        const char *association,
                        const char *reportedby,
                        const uint8_t *p,
                        size_t n) {


    rec->message     = (char*) message;
    rec->severity    = (char*) severity;
    rec->association = (char*) association;
    rec->reportedby  = (char*) reportedby;
    rec->p           = (uint8_t*) p;
    rec->n           = n;

    return;
}

int main(int argc, char *argv[]) {

	uint8_t p[] = {0x3, 0x32, 0x34, 0x36};
	event_record_t rec, *prec;
	string s;


	system("exec rm -r ./events/* 2> /dev/null");

	event_manager m(eventspath);

	ASSERT("Empty DB", m.get_managed_size(), 0);

	ASSERT("No Logs", m.next_log(), 0);
	ASSERT("Double No Logs", m.next_log(), 0);
	m.next_log_refresh();
	ASSERT("No Logs Refresh", m.next_log(), 0);
	ASSERT("latest log 0", m.latest_log_id(), 0);
	ASSERT("Log Entries 0", m.log_count(), 0);

	// Building 1 Event log
	build_event_record(&rec,"Testing Message1", "Info", "Association", "Test", p, 4);
	ASSERT("Create log 1", m.create(&rec), 1);
	ASSERT("1 log is 36 byte", m.get_managed_size(), 75);
	ASSERT("Log Entries 0", m.log_count(), 1);
	ASSERT("latest log 1", m.latest_log_id(), 1);
	m.next_log_refresh();
	ASSERT("Next log w/ 1", m.next_log(), 1);
	m.next_log_refresh();

	// Building 2nd Event log
	build_event_record(&rec,"Testing Message2", "Info", "Association", "Test", p, 4);
	ASSERT("Create log 2", m.create(&rec), 2);
	ASSERT("double event manager size", m.get_managed_size(), 150);
	ASSERT("Log Entries 0", m.log_count(), 2);
	ASSERT("latest log 2", m.latest_log_id(), 2);
	m.next_log_refresh();
	ASSERT("Next Logs scan 1", m.next_log(), 1);
	ASSERT("Next Logs scan 2", m.next_log(), 2);

	// Read Log 1
	ASSERT("Log 1 load", m.open(1, &prec), 1);
	s = prec->message;
	ASSERT("Log 1 validation", s.compare("Testing Message1"), 0);
	m.close(prec);

	// Read Log 2
	ASSERT("Log 2 load", m.open(2, &prec), 2);
	s = prec->message;
	ASSERT("Log 2 validation", s.compare("Testing Message2"), 0);
	m.close(prec);

	// Lets delete the earlier log, then create a new event manager
	// the latest_log_id should still be 2
	m.remove(1);
	ASSERT("manage size with deletion", m.get_managed_size(), 75);

	event_manager q(eventspath);
	ASSERT("latest log 2", q.latest_log_id(), 2);
	ASSERT("Log Entries 0", q.log_count(), 1);
	m.next_log_refresh();


	// Travese log list stuff
	system("exec rm -r ./events/* 2> /dev/null");
	event_manager a(eventspath);
	ASSERT("init next log with no logs", a.next_log(), 0);

	build_event_record(&rec,"Testing list", "Info", "Association", "Test", p, 4);
	a.create(&rec);
	a.create(&rec);

	event_manager b(eventspath);
	ASSERT("init next log with 1 log", b.next_log(), 1);





	return 0;
}