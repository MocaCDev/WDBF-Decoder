#ifndef dot_doc_file_api
#define dot_doc_file_api

/* Locations for everything in the file. */
enum class data_locations: ut_LSIZE
{
    /* HS - Header Signature. */
    WDBF_HS     = 0x0,
    /* PD1 - Padding 1. */
    WDBF_PD1    = 0x8,
    /* MV - Minor Version. */
    WDBF_MV     = 0x18,
    /* MJV - Major Version. */
    WDBF_MJV    = 0x1A,
    /* BO - Byte Order. */
    WDBF_BO     = 0x1C,
    /* SS - Sector Size. */
    WDBF_SS     = 0x1E,
    /* MSS - Mini Sector Size (Mini Stream Size). */
    WDBF_MSS    = 0x20,
    /* PD2 - Padding 2 (reserved 6 bytes). */
    WDBF_PD2    = 0x22,
    /* NOD - Number Of Directories. */
    WDBF_NOD    = 0x28,
    /* NOFS - Number Of Fat Sectors. */
    WDBF_NOFS   = 0x2C,
    /* FDSL - First Directory Sector Location. */
    WDBF_FDSL   = 0x30,
    /* TSN - Transaction Signature Number. */
    WDBF_TSN    = 0x34,
    /* MSCS - Mini Stream Cutoff Size. */
    WDBF_MSCS   = 0x38,
    /* FMFSL - First Mini Fat Sector Location. */
    WDBF_FMFSL  = 0x3C,
    /* NOMFS - Number Of Mini Fat Sectors. */
    WDBF_NOMFS  = 0x40,
    /* FDFSL - First DIFAT Sector Location. */
    WDBF_FDFSL  = 0x44,
    /* NODFS - Number Of DIFAT Sectors. */
    WDBF_NODFS  = 0x48,
    /* FSL - Fat Sector Locations. */
    WDBF_FSL    = 0x4C
};

template<typename T>
concept BYTE_WORD_DWORD = requires {
    std::is_same<T, ut_BYTE>::value ||
        std::is_same<T, ut_WORD>::value ||
        std::is_same<T, ut_DWORD>::value;
};

/* FileAPI - class that extensively works with reading from/writing to a file.
 *           This class will be capable of transitioning from reading a file to writing to the file
 *           when needed. When initiated, the file passed to the constructor `FileAPI` will be opened in Read Binary (rb) mode.
 * 
 * Variables:
 *      FILE *FBWW - File Being Worked With; the file being read from/written to.
 *      ut_LSIZE seek_pos - The current position in the file. This gets set anytime we read from the file,
 *                          write to the file, or use `fseek`.
 *      _dot_doc_header *WDBF_header - Structure representing the entire header of the Word Document Binary File (WDBF).
 *
 */
class FileAPI
{
private:
    FILE *FBWW = NULL;
    ut_LSIZE seek_pos = 0;
    ut_BYTE *read_in_data = nullptr;
    ut_BYTE *all_file_data = nullptr;
    ut_LSIZE WDBF_size = 0;

public:
    FileAPI(ut_BYTE *filename)
    {
        FBWW = fopen(nt_BYTE_CPTR filename, "rb");

        dot_doc_assert(FBWW, "\n%sFile Error:%s\n\tThere was an error opening the file `%s`.\n",
            red, white,
            filename)
        
        /* Get filesize. */
        fseek(FBWW, 0, SEEK_END);
        WDBF_size = ftell(FBWW);
        fseek(FBWW, 0, SEEK_SET);

        all_file_data = new ut_BYTE[WDBF_size];
        dot_doc_assert(all_file_data, "\n%sMemory Allocation Error:%s\n\tThere was an error allocating memory for file data.\n",
            red, white)
        
        seek_pos = fread(all_file_data, sizeof(*all_file_data), WDBF_size, FBWW);
        dot_doc_assert(seek_pos == WDBF_size, "\n%sRead Error:%s\n\tThere was an error reading the file.\n",
            red, white)

        /* Go back to the beginning. */
        seek_pos = 0;
        fseek(FBWW, seek_pos, SEEK_SET);
    }

    void print_seek_pos()
    { printf("Seek Pos: %llX\n", seek_pos); }

