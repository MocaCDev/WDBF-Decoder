#ifndef dot_doc_cerror // cerror = custom error
#define dot_doc_cerror

enum error_type
{
    invalid_doc_file_header_sig = 0xE0,
    invalid_doc_file_padding = 0xE2,
    invalid_CFB_minor_version = 0xE4,
    invalid_CFB_major_version = 0xE6,
    invalid_CFB_little_endian_indication = 0xE8,
    invalid_CFB_sector_size_indication = 0xEA,
    no_error = 0x0
};

struct error_tracker
{
    enum error_type         current_error;
    enum error_type         previous_error;

    ut_BYTE                 same_error_count;
    ut_WORD                 error_count;

    bool                    all_errors_fixed;

    ut_BYTE                 error_msg[400];

    error_tracker()
    {
        current_error = previous_error = no_error;
        same_error_count = error_count = 0;
        all_errors_fixed = true;
        memset(error_msg, 0, 400);
    }

    ut_BYTE get_same_error_count()
    { return same_error_count; }

    ut_WORD get_error_count()
    { return error_count; }

    bool is_same_error(enum error_type error_to_be_raised)
    {
        if(error_count == 0)
            return false;
        
        if(previous_error == error_to_be_raised || current_error == error_to_be_raised)
            return true;
        
        return false;
    }

    ut_BYTE *get_error_name(enum error_type error_to_be_raised)
    {
        switch(error_to_be_raised)
        {
            case invalid_doc_file_header_sig: return ut_BYTE_PTR "Invalid Header Signature";break;
            case invalid_doc_file_padding: return ut_BYTE_PTR "Invalid Padding";break;
            case invalid_CFB_minor_version: return ut_BYTE_PTR "Invalid CFB Minor Version";break;
            case invalid_CFB_major_version: return ut_BYTE_PTR "Invalid CFB Major Version";break;
            case invalid_CFB_little_endian_indication: return ut_BYTE_PTR "Invalid Little-Endian Indicator";break;
            case invalid_CFB_sector_size_indication: return ut_BYTE_PTR "Invalid CFB Sector Size Indication";break;
            default: break;
        }

        return ut_BYTE_PTR "Unknown Error";
    }

    void raise_error(enum error_type error_encountered, bool can_fix)
    {
        /* If `err_tracker.current_error` does not represent `no_error`, assign `err_tracker.previous_error` to whatever the last error was
         * before `encountered_error` got called again.
         * */
        if(current_error != no_error)
            previous_error = current_error;
        
        if(is_same_error(error_encountered))
            goto same_error;

        /* Check to see if `err_tracker.err_count` is greater than zero. If it is, check to see if the error being encountered
         * is the same as the error that was encountered prior.
         * */
        if(error_count >= 1 && previous_error == error_encountered)
        {
            same_error:
            /* Increase the amount of same errors. */
            same_error_count++;

            /* Make sure `err_msg` is not NULL, then print. */
            dot_doc_assert(error_msg, "\n\e[0;31m[PROGRAM MEMORY ERROR]\e[0;37m\tInternal program error.\n")
            printf("%s", error_msg);

            goto encountered_error_end;
        }

        printf("\n");
    
        current_error = error_encountered;
        dot_doc_assert(error_msg, "Internal program error.\n")
        printf("%s", error_msg);

        same_error_count = 1;

        encountered_error_end:
        if(!can_fix) all_errors_fixed = false;

        error_count++;
        memset(error_msg, 0, 400);
    }

    void delete_instance(struct error_tracker *err_t)
    {
        delete err_t;
    }

    ~error_tracker()
    {
        /* Make sure all errors, if any, were fixed. */
        dot_doc_assert(all_errors_fixed, "\n\e[0;31m[WDBF INTERNAL ERROR]\e[0;37m\tThere is an error inside the WDBF that could not be fixed.\n")

        /* Debugging. */
        std::cout << "\n\e[0;32m[DEBUG ➟ \e[0;34mstruct \e[1;35merror_tracker\e[0;32m]\e[0;37m\t`err_tracker` released." << std::endl;
    }
};

static struct error_tracker *err_tracker = new struct error_tracker;

#endif