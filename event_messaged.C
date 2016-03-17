#include <iostream>
#include "message.H"
#include "event_messaged_sdbus.h"
#include <string>
#include <unistd.h>

const char *path_to_messages = "/var/lib/obmc/events";

using namespace std;

void message_refresh_events(event_manager *em)
{
	em->next_log_refresh();
	return;
}
uint16_t message_next_event(event_manager *em)
{
	return em->next_log();
}

uint16_t message_create_new_log_event(event_manager *em, event_record_t *rec)
{
	return em->create(rec);
}
int message_load_log(event_manager *em,uint16_t logid, event_record_t **rec)
{
	return em->open(logid, rec);
}
void message_free_log(event_manager *em, event_record_t *rec)
{
	return em->close(rec);
}
int message_delete_log(event_manager *em, uint16_t logid)
{
	return em->remove(logid);
}

int load_existing_events(event_manager *em)
{
	uint16_t id;

	while ( (id = em->next_log()) != 0) {
		send_log_to_dbus(em, id);
	}

	return 0;
}


void print_usage(void)
{
	cout << "[-s <x>] : Maximum bytes to use for event logger"  << endl;
	return;
}


int main(int argc, char *argv[])
{
	unsigned long maxsize=0;
	int rc, c;

	while ((c = getopt (argc, argv, "s:")) != -1)
		switch (c) {
			case 's':
				maxsize =  strtoul(optarg, NULL, 10);
				break;
			case 'h':
			case '?':
				print_usage();
				return 1;
		}


	cout << maxsize <<endl;
	event_manager em(path_to_messages, maxsize);


	rc = build_bus(&em);
	if (rc < 0) {
		fprintf(stderr, "Event Messager failed to connect to dbus rc=%d", rc);
		goto finish;
	}

	rc = load_existing_events(&em);
	if (rc < 0) {
		fprintf(stderr, "Event Messager failed add previous logs to dbus rc=%d", rc);
		goto finish;
	}

	rc = start_event_monitor();

finish:
	cleanup_event_monitor();

	return rc;
}