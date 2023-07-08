#include <iostream>
#include "common.hpp"

int main(int args, char *argv[])
{
    dot_doc_assert(args > 1, "\n%sArgument Error:%s\n\tExpected file as input.\n",
        red, white)
    
    /* Make sure the file starts with a ASCII-based value. */
    dot_doc_assert(is_ascii(argv[1][0]), "\n%sArgument Error:%s\n\tThe argument needs to start with an ASCII-based value. Got `%c`.\n",
        red, white,
        argv[1][0])
    
    /* WDBFH - Word Document Binary Format Heading. */
    DotDoc_Header *WDBFH = new DotDoc_Header(ut_BYTE_PTR argv[1]);
    WDBFH->gather_WDBF_heading();

    WDBFH->delete_instance(WDBFH);//delete WDBFH;

    return 0;
}