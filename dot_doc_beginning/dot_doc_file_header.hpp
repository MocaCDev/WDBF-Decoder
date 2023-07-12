#ifndef dot_doc_header
#define dot_doc_header

/* Lengths. */
#define WDBF_header_sig_length      0x08
#define WDBF_PL_header_sig          0x10 // PL - Padding Length; 16 bytes of padding following 8-byte header signature
#define WDBF_RPL_header_sig         0x06 // RPL - Reserved Padding Length; 6 bytes of padding following Mini Stream sector size 

/* Word Document Binary file heading structure. */
struct _dot_doc_header : public PD_WDBF_values
{
public:
    /* Values to be used. */
    const ut_WORD mv3 = merge_bytes<ut_WORD, ut_BYTE> (dot_doc_majr_vers_3[1], dot_doc_majr_vers_3[0]);
    const ut_WORD mv4 = merge_bytes<ut_WORD, ut_BYTE> (dot_doc_majr_vers_4[1], dot_doc_majr_vers_4[0]);

    ut_BYTE         header_sig[8];                  // D0 CF 11 E0 A1 B1 1A E1
    ut_BYTE         padding[16];                    // 00 * 16
    ut_WORD         CFB_minor_version;              // 3E 00
    ut_WORD         CFB_major_version;              // Major version 3 = 03 00, Major version 4 = 04 00
    ut_WORD         CFB_byte_order_indication;      // Word Document Binary files use little endian always; FE FF
    ut_WORD         CFB_sector_size;                // Major version 3 = 09 00 (512-byte sectors), Major version 4 = 0C 00 (4096-byte sectors)
    ut_WORD         CFB_mini_sector_size;           // Sector size of the Mini Stream as a power of 2 (64 bytes)
    ut_BYTE         reserved[6];
    ut_DWORD        CFB_number_of_dir_sectors;      // If Major Version is 3, this has to be zero
    ut_DWORD        CFB_number_of_FAT_sectors;      // Number of FAT sectors in the Compound File
    ut_DWORD        CFB_first_dir_sector_loc;       // Starting sector number for directory stream
    ut_DWORD        CFB_transaction_sig_number;
    ut_DWORD        CFB_mini_stream_cutoff_size;    // Must be 0x00001000
    ut_DWORD        CFB_first_minifat_sector_loc;   // Starting sector for mini FAT
    ut_DWORD        CFB_number_of_minifat_sectors;  
    ut_DWORD        CFB_first_DIFAT_sector_loc;
    ut_DWORD        CFB_number_of_DIFAT_sectors;

