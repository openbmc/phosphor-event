#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <systemd/sd-bus.h>
#include "list.h"

uint16_t g_logid = 0;
sd_bus *bus = NULL;
List *glist;


int create_new_log_event(void *userdata,
                         const char *message,
                         const char *severity,
                         const char *association,
                         const char *reportedby,
                         uint8_t *p,
                         size_t n);

typedef struct messageEntry_t {
	char    *message;
	char    *severity;
	char    *reportedby;
	char    *association;
	uint8_t *debugbytes;
	size_t   debuglength;
	size_t   logid;

	sd_bus_slot *messageslot;
	sd_bus_slot *deleteslot;

} messageEntry_t;


void message_storage_delete(messageEntry_t *m) {

	char *path;
	int r;

	asprintf(&path, "/org/openbmc/records/events/%d", m->logid);


	printf("Attempting to delete %s\n", path);

	free(m->message);
	free(m->severity);
	free(m->reportedby);
	free(m->association);
	free(m->debugbytes);


	r = sd_bus_emit_object_removed(bus, path);
	if (r < 0) {
		fprintf(stderr, "Failed to emit the delete signal %s\n", strerror(-r));
		return;
	}

	sd_bus_slot_unref(m->messageslot);
	sd_bus_slot_unref(m->deleteslot);

	free(m);

	return;
}



void message_add(messageEntry_t **n, 
                const char *message,
                const char *severity,
                const char *association,
                const char *reportedby,
                size_t      logid,
                uint8_t    *data,
                size_t      datalen)
{

	messageEntry_t *p = *n = malloc(sizeof(messageEntry_t));

	asprintf(&(*n)->message,     "%s", message);
	asprintf(&(*n)->severity,    "%s", severity);
	asprintf(&(*n)->association, "%s", association);
	asprintf(&(*n)->reportedby,  "%s", reportedby);

	(*n)->logid       = logid;
	(*n)->debugbytes  = (uint8_t*) malloc (datalen);
	(*n)->debuglength = datalen;
	memcpy((*n)->debugbytes, data, datalen);

	(*n)->messageslot = NULL;
	(*n)->deleteslot  = NULL;
	return;
}


static int get_message_dd(sd_bus *bus,
                          const char *path,
                          const char *interface,
                          const char *property,
                          sd_bus_message *reply,
                          void *userdata,
                          sd_bus_error *error) {

	int r;
	messageEntry_t *p = (messageEntry_t *)userdata;
	
	r = sd_bus_message_append_array(reply, 'y', p->debugbytes, p->debuglength);
	if (r < 0) {
	  printf("Error building array for property %s\n", strerror(-r));
	}

	return r;
}


/////////////////////////////////////////////////////////////
// Receives an array of bytes as an esel error log
// returns the messageid in 2 byte format
//  
//  S1 - Message - Simple sentence about the fail
//  S2 - Severity - How bad of a problem is this
//  S3 - Association - sensor path
//  ay - Detailed data - developer debug information
//
/////////////////////////////////////////////////////////////
static int method_accept_host_message(sd_bus_message *m, 
                                      void *userdata, 
                                      sd_bus_error *ret_error) {

	char *message, *severity, *association;
	size_t   n = 4;
	uint8_t *p;
	int r;

	r = sd_bus_message_read(m, "sss", &message, &severity, &association);
	if (r < 0) {
		fprintf(stderr, "Failed to parse the String parameter: %s\n", strerror(-r));
		return r;
	}

	r = sd_bus_message_read_array(m, 'y', (const void **)&p, &n);
	if (r < 0) {
		fprintf(stderr, "Failed to parse the array of bytes parameter: %s\n", strerror(-r));
		return r;
	}

	create_new_log_event(userdata, message, severity, association, "Host", p, n);

	return sd_bus_reply_method_return(m, "q", g_logid);
}


static int method_accept_test_message(sd_bus_message *m, 
                                      void *userdata, 
                                      sd_bus_error *ret_error) {

	uint8_t p[] = {0x30, 0x32, 0x34, 0x36};

	create_new_log_event(userdata, "Testing Message", "Info", "Association", "Test", p, 4);

	return sd_bus_reply_method_return(m, "q", g_logid);
}


