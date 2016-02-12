#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <assert.h>

#include "message.H"

using namespace std;

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

int main(int argc, char *argv[])
{
	uint8_t p[] = {0x3, 0x32, 0x34, 0x36};
	event_record_t rec, *prec;
	string s;


	system("exec rm -r ./events/* 2> /dev/null");

	event_manager m(eventspath);

	assert(m.get_managed_size() == 0);

	assert(m.next_log() == 3);
	assert(m.next_log() == 0);
	m.next_log_refresh();
	assert(m.next_log() == 0);
	assert(m.latest_log_id() == 0);
	assert(m.log_count() == 0);

	// Building 1 Event log
	build_event_record(&rec,"Testing Message1", "Info", "Association", "Test", p, 4);
	assert(m.create(&rec) == 1);
	assert(m.get_managed_size() == 75);
	assert(m.log_count() == 1);
	assert(m.latest_log_id() == 1);
	m.next_log_refresh();
	assert(m.next_log() == 1);
	m.next_log_refresh();

	// Building 2nd Event log
	build_event_record(&rec,"Testing Message2", "Info", "Association", "Test", p, 4);
	assert(m.create(&rec) == 2);
	assert(m.get_managed_size() == 150);
	assert(m.log_count() == 2);
	assert(m.latest_log_id() == 2);
	m.next_log_refresh();
	assert(m.next_log() == 1);
	assert(m.next_log() == 2);

	// Read Log 1
	assert(m.open(1, &prec) == 1);
	s = prec->message;
	assert(s.compare("Testing Message1") == 0);
	m.close(prec);

	// Read Log 2
	assert(m.open(2, &prec) == 2);
	s = prec->message;
	assert(s.compare("Testing Message2") == 0);
	m.close(prec);

	// Lets delete the earlier log, then create a new event manager
	// the latest_log_id should still be 2
	m.remove(1);
	assert(m.get_managed_size() == 75);

	event_manager q(eventspath);
	assert(q.latest_log_id() == 2);
	assert(q.log_count() == 1);
	m.next_log_refresh();

	// Travese log list stuff
	system("exec rm -r ./events/* 2> /dev/null");
	event_manager a(eventspath);
	assert(a.next_log() == 0);

	build_event_record(&rec,"Testing list", "Info", "Association", "Test", p, 4);
	a.create(&rec);
	a.create(&rec);

	event_manager b(eventspath);
	assert(b.next_log() == 1);

	return 0;
}