    /* `T` can be `ut_BYTE`, `ut_WORD` or `ut_DWORD`.
     * `bytes` will be multiplied by the size of the variable `bytes`.
     * So, with `FBWW_read_in<ut_WORD> (4)`, the function will read in four 2-byte values (8 bytes in total).
     * */
    template<typename T>
        requires BYTE_WORD_DWORD<T>
    ut_BYTE *FBWW_read_in(T bytes)
    {
        if(read_in_data) delete read_in_data;

        read_in_data = new ut_BYTE[bytes * sizeof(bytes)];

        /* Move the position, then make sure it does not exceed the size of the binary file. */
        seek_pos += bytes * sizeof(bytes);
        dot_doc_assert(seek_pos <= WDBF_size,
            "\n\t%sRead Error:%s\n\tThe WDBF is only %llX (%lld) bytes in size, the program is attempting to read %lld bytes beyond the size of the file.\n\tPerhaps try replacing the file.\n",
            red, white,
            WDBF_size, WDBF_size,
            seek_pos - WDBF_size)
        
        ut_LSIZE read_in_size = fread(read_in_data, sizeof(bytes), bytes, FBWW);
        
        /* Make sure we read in the correct amount of bytes. */
        dot_doc_assert(read_in_size == bytes,
            "\n\t%sRead Error:%s\n\tExpected to read in %lX (%ld) bytes, instead read in only %llX (%lld) bytes.\n",
            red, white,
            bytes * sizeof(bytes), bytes * sizeof(bytes),
            read_in_size, read_in_size)

        return read_in_data;
    }

    /* Seek beyond the current position.
     * This is especially useful when the program is obtaining a 4-byte value and the last
     * two bytes are zero.
     * */
    void FBWW_manual_seek(nt_LSIZE seek_length)
    { seek_pos += seek_length; }

    /* Seek forward and check a value.
     * As stated above, this will be useful when reading in 4-byte values and we want to check
     * and see if values are zero.
     * 
     * After seeking forward, it will check the value against `VTTA` (Value To Test Against),
     * if it matches and `move_if_matches` is true then the function will move forward otherwise it
     * will keep the seek position where it is at.
     * */
    template<typename T>
        requires BYTE_WORD_DWORD<T>
    bool FBWW_seek_and_check(ut_LSIZE seek_length, bool move_if_matches, T VTTA)
    {
        seek_pos += seek_length;

        switch(sizeof(VTTA))
        {
            case 1: {
                if(VTTA == all_file_data[seek_pos])
                { 
                    if(!move_if_matches) FBWW_manual_seek(seek_length * -1);
                    return true;
                }

                /* If we are here, the value did not match. Go back. */
                seek_pos -= seek_length;
                break;
            }
            case 2: {
                break;
            }
            case 4: {
                break;
            }
            default: break;
        }

        return false;
    }

    /* If there was any error with the data, manipulate the values where the reside via
     * `all_file_data`.
     * */
    template<typename T>
        requires BYTE_WORD_DWORD<T>
    void rewrite(T value)
    {
        seek_pos -= sizeof(value);

        switch(sizeof(value))
        {
            case 2: {
                all_file_data[seek_pos] = value & 0xFF;
                seek_pos++;

                all_file_data[seek_pos] = (value >> 8) & 0xFF;
                seek_pos++;
                break;
            }
            case 4: {
                all_file_data[seek_pos] = value & 0xFF;
                seek_pos++;

                all_file_data[seek_pos] = (value >> 8) & 0xFF;
                seek_pos++;

                all_file_data[seek_pos] = (value >> 16) & 0xFF;
                seek_pos++;
                
                all_file_data[seek_pos] = (value >> 24) & 0xFF;
                seek_pos++;
                
                break;
            }
            default: break;
        }
    }

    /* Rewrite data at a specific location in the data. */
    template<typename T>
        requires BYTE_WORD_DWORD<T>
    void rewrite_specific(data_locations location, T value)
    {
        ut_LSIZE curr_pos = seek_pos;
        seek_pos = (ut_LSIZE) location;
        rewrite<T> (value);
        seek_pos = curr_pos;
    }

    ~FileAPI()
    {
        if(FBWW) fclose(FBWW);
        if(read_in_data) delete read_in_data;
        if(all_file_data) delete all_file_data;

        read_in_data = nullptr;
        all_file_data = nullptr;

        FBWW = NULL;
    }
};

#endif