#ifndef dot_doc_file_beginning
#define dot_doc_file_beginning

/* Predetermined values for the WDBF. */
class PD_WDBF_values
{
public:
    const ut_BYTE dot_doc_header_sig[8]     = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
    const ut_BYTE dot_doc_req_minor_v[2]    = {0x3E, 0x00};
    const ut_BYTE dot_doc_majr_vers_3[2]    = {0x03, 0x00};
    const ut_BYTE dot_doc_majr_vers_4[2]    = {0x04, 0x00};
    const ut_BYTE dot_doc_byte_order[2]     = {0xFE, 0xFF};
    const ut_BYTE dot_doc_MV3_SS[2]         = {0x09, 0x00}; // MV3_SS - Major Version 3 Sector Size
    const ut_BYTE dot_doc_MV4_SS[2]         = {0x0C, 0x00}; // MV4_SS - Major Version 4 Sector Size
    const ut_BYTE dot_doc_MSS[2]            = {0x06, 0x00}; // MSS - Mini Stream Size
};

#include "dot_doc_beginning/dot_doc_file_header.hpp"

#endif