    ut_DWORD        *FAT_sector_locations;
    void allocate_FAT_sector_locations_memory(FileAPI& fapi)
    {

        /* If `CFB_major_version` is `03 00`, `FAT_sector_locations` will have a size of 436 (0x1B4) bytes.
         * Otherwise, if it is `04 00`, `FAT_sector_locations` will have a size of 3,584 (0xE00) bytes.
         * */
        if(CFB_major_version == mv3)
        {
            std::cout << "Major Version 3." << std::endl;
            
            /* Perform some checks. */
            if(CFB_number_of_dir_sectors != 0)
            {
                CFB_number_of_dir_sectors = 0;
                fapi.rewrite_specific <ut_DWORD> (data_locations::WDBF_NOD, CFB_number_of_dir_sectors);
            }

            //if()
            return;
        }

        std::cout << "Major Version 4." << std::endl;
        printf("%X, %X\n", CFB_major_version, mv4);
        return;
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

class DotDoc_Header : public PD_WDBF_values
{
private:
    
    FileAPI *fapi = nullptr;
    struct _dot_doc_header *WDBF_header = nullptr;

    /* DIT - Data Input Type.
     *      DIT will be WDBF_PD1 for the 16-bytes of padding after 8-byte header signature.
     *      DIT will be WDBF_PD2 for the 6-bytes of reserved padding after WDBF_MSS.
     * */
    void get_WDBF_padding_or_reserved(data_locations DIT)
    {
        switch(DIT)
        {
            case data_locations::WDBF_PD1: {
                memcpy(&WDBF_header->padding, fapi->FBWW_read_in<ut_BYTE> (WDBF_PL_header_sig), WDBF_PL_header_sig);
                return;
            }
            case data_locations::WDBF_PD2: {
                memcpy(&WDBF_header->reserved, fapi->FBWW_read_in<ut_BYTE> (WDBF_RPL_header_sig), WDBF_RPL_header_sig);
                return;
            }
            default: break;
        }
    }

    void get_WDBF_heading_sig()
    {
        memcpy(&WDBF_header->header_sig, fapi->FBWW_read_in<ut_BYTE> (WDBF_header_sig_length), WDBF_header_sig_length);
        
        /* Make sure all 8 bytes are correct for the header signature. */
        for(ut_BYTE i = 0; i < WDBF_header_sig_length; i++)
        {
            if(WDBF_header->header_sig[i] != dot_doc_header_sig[i])
            {
                if(!err_tracker->is_same_error(invalid_doc_file_header_sig))
                    dot_doc_raise_exception(invalid_doc_file_header_sig, true,
                        "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tAt index %d of the 8-byte header signature, the value encountered was \e[0;31m`0x%X`\e[0;37m when the program was expecting \e[0;33m`0x%X`\e[0;37m.\n",
                        i + 1, WDBF_header->header_sig[i], dot_doc_header_sig[i])
                else
                    dot_doc_raise_exception(invalid_doc_file_header_sig, true,
                        "\t\e[0;33m[WordDocument Binary Flaw]\e[0;37m\tException #%d: Value encountered at index %d was \e[0;31m`0x%X`\e[0;37m, expected \e[0;33m`0x%X`\e[0;37m.\n",
                            err_tracker->get_same_error_count() + 1,
                            i + 1, WDBF_header->header_sig[i], dot_doc_header_sig[i])

                WDBF_header->header_sig[i] = dot_doc_header_sig[i];
                fapi->rewrite<ut_BYTE> (WDBF_header->header_sig[i]);
            }
        }

        /* Following the header signature comes 16 bytes of padding. Go ahead and obtain those. */
        get_WDBF_padding_or_reserved(data_locations::WDBF_PD1);

        /* Make sure the padding is all zeros. */
        for(ut_BYTE i = 0; i < WDBF_PL_header_sig; i++)
        {
            if(WDBF_header->padding[i] != 0)
            {
                if(!err_tracker->is_same_error(invalid_doc_file_padding))
                    dot_doc_raise_exception(invalid_doc_file_padding, true,
                        "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tAt index %d of the 16 bytes of padding the value found was \e[0;31m`0x%X`\e[0;37m, when it is expected to be `0x00`.\n",
                        i + 1, WDBF_header->padding[i])
                else
                    dot_doc_raise_exception(invalid_doc_file_padding, true,
                        "\t\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tException #%d: Value encountered at index %d was \e[0;31m`0x%X`\e[0;37m, expected `0x00`.\n",
                        err_tracker->get_same_error_count() + 1,
                        i + 1, WDBF_header->padding[i])

                WDBF_header->padding[i] = 0x0;
                fapi->rewrite<ut_BYTE> (WDBF_header->padding[i]);
            }
        }
    }

    void get_WDBF_minor_and_major_version()
    {
        WDBF_header->CFB_minor_version = convert_endian<ut_WORD> (merge_bytes<ut_WORD, ut_BYTE> (fapi->FBWW_read_in<ut_BYTE> (1)[0], fapi->FBWW_read_in<ut_BYTE> (1)[0]));

        /* Make sure the minor version matches. */
        if(((WDBF_header->CFB_minor_version >> 8) & 0xFF) != dot_doc_req_minor_v[0] ||
           (WDBF_header->CFB_minor_version & 0xFF) != dot_doc_req_minor_v[1])
        {
            dot_doc_raise_exception(invalid_CFB_minor_version, true,
                "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tThe minor version was found to be \e[0;31m`0x%X`\e[0;37m, when it is expected to be \e[0;33m`0x3E00`\e[0;37m.\n",
                WDBF_header->CFB_minor_version)

            WDBF_header->CFB_minor_version ^= WDBF_header->CFB_minor_version;
            WDBF_header->CFB_minor_version |= convert_endian<ut_WORD> (merge_bytes<ut_WORD, ut_BYTE> (dot_doc_req_minor_v[0], dot_doc_req_minor_v[1]));
            fapi->rewrite<ut_WORD> (WDBF_header->CFB_minor_version);
        }

        WDBF_header->CFB_major_version = convert_endian<ut_WORD> (merge_bytes<ut_WORD, ut_BYTE> (fapi->FBWW_read_in<ut_BYTE> (1)[0], fapi->FBWW_read_in<ut_BYTE> (1)[0]));

        /* Make sure the major version is valid. */
        if((((WDBF_header->CFB_major_version >> 8) & 0xFF) != dot_doc_majr_vers_3[0] ||
           (WDBF_header->CFB_major_version & 0xFF) != dot_doc_majr_vers_3[1]) && 
           (((WDBF_header->CFB_major_version >> 8) & 0xFF) != dot_doc_majr_vers_4[0] ||
           (WDBF_header->CFB_major_version & 0xFF) != dot_doc_majr_vers_4[1]))
        {
            dot_doc_raise_exception(invalid_CFB_major_version, true,
                "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tThe major version was found to be \e[0;31m`0x%X`\e[0;37m, when it is expected to be \e[0;33m`0x0300`\e[0;37m or \e[0;33m`0x0400`\e[0;37m.\n",
                WDBF_header->CFB_major_version)

            WDBF_header->CFB_major_version ^= WDBF_header->CFB_major_version;
            WDBF_header->CFB_major_version |= convert_endian<ut_WORD> (merge_bytes<ut_WORD, ut_BYTE> (dot_doc_majr_vers_3[0], dot_doc_majr_vers_3[1]));
            fapi->rewrite<ut_WORD> (WDBF_header->CFB_major_version);
        }
    }

    void get_WDBF_byte_order()
    {
        WDBF_header->CFB_byte_order_indication = convert_endian<ut_WORD> (merge_bytes<ut_WORD, ut_BYTE> (fapi->FBWW_read_in<ut_BYTE> (1)[0], fapi->FBWW_read_in<ut_BYTE> (1)[0]));

        if(((WDBF_header->CFB_byte_order_indication >> 8) & 0xFF) != dot_doc_byte_order[0] ||
           (WDBF_header->CFB_byte_order_indication & 0xFF) != dot_doc_byte_order[1])
        {
            dot_doc_raise_exception(invalid_CFB_little_endian_indication, true,
                "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tThe byte order indication was found to be \e[0;31m`0x%X`\e[0;37m, when it is expected to be \e[0;33m`0xFEFF`\e[0;37m.\n",
                WDBF_header->CFB_minor_version)

            WDBF_header->CFB_byte_order_indication ^= WDBF_header->CFB_byte_order_indication;
            WDBF_header->CFB_byte_order_indication |= merge_bytes<ut_WORD, ut_BYTE> (dot_doc_byte_order[0], dot_doc_byte_order[1]);
            fapi->rewrite<ut_WORD> (WDBF_header->CFB_byte_order_indication);
        }
    }

    /* TODO: Is there a way to do the following without much repitition in the code being performed? */
    void get_WDBF_sector_size()
    {
        WDBF_header->CFB_sector_size = merge_bytes<ut_WORD, ut_BYTE> (fapi->FBWW_read_in<ut_BYTE> (1)[0], fapi->FBWW_read_in<ut_BYTE> (1)[0]);
        
        /* Depending on the major version, test if the value is correct.
         * Major Version 3 = 09 00 in binary.
         * Major Version 4 = 0C 00 in binary.
         * */
        if(WDBF_header->CFB_major_version == WDBF_header->mv3)
        {
            /* `CFB_sector_size` should only represent one byte. */
            if((WDBF_header->CFB_sector_size & 0xFF) != dot_doc_MV3_SS[0])
            {
                dot_doc_raise_exception(invalid_CFB_sector_size_indication, true,
                    "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tThe sector size indication was found to be \e[0;31m`0x%X`\e[0;37m, when it is expected to be \e[0;33m`0x%X`\e[0;37m.\n\t\t\t\tThe Major Version is \e[0;33m`0x%X` (%s)\e[0;37m, which requires sector size of 512 bytes.\n",
                        WDBF_header->CFB_sector_size, merge_bytes<ut_WORD, ut_BYTE> (dot_doc_MV3_SS[1], dot_doc_MV3_SS[0]), WDBF_header->CFB_major_version,
                        "Major Version 3")
                
                WDBF_header->CFB_sector_size ^= WDBF_header->CFB_sector_size;
                WDBF_header->CFB_sector_size |= merge_bytes<ut_WORD, ut_BYTE> (dot_doc_MV3_SS[1], dot_doc_MV3_SS[0]);
                fapi->rewrite<ut_WORD> (WDBF_header->CFB_sector_size);
            }
            return;
        }

        /* `CFB_sector_size` should only represent one byte. */
        if((WDBF_header->CFB_sector_size & 0xFF) != dot_doc_MV4_SS[0])
        {
            dot_doc_raise_exception(invalid_CFB_sector_size_indication, true,
                "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tThe sector size indication was found to be \e[0;31m`0x%X`\e[0;37m, when it is expected to be \e[0;33m`0x%X`\e[0;37m.\n\t\t\t\tThe Major Version is \e[0;33m`0x%X` (%s)\e[0;37m, which requires sector size of 512 bytes.\n",
                    WDBF_header->CFB_sector_size, merge_bytes<ut_WORD, ut_BYTE> (dot_doc_MV3_SS[1], dot_doc_MV4_SS[0]), WDBF_header->CFB_major_version,
                    "Major Version 4")
            
            WDBF_header->CFB_sector_size ^= WDBF_header->CFB_sector_size;
            WDBF_header->CFB_sector_size |= merge_bytes<ut_WORD, ut_BYTE> (dot_doc_MV4_SS[1], dot_doc_MV4_SS[0]);
            fapi->rewrite<ut_WORD> (WDBF_header->CFB_sector_size);
        }

        return;
    }

    void get_WDBF_mini_stream_size()
    {
        WDBF_header->CFB_mini_sector_size = merge_bytes<ut_WORD, ut_BYTE> (fapi->FBWW_read_in<ut_BYTE> (1)[0], fapi->FBWW_read_in<ut_BYTE> (1)[0]);

        /* Make sure `CFB_mini_sector_size` is `06 00`.
         * The size of the Mini Stream should always be represented by `06 00` in the binary file.
         * */
        if((WDBF_header->CFB_mini_sector_size & 0xFF) != dot_doc_MSS[0])
        {
            dot_doc_raise_exception(invalid_CFB_mini_stream_sector_size, true,
                "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tThe Mini Stream sector size was found to be \e[0;31m`0x%X`\e[0;37m, when it is expected to be \e[0;33m`0x0600`\e[0;37m.\n",
                    WDBF_header->CFB_mini_sector_size)
            
            WDBF_header->CFB_mini_sector_size ^= WDBF_header->CFB_mini_sector_size;
            WDBF_header->CFB_mini_sector_size |= merge_bytes<ut_WORD, ut_BYTE> (dot_doc_MSS[1], dot_doc_MSS[0]);
            fapi->rewrite<ut_WORD> (WDBF_header->CFB_mini_sector_size);
        }

        /* Go ahead and get the 6-bytes of reserved padding the follows the Mini Stream sector size. */
        get_WDBF_padding_or_reserved(data_locations::WDBF_PD2);

        /* Make sure the padding is all zeros. */
        for(ut_BYTE i = 0; i < WDBF_RPL_header_sig; i++)
        {
            if(WDBF_header->reserved[i] != 0)
            {
                if(!err_tracker->is_same_error(invalid_doc_file_padding))
                    dot_doc_raise_exception(invalid_doc_file_padding, true,
                        "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tAt index %d of the 6 bytes of reserved padding the value found was \e[0;31m`0x%X`\e[0;37m, when it is expected to be `0x00`.\n",
                        i + 1, WDBF_header->reserved[i])
                else
                    dot_doc_raise_exception(invalid_doc_file_padding, true,
                        "\t\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tException #%d: Value encountered at index %d was \e[0;31m`0x%X`\e[0;37m, expected `0x00`.\n",
                        err_tracker->get_same_error_count() + 1,
                        i + 1, WDBF_header->reserved[i])

                WDBF_header->reserved[i] = 0x0;
                fapi->rewrite<ut_BYTE> (WDBF_header->reserved[i]);
            }
        }
    }

    void get_WDBF_number_of_DIR_sectors()
    {
        WDBF_header->CFB_number_of_dir_sectors = merge_bytes<ut_DWORD, ut_WORD> (fapi->FBWW_read_in<ut_WORD> (1)[0], fapi->FBWW_read_in<ut_WORD> (1)[0]);

        /* If `CFB_major_version` represents Major Version 3, this value must be zero. */
        if(WDBF_header->CFB_major_version == WDBF_header->mv3 && WDBF_header->CFB_number_of_dir_sectors != 0)
        {
            dot_doc_raise_exception(invalid_CFB_dir_sector_count, true,
                "\e[1;93m[WordDocument Binary Flaw]\e[0;37m\tThe number of Directory sectors was found to be \e[0;31m`0x%X`\e[0;37m, when it is expected to be \e[0;33m`0x00`\e[0;37m.\n\t\t\t\tWith the WDBF using Major Version 3, this value must be zero.\n",
                    WDBF_header->CFB_number_of_dir_sectors)
            
            WDBF_header->CFB_number_of_dir_sectors ^= WDBF_header->CFB_number_of_dir_sectors;
            fapi->rewrite<ut_DWORD> (WDBF_header->CFB_number_of_dir_sectors);
        }
    }

    void get_WDBF_number_of_FAT_sectors()
    {
        WDBF_header->CFB_number_of_FAT_sectors = merge_bytes<ut_DWORD, ut_WORD> (fapi->FBWW_read_in<ut_WORD> (1)[0], fapi->FBWW_read_in<ut_WORD> (1)[0]);
    }

    void get_WDBF_first_directory_sector_location()
    {
        ut_BYTE i = 1;
        ut_BYTE byte_read = fapi->FBWW_read_in<ut_BYTE> (1)[0];
        WDBF_header->CFB_first_dir_sector_loc |= byte_read;

        while(i < sizeof(ut_DWORD))
        {
            byte_read = fapi->FBWW_read_in<ut_BYTE> (1)[0];
            WDBF_header->CFB_first_dir_sector_loc = (WDBF_header->CFB_first_dir_sector_loc << 8) | byte_read;

            i++;
        }
        WDBF_header->CFB_first_dir_sector_loc = convert_endian<ut_DWORD> (WDBF_header->CFB_first_dir_sector_loc);
    }

    void get_WDBF_transaction_sig_number()
    {
        WDBF_header->CFB_transaction_sig_number = fapi->FBWW_read_in<ut_DWORD> (1)[0];
    }

    void get_WDBF_mini_stream_cutoff_size()
    {
        ut_BYTE i = 1;
        ut_BYTE byte_read = fapi->FBWW_read_in<ut_BYTE> (1)[0];
        WDBF_header->CFB_mini_stream_cutoff_size |= byte_read;

        while(i < sizeof(ut_DWORD))
        {
            byte_read = fapi->FBWW_read_in<ut_BYTE> (1)[0];
            WDBF_header->CFB_mini_stream_cutoff_size = (WDBF_header->CFB_mini_stream_cutoff_size << 8) | byte_read;

            i++;
        }
        WDBF_header->CFB_mini_stream_cutoff_size = convert_endian<ut_DWORD> (WDBF_header->CFB_mini_stream_cutoff_size);
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
        get_WDBF_byte_order();
        get_WDBF_sector_size();
        get_WDBF_mini_stream_size();
        get_WDBF_number_of_DIR_sectors();
        get_WDBF_number_of_FAT_sectors();
        get_WDBF_first_directory_sector_location();
        get_WDBF_transaction_sig_number();
        get_WDBF_mini_stream_cutoff_size();
        get_WDBF_first_minifat_sector_loc();

        /* Debug printing to see all the data. */
        std::cout << "\n\e[0;32m[DEBUG ➟ \e[1;35mWDBF_header->header_sig\e[0;32m]\e[0;37m WDBF Heading Signature: ";
        for(ut_BYTE i = 0; i < 8; i++)
        {
            if((i + 1) == 8) printf("%X\n", WDBF_header->header_sig[i]);
            else printf("%X, ", WDBF_header->header_sig[i]);
        }

        std::cout << "\e[0;32m[DEBUG ➟ \e[1;35mWDBF_header->CFB_minor_version\e[0;32m]\e[0;37m WDBF Minor Version: ";
        printf("0x%X\n", WDBF_header->CFB_minor_version);
        std::cout << "\e[0;32m[DEBUG ➟ \e[1;35mWDBF_header->CFB_major_version\e[0;32m]\e[0;37m WDBF Major Version: ";
        printf("0x%X\n", WDBF_header->CFB_major_version);

        std::cout << "\e[0;32m[DEBUG ➟ \e[1;35mWDBF_header->CFB_byte_order_indication\e[0;32m]\e[0;37m WDBF Byte Order Indication: ";
        printf("0x%X (%s)\n", WDBF_header->CFB_byte_order_indication, "Little Endian");

        std::cout << "\e[0;32m[DEBUG ➟ \e[1;35mWDBF_header->CFB_sector_size\e[0;32m]\e[0;37m WDBF Sector Size Indication: ";
        printf("0x%X (%s)\n", WDBF_header->CFB_sector_size, (WDBF_header->CFB_sector_size & 0xFF) == dot_doc_MV3_SS[0] ? "Major Version 3 Sector Size Of 512 Bytes" : "Major Version 4 Sector Size Of 4,096 Bytes");

        std::cout << "\e[0;32m[DEBUG ➟ \e[1;35mWDBF_header->CFB_mini_sector_size\e[0;32m]\e[0;37m WDBF Mini Sector Size Indication: ";
        printf("0x%X\n", WDBF_header->CFB_mini_sector_size);

        std::cout << "\e[0;32m[DEBUG ➟ \e[1;35mWDBF_header->CFB_number_of_dir_sectors\e[0;32m]\e[0;37m WDBF Number Of Directory Sectors: ";
        printf("0x%X%s\n", WDBF_header->CFB_number_of_dir_sectors, WDBF_header->CFB_number_of_dir_sectors == 0 ? " (Zero Due To Major Version 3)" : "\0");

        std::cout << "\e[0;32m[DEBUG ➟ \e[1;35mWDBF_header->CFB_number_of_FAT_sectors\e[0;32m]\e[0;37m WDBF Number Of FAT Sectors: ";
        printf("0x%X\n", WDBF_header->CFB_number_of_FAT_sectors);

        std::cout << "\e[0;32m[DEBUG ➟ \e[1;35mWDBF_header->CFB_first_dir_sector_loc\e[0;32m]\e[0;37m WDBF First Directory Sector Location: ";
        printf("0x%X\n", WDBF_header->CFB_first_dir_sector_loc);

        std::cout << "\e[0;32m[DEBUG ➟ \e[1;35mWDBF_header->CFB_transaction_sig_number\e[0;32m]\e[0;37m WDBF Transaction Sig Number: ";
        printf("%X%s\n", WDBF_header->CFB_transaction_sig_number, WDBF_header->CFB_transaction_sig_number == 0 ? " (File Transactions Not Enabled)" : " (File Transactions Enabled)");

        std::cout << "\e[0;32m[DEBUG ➟ \e[1;35mWDBF_header->CFB_mini_stream_cutoff_size\e[0;32m]\e[0;37m WDBF Mini Stream Cutoff Size: ";
        printf("%X\n", WDBF_header->CFB_mini_stream_cutoff_size);



        WDBF_header->allocate_FAT_sector_locations_memory(*fapi);
    }

    void delete_instance(DotDoc_Header *dheader)
    {
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