static int method_clearall(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {

	Node *n;

	// This deletes one log at a time and seeing how the
	// list shrinks using the NULL works fine here
	while (n = list_get_next_node(glist, NULL)) {
		message_storage_delete(n->data);
		list_delete_node(glist, n);
	}

	return sd_bus_reply_method_return(m, "q", 0);
}


static int method_deletelog(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {

	Node *n = (Node*)userdata;

	message_storage_delete(n->data);
	list_delete_node(glist, n);

	return sd_bus_reply_method_return(m, "q", 0);
}



static const sd_bus_vtable recordlog_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("acceptHostMessage", "sssay", "q", method_accept_host_message, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("acceptTestMessage", NULL, "q", method_accept_test_message, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("Clear", NULL, NULL, method_clearall, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_VTABLE_END
};

static const sd_bus_vtable log_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_PROPERTY("Association", "s", NULL, offsetof(messageEntry_t, association), SD_BUS_VTABLE_PROPERTY_CONST),
	SD_BUS_PROPERTY("Message",  "s", NULL, offsetof(messageEntry_t, message), SD_BUS_VTABLE_PROPERTY_CONST),
	SD_BUS_PROPERTY("Severity", "s", NULL, offsetof(messageEntry_t, severity), SD_BUS_VTABLE_PROPERTY_CONST),
	SD_BUS_PROPERTY("Reported_By", "s", NULL, offsetof(messageEntry_t, reportedby),  SD_BUS_VTABLE_PROPERTY_CONST),
	SD_BUS_PROPERTY("DevelopmentData", "ay", get_message_dd,0, SD_BUS_VTABLE_PROPERTY_CONST),
	SD_BUS_VTABLE_END
};


static const sd_bus_vtable recordlog_delete_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("Delete", NULL, "q", method_deletelog, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_VTABLE_END
};

uint16_t get_new_log_number() {
	return ++g_logid;
}

int create_new_log_event(void *userdata,
                         const char *message,
                         const char *severity,
                         const char *association,
                         const char *reportedby,
                         uint8_t *p, 
                         size_t n) {
	char loglocation[64];
	int r;
	messageEntry_t *m;
	Node *node;
	uint16_t logid = get_new_log_number();


	snprintf(loglocation, sizeof(loglocation), "/org/openbmc/records/events/%d", logid);

	message_add(&m, message, severity, association, reportedby, logid, p, n);

	node = list_add_node(glist, m);

	r = sd_bus_add_object_vtable(bus,
	                             &m->messageslot,
	                             loglocation,
	                             "org.openbmc.record",
	                             log_vtable,
	                             m);
	if (r < 0) {
		fprintf(stderr, "Failed to acquire service name: %s %s\n", loglocation, strerror(-r));
		message_storage_delete(m);
		list_delete_last_node(glist);
		return 0;
	}

	r = sd_bus_add_object_vtable(bus,
	                             &m->deleteslot,
	                             loglocation,
	                             "org.openbmc.Object.Delete",
	                             recordlog_delete_vtable,
	                             node);

	printf("Event Log added %s\n", loglocation);

	r = sd_bus_emit_object_added(bus, loglocation);
	if (r < 0) {
		fprintf(stderr, "Failed to emit signal %s\n", strerror(-r));
		return 0;
	}



	return logid;
}


int start_event_recording(void) {

	int r;

	sd_bus_slot *slot;

	/* Connect to the user bus this time */
	r = sd_bus_open_system(&bus);
	if (r < 0) {
		fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
		goto finish;
	}

	/* Install the object */
	r = sd_bus_add_object_vtable(bus,
	                             &slot,
	                             "/org/openbmc/records/events",  /* object path */
	                             "org.openbmc.recordlog",   /* interface name */
	                             recordlog_vtable,
	                             NULL);
	if (r < 0) {
		fprintf(stderr, "Failed to issue method call: %s\n", strerror(-r));
		goto finish;
	}
	
	/* Take a well-known service name so that clients can find us */
	r = sd_bus_request_name(bus, "org.openbmc.records.events", 0);
	if (r < 0) {
		fprintf(stderr, "Failed to acquire service name: %s\n", strerror(-r));
		goto finish;
	}

	r = sd_bus_add_object_manager(bus, NULL, "/org/openbmc/records/events") ;
	if (r < 0) {
		fprintf(stderr, "Object Manager failure  %s\n", strerror(-r));
		return 0;
	}

	for (;;) {
		r = sd_bus_process(bus, NULL);
		if (r < 0) {
			fprintf(stderr, "Failed to process bus: %s\n", strerror(-r));
			goto finish;
		}
		if (r > 0)
			continue;

		r = sd_bus_wait(bus, (uint64_t) -1);
		if (r < 0) {
			fprintf(stderr, "Failed to wait on bus: %s\n", strerror(-r));
			goto finish;
		}
	}
	finish:
		sd_bus_slot_unref(slot);
		sd_bus_unref(bus);

	return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}


int main(int argc, char *argv[]) {

	glist = list_create();
	return start_event_recording();
}