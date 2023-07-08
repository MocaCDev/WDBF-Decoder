#ifndef dot_doc_file_api
#define dot_doc_file_api

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

    /* `T` can be `ut_BYTE`, `ut_WORD` or `ut_DWORD`.
     * `bytes` will be multiplied by the size of the variable `bytes`.
     * So, with `FBWW_read_in<ut_WORD> (4)`, the function will read in four 2-byte values (8 bytes in total).
     * */
    template<typename T>
    ut_BYTE *FBWW_read_in(T bytes)
    {
        if(read_in_data) delete read_in_data;

        read_in_data = new ut_BYTE[bytes * sizeof(bytes)];

        seek_pos += fread(read_in_data, sizeof(bytes), bytes, FBWW);

        return read_in_data;
    }

    /* If there was any error with the data, manipulate the values where the reside via
     * `all_file_data`.
     * */
    template<typename T>
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