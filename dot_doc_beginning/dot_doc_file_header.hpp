#ifndef dot_doc_header
#define dot_doc_header

/* Lengths. */
#define WDBF_header_sig_length      0x08
#define WDBF_PL_header_sig          0x10 // PL - Padding Length; 16 bytes of padding following 8-byte header signature

/* Word Document Binary file heading structure. */
struct _dot_doc_header
{
    ut_BYTE         header_sig[8];                  // D0 CF 11 E0 A1 B1 1A E1
    ut_BYTE         padding[16];                    // 00 * 16
    ut_WORD         CFB_minor_version;              // 3E 00
    ut_WORD         CFB_major_version;              // Major version 3 = 03 00, Major version 4 = 04 00
    ut_WORD         CFB_byte_order_indication;      // Word Document Binary files use little endian always; FE FF
    ut_WORD         CFB_sector_size;                // Major version 3 = 09 00 (512-byte sectors), Major version 4 = 0C 00 (4096-byte sectors)
    ut_WORD         CFB_mini_sector_size;           // Sector size of the Mini Stream as a power of 2 (64 bytes)
    ut_BYTE         reserved[6];
    ut_DWORD        CFB_number_of_dirs;             // If Major Version is 3, this has to be zero
    ut_DWORD        CFB_number_of_FAT_sectors;      // Number of FAT sectors in the Compound File
    ut_DWORD        CFB_first_dir_sector_loc;       // Starting sector number for directory stream
    ut_DWORD        CFB_transaction_sig_number;
    ut_DWORD        CFB_mini_stream_cutoff_size;    // Must be 0x00001000
    ut_DWORD        CFB_first_minifat_sector_loc;   // Starting sector for mini FAT
    ut_DWORD        CFB_number_of_minifat_sectors;  
    ut_DWORD        CFB_first_DIFAT_sector_loc;
    ut_DWORD        CFB_number_of_DIFAT_sectors;

    ut_DWORD        *FAT_sector_locations;
    void allocate_FAT_sector_locations_memory()
    {
        /*  */
    }

    _dot_doc_header()
    {
        memset(header_sig, 0, WDBF_header_sig_length);

        /* Set all elements to one to make sure the padding in the WDBF is represented by zeroes. */
        memset(padding, 1, WDBF_PL_header_sig);

        CFB_minor_version = CFB_major_version = CFB_byte_order_indication = CFB_sector_size = 0;
        FAT_sector_locations = nullptr;
    }

    ~_dot_doc_header()
    {
        if(FAT_sector_locations) delete FAT_sector_locations;

        FAT_sector_locations = nullptr;
    }
};

class DotDoc_Header
{
private:
    const ut_BYTE dot_doc_header_sig[8]     = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
    const ut_BYTE dot_doc_req_minor_v[2]    = {0x3E, 0x00};
    const ut_BYTE dot_doc_majr_vers_3[2]    = {0x03, 0x00};
    const ut_BYTE dot_doc_majr_vers_4[2]    = {0x04, 0x00};
    const ut_BYTE dot_doc_byte_order[2]     = {0xFE, 0xFF};
    
    FileAPI *fapi = nullptr;
    struct _dot_doc_header *WDBF_header = nullptr;

    void get_WDBF_heading_sig()
    {
        memcpy(&WDBF_header->header_sig, fapi->FBWW_read_in<ut_BYTE> (WDBF_header_sig_length), WDBF_header_sig_length);
        
        /* Following the header signature comes 16 bytes of padding. Go ahead and obtain those. */
        memcpy(&WDBF_header->padding, fapi->FBWW_read_in<ut_BYTE> (WDBF_PL_header_sig), WDBF_PL_header_sig);
    }

    void get_WDBF_minor_and_major_version()
    {
        WDBF_header->CFB_minor_version = little_endian<ut_WORD> (merge_bytes<ut_WORD, ut_BYTE> (fapi->FBWW_read_in<ut_BYTE> (1)[0], fapi->FBWW_read_in<ut_BYTE> (1)[0]));
    
        /* Make sure the minor version matches. */
        if(((WDBF_header->CFB_minor_version >> 8) & 0xFF) != dot_doc_req_minor_v[0] ||
           (WDBF_header->CFB_minor_version & 0xFF) != dot_doc_req_minor_v[1])
        {
            sprintf(nt_BYTE_PTR err_tracker->error_msg, nt_BYTE_CPTR "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tThe minor version was found to be \e[0;31m`0x%X`\e[0;37m, when it is expected to be `0x3E00`.\n\t\t\t\tThe program will fix this automatically.\n",
                WDBF_header->CFB_minor_version);
            err_tracker->raise_error(invalid_CFB_minor_version, true);

            WDBF_header->CFB_minor_version ^= WDBF_header->CFB_minor_version;
            WDBF_header->CFB_minor_version |= merge_bytes<ut_WORD, ut_BYTE> (dot_doc_req_minor_v[0], dot_doc_req_minor_v[1]);
            fapi->rewrite<ut_WORD> (WDBF_header->CFB_minor_version);
        }

        WDBF_header->CFB_major_version = little_endian<ut_WORD> (merge_bytes<ut_WORD, ut_BYTE> (fapi->FBWW_read_in<ut_BYTE> (1)[0], fapi->FBWW_read_in<ut_BYTE> (1)[0]));

        /* Make sure the major version is valid. */
        if((((WDBF_header->CFB_major_version >> 8) & 0xFF) != dot_doc_majr_vers_3[0] ||
           (WDBF_header->CFB_major_version & 0xFF) != dot_doc_majr_vers_3[1]) && 
           (((WDBF_header->CFB_major_version >> 8) & 0xFF) != dot_doc_majr_vers_4[0] ||
           (WDBF_header->CFB_major_version & 0xFF) != dot_doc_majr_vers_4[1]))
        {
            sprintf(nt_BYTE_PTR err_tracker->error_msg, nt_BYTE_CPTR "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tThe major version was found to be \e[0;31m`0x%X`\e[0;37m, when it is expected to be `0x0300` or `0x0400`.\n\t\t\t\tThe program will fix this automatically.\n",
                WDBF_header->CFB_minor_version);
            err_tracker->raise_error(invalid_CFB_major_version, true);

            WDBF_header->CFB_major_version ^= WDBF_header->CFB_major_version;
            WDBF_header->CFB_major_version |= merge_bytes<ut_WORD, ut_BYTE> (dot_doc_majr_vers_3[0], dot_doc_majr_vers_3[1]);
            fapi->rewrite<ut_WORD> (WDBF_header->CFB_major_version);
        }
    }

    void get_WDBF_byte_order()
    {
        WDBF_header->CFB_byte_order_indication = little_endian<ut_WORD> (merge_bytes<ut_WORD, ut_BYTE> (fapi->FBWW_read_in<ut_BYTE> (1)[0], fapi->FBWW_read_in<ut_BYTE> (1)[0]));

        if(((WDBF_header->CFB_byte_order_indication >> 8) & 0xFF) != dot_doc_byte_order[0] ||
           (WDBF_header->CFB_byte_order_indication & 0xFF) != dot_doc_byte_order[1])
        {
            sprintf(nt_BYTE_PTR err_tracker->error_msg, nt_BYTE_CPTR "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tThe byte order indication was found to be \e[0;31m`0x%X`\e[0;37m, when it is expected to be `0xFEFF`.\n\t\t\t\tThe program will fix this automatically.\n",
                WDBF_header->CFB_minor_version);
            err_tracker->raise_error(invalid_CFB_little_endian_indication, true);

            WDBF_header->CFB_byte_order_indication ^= WDBF_header->CFB_byte_order_indication;
            WDBF_header->CFB_byte_order_indication |= merge_bytes<ut_WORD, ut_BYTE> (dot_doc_byte_order[0], dot_doc_byte_order[1]);
            fapi->rewrite<ut_WORD> (WDBF_header->CFB_byte_order_indication);
        }
    }

public:
    DotDoc_Header(ut_BYTE *filename)
    {
        fapi = new FileAPI(filename);
        WDBF_header = new struct _dot_doc_header;

        dot_doc_assert(fapi && WDBF_header, "\n%sMemory Allocation Error:%s\n\tThere was an error initializing memory for an internal variable.\n",
            red, white)
    }

    void gather_WDBF_heading()
    {
        get_WDBF_heading_sig();
        get_WDBF_minor_and_major_version();
    }

    void delete_instance(DotDoc_Header *dheader)
    {
        if(dheader->fapi) delete dheader->fapi;

        dheader->fapi = nullptr;
        delete dheader;
    }

    ~DotDoc_Header()
    {
        if(fapi) delete fapi;
        if(WDBF_header) delete WDBF_header;
        if(err_tracker) delete err_tracker;

        fapi = nullptr;
        WDBF_header = nullptr;
        err_tracker = nullptr;

        /* Debugging. */
        std::cout << "\e[0;32m[DEBUG ➟ \e[0;34mclass \e[1;35mFileAPI\e[0;32m]\e[0;37m\t\t`fapi` instance released." << std::endl;
        std::cout << "\e[0;32m[DEBUG ➟ \e[0;34mclass \e[1;35mDotDoc_Header\e[0;32m]\e[0;37m\t`DotDoc_Header` instance released." << std::endl;
    }
};

#endif