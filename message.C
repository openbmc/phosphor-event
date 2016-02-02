#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdint>
#include <string>
#include <sys/types.h> 
#include <dirent.h> 
#include <sstream>
#include <sys/stat.h>
#include <cstring>
#include "message.H"
#include <time.h>
#include <stddef.h>
#include <cstdio>

const uint32_t g_eyecatcher = 0x4F424D43; // OBMC
const uint16_t g_version    = 1;

struct logheader_t {
    uint32_t eyecatcher;
    uint16_t version;
    uint16_t logid;
    time_t   timestamp;
    uint16_t detailsoffset;
    uint16_t messagelen;
    uint16_t severitylen;
    uint16_t associationlen;
    uint16_t reportedbylen;
    uint16_t debugdatalen;
};



event_manager::event_manager(string path) {
    uint16_t x;

    eventpath = path;

    latestid = 0;
    dirp = NULL;
    logcount = 0;

    // examine the files being managed and advance latestid to that value
    while ( (x = next_log())  ) {
        logcount++;
        if ( x > latestid )
            latestid = x;
    }

    return;
}

event_manager::~event_manager() {

    if (dirp) {
        closedir(dirp);
    }

    return;
}

bool event_manager::is_file_a_log(string str) {

    std::ostringstream buffer;
    ifstream f;
    logheader_t hdr;

    if (!str.compare("."))
        return 0;
    if (!str.compare(".."))
        return 0;

    buffer << eventpath << "/" << str;

    f.open( buffer.str(), ios::binary);

    if (!f.good()) {
        return 0;
    }

    f.read((char*)&hdr, sizeof(hdr));

    if (hdr.eyecatcher != g_eyecatcher)
        return 0;

    return 1;
}

uint16_t event_manager::log_count(void) {
    return logcount;
}
uint16_t event_manager::latest_log_id(void) {
    return latestid;
}
uint16_t event_manager::new_log_id(void) {
    return ++latestid;
}
void event_manager::next_log_refresh(void) {
    if (dirp) {
        closedir(dirp);
        dirp = NULL;
    }
    return;
}

uint16_t event_manager::next_log(void) {

    std::ostringstream buffer;
    struct dirent *ent;
    uint16_t id;


    if (dirp == NULL)
        dirp = opendir(eventpath.c_str());

    if (dirp) {
        do {
            ent = readdir(dirp);

            if (ent == NULL)
                break;

            string str(ent->d_name);

            if (is_file_a_log(str)) {
                id = (uint16_t) atoi(str.c_str());
                break;
            }

        } while( 1 );
    } else {
        cerr << "Error opening directory " << eventpath << endl;
        ent    = NULL;
        id = 0;
    }

    if (ent == NULL) {
        closedir(dirp);
        dirp = NULL;
    }

    return  ((ent == NULL) ? 0 : id);
}


uint16_t event_manager::create(event_record_t *rec) {

    rec->logid = new_log_id();
    rec->timestamp = time(NULL);
    return create_log_event(rec);
}

inline uint16_t getlen(const char *s) {
    return (uint16_t) (1 + strlen(s));
}



size_t event_manager::get_managed_size(void) {
    DIR *dirp;
    std::ostringstream buffer;
    struct dirent *ent;
    ifstream f;

    size_t db_size = 0;

    dirp = opendir(eventpath.c_str());

    if (dirp) {
        while ( (ent = readdir(dirp)) != NULL ) {

            string str(ent->d_name);

            if (is_file_a_log(str)) {

                buffer.str("");
                buffer << eventpath << "/" << str.c_str();

                f.open(buffer.str() , ios::in|ios::binary|ios::ate);
                db_size += f.tellg();
                f.close();
            }
        }
    }

    closedir(dirp);

    return  (db_size);
}

uint16_t event_manager::create_log_event(event_record_t *rec) {

    std::ostringstream buffer;
    ofstream myfile;
    logheader_t hdr = {0};

    buffer << eventpath << "/" << int(rec->logid) ;

    hdr.eyecatcher     = g_eyecatcher;
    hdr.version        = g_version;
    hdr.logid          = rec->logid;
    hdr.timestamp      = rec->timestamp;
    hdr.detailsoffset  = offsetof(logheader_t, messagelen);
    hdr.messagelen     = getlen(rec->message);
    hdr.severitylen    = getlen(rec->severity);
    hdr.associationlen = getlen(rec->association);
    hdr.reportedbylen  = getlen(rec->reportedby);
    hdr.debugdatalen   = rec->n;

    myfile.open(buffer.str() , ios::out|ios::binary);
    myfile.write((char*) &hdr, sizeof(hdr));
    myfile.write((char*) rec->message, hdr.messagelen);
    myfile.write((char*) rec->severity, hdr.severitylen);
    myfile.write((char*) rec->association, hdr.associationlen);
    myfile.write((char*) rec->reportedby, hdr.reportedbylen);
    myfile.write((char*) rec->p, hdr.debugdatalen);
    myfile.close();

    logcount++;

    return rec->logid;
}

int event_manager::open(uint16_t logid, event_record_t **rec) {

    std::ostringstream buffer;
    ifstream f;
    logheader_t hdr;

    buffer << eventpath << "/" << int(logid);

    f.open( buffer.str(), ios::binary );

    if (!f.good()) {
        return 0;
    }

    *rec = new event_record_t;

    f.read((char*)&hdr, sizeof(hdr));

    (*rec)->logid     = hdr.logid;
    (*rec)->timestamp = hdr.timestamp;


    (*rec)->message = new char[hdr.messagelen];
    f.read((*rec)->message, hdr.messagelen);

    (*rec)->severity = new char[hdr.severitylen];
    f.read((*rec)->severity, hdr.severitylen);

    (*rec)->association = new char[hdr.associationlen];
    f.read((*rec)->association, hdr.associationlen);

    (*rec)->reportedby = new char[hdr.reportedbylen];
    f.read((*rec)->reportedby, hdr.reportedbylen);

    (*rec)->p = new uint8_t[hdr.debugdatalen];
    f.read((char*)(*rec)->p, hdr.debugdatalen);
    (*rec)->n = hdr.debugdatalen;


    f.close();
    return logid;
}

void event_manager::close(event_record_t *rec) {

    delete[] rec->message;
    delete[] rec->severity;
    delete[] rec->association;
    delete[] rec->reportedby;
    delete[] rec->p;
    delete rec;

    logcount--;
    return ;
}

int event_manager::remove(uint16_t logid) {

    std::stringstream buffer;
    string s;

    buffer << eventpath << "/" << int(logid);

    s = buffer.str();
    std::remove(s.c_str());

    return 0;
}
