#ifndef dot_doc_decoder_common
#define dot_doc_decoder_common
#include <iostream>
#include <cstring>
#include <limits>

extern "C"
{
    typedef char				nt_BYTE;
    typedef unsigned char		ut_BYTE;
    typedef signed char			st_BYTE;
    #define nt_BYTE_PTR			(nt_BYTE *)
    #define nt_BYTE_CPTR		(const nt_BYTE *)
    #define ut_BYTE_PTR			(ut_BYTE *)
    #define ut_BYTE_CPTR		(const ut_BYTE *)
    #define st_BYTE_PTR			(st_BYTE *)
    #define st_BYTE_CPTR		(const st_BYTE *)

    typedef short				nt_WORD;
    typedef unsigned short		ut_WORD;
    typedef signed short		st_WORD;
    #define nt_WORD_PTR     	(nt_WORD *)
    #define nt_WORD_CPTR    	(const nt_WORD *)
    #define ut_WORD_PTR			(ut_WORD *)
    #define ut_WORD_CPTR		(const ut_WORD *)
    #define st_WORD_PTR			(st_WORD *)
    #define st_WORD_CPTR		(const st_WORD *)

    typedef int					nt_DWORD;
    typedef unsigned int		ut_DWORD;
    typedef signed int			st_DWORD;
    #define nt_DWORD_PTR		(nt_DWORD *)
    #define nt_DWORD_CPTR		(const nt_DWORD *)
    #define ut_DWORD_PTR		(ut_DWORD *)
    #define ut_DWORD_CPTR		(const ut_DWORD *)
    #define st_DWORD_PTR		(st_DWORD *)
    #define st_DWORD_CPTR		(const st_DWORD *)

    typedef long				nt_LBYTE;
    typedef unsigned long		ut_LBYTE;
    typedef signed long			st_LBYTE;
    #define nt_LBYTE_PTR		(nt_LBYTE *)
    #define nt_LBYTE_CPTR		(const nt_LBYTE *)
    #define ut_LBYTE_PTR		(ut_LBYTE *)
    #define ut_LBYTE_CPTR		(const ut_LBYTE *)
    #define st_LBYTE_PTR		(st_LBYTE *)
    #define st_LBYTE_CPTR		(const st_LBYTE *)

    typedef long long			nt_LLBYTE;
    typedef unsigned long long	ut_LLBYTE;
    typedef signed long long	st_LLBYTE;
    #define nt_LLBYTE_PTR		(nt_LLBYTE *)
    #define nt_LLBYTE_CPTR		(const nt_LLBYTE *)
    #define ut_LLBYTE_PTR		(ut_LLBYTE *)
    #define ut_LLBYTE_CPTR		(const ut_LLBYTE *)
    #define st_LLBYTE_PTR		(st_LLBYTE *)
    #define st_LLBYTE_CPTR		(const st_LLBYTE *)

    typedef ut_LLBYTE			ut_LSIZE;
    #define ut_LSIZE_PTR		(ut_LSIZE *)
    #define ut_LSIZE_CPTR		(const ut_LSIZE *)

    #ifndef __cplusplus
    typedef ut_BYTE				bool;
    #endif

    #ifndef true
    #define true				1
    #endif

    #ifndef false
    #define false				0
    #endif
}

/* Colors for printing. */
#define red					ut_BYTE_CPTR "\e[0;31m"
#define yellow				ut_BYTE_CPTR "\e[0;33m"
#define green				ut_BYTE_CPTR "\e[0;32m"
#define bgreen				ut_BYTE_CPTR "\e[1;32m"
#define white				ut_BYTE_CPTR "\e[0;37m"
#define reset				white

/* Custom asserts/printing.
 * `dot_doc_assert_NERR` means assert, but warn rather than error and exit.
 * */
#define dot_doc_error(msg, ...)			\
{						\
	fprintf(stderr, msg, ##__VA_ARGS__);	\
	exit(EXIT_FAILURE);			\
}
#define dot_doc_warning(msg, ...)       \
{                                       \
    fprintf(stderr, msg, ##__VA_ARGS__);\
}
#define dot_doc_assert(cond, msg, ...)		\
	if(!(cond)) 				\
		dot_doc_error(msg, ##__VA_ARGS__)
#define dot_doc_assert_NERR(cond, msg, ...) \
    if(!(cond))                             \
        dot_doc_warning(msg, ##__VA_ARGS__)

/* Checks for ASCII, number, ASCII with exception and number with exception.
 * is_ascii_WE - WE stands for With Exception.
 * is_number_WE - WE stand for With Exception.
 * */
#define is_ascii(val)   (val >= 'a' && val <= 'z') || (val >= 'A' && val <= 'Z') ? true : false
#define is_ascii_WE(val, exc) (is_ascii(val)) || val == exc ? true : false

/* "Merge" 2 bytes into a short, or 4 bytes into a integer.
 * RT is the type to be returned (Return Type).
 * IT is the type of bytes passed to `byte1` and `byte2`.
 * I am the only one that will ever use this function, so I don't care too much about safety.
 * */
template<typename RT, typename IT>
RT merge_bytes(IT byte1, IT byte2)
{
    RT ret_value = 0;

    switch(sizeof(ret_value))
    {
        case 2: {
            ret_value |= byte1 & 0xFF;
            ret_value = (ret_value << 8) | (byte2 & 0xFF);
            break;
        }
        case 4: {
            ret_value |= byte1;
            ret_value = (ret_value << 16) | byte2;
            break;
        }
        default: break;
    }

    return ret_value;
}

/* Convert endianess. Sometimes `merge_bytes` will return in big endian. */
template<typename T>
T little_endian(T value)
{
    switch(sizeof(value))
    {
        case 2: {
            ut_BYTE byte1 = value & 0xFF;
            ut_BYTE byte2 = (value >> 8) & 0xFF;
            
            value ^= value;
            value |= byte1;
            value = (value << 8) | byte2;
            break;
        }
        case 4: {
            ut_WORD byte1 = (value >> 16);
            ut_WORD byte2 = (value >> 24);

            value ^= value;
            value |= byte1;
            value = (value << 16) | byte2;
            break;
        }
        default: break;
    }

    return value;
}

#include "error.hpp"
#include "file_api.hpp"
#include "dot_doc_file_beginning.hpp"

#endif