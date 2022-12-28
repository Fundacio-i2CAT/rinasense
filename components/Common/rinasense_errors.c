#include "portability/port.h"

string_t GenericErrorMessages[] = {
    /* 0x01 */ "Unknown error",
    /* 0x02 */ "Out of memory",
    /* 0x03 */ "Bad arguments"
};

string_t GpaErrorMessages[] = {
    /* 0x01 */ "Invalid GPA (General Protocol Address)",
    /* 0x02 */ "Invalid GHA (General Hardware Address)"
};

string_t NetBufsMessages[] = {
    /* 0x01 */ "Failed to split netbufs"
};

string_t SerDesMessages[] = {
    /* 0x01 */ "Encoding failed",
    /* 0x02 */ "Decoding failed"
};

string_t RibErrorMessages[] = {
     /* 0x01 */ "RIB object table is full",
     /* 0x02 */ "No pending RIB requests found for invoke ID %d",
     /* 0x03 */ "Neighbor %s was not found",
     /* 0x04 */ "Too many active connections"
     /* 0x05 */ "No such connection",
     /* 0x06 */ "Unsupported RIB object: '%s'",
     /* 0x07 */ "Unsupported method %s on RIB object '%s'"
     /* 0x08 */ "Invalid connection state"
     /* 0x09 */ "Connection already exists"
     /* 0x10 */ "Timed out"
};

string_t EnrollmentErrorMessages[] = {
    /* 0x01 */ "Inbound enrollment process failed"
};

string_t *ErrorMessages[] = {
    /* 0x00 */ GenericErrorMessages,
    /* 0x01 */ GpaErrorMessages,
    /* 0x02 */ NetBufsMessages,
    /* 0x03 */ SerDesMessages,
    /* 0x04 */ RibErrorMessages
};

