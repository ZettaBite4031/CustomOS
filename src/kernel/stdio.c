#include <stdio.h>
#include <arch/i686/io.h>

#include <stdarg.h>
#include <stdbool.h>

#include <hal/vfs.h>

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

const char g_HexChars[] = "0123456789ABCDEF";

void fputc(char c, fd_t file) {
    VFS_Write(file, &c, sizeof(c));
}

void fputs(const char* str, fd_t file) {
    while (*str) {
        fputc(*str, file);
        str++;
    }
}

void fprintf_unsigned(fd_t file, unsigned long long number, int radix) {
    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        fputc(buffer[pos], file);
}

void fprintf_signed(fd_t file, long long number, int radix) {
    if (number < 0) {
        fputc(file, '-');
        fprintf_unsigned(file, -number, radix);
    }
    else fprintf_unsigned(file, number, radix);
}

void fprintf_double(fd_t file, double number, int radix) {
    if (number < 0) {
        fputc('-', file);
        number = -number;
    }

    unsigned long long int_part = (unsigned long long)number;
    double fractional = number - (double)int_part;

    fprintf_unsigned(file, int_part, radix);
    fputc('.', file);

    char buffer[32];
    int pos = 0;

    // TODO: implement a more sophisticated precision specifier
    int precision = 4;
    for (int i = 0; i < precision; i++) {
        fractional *= 10.f;
        int digit = (int)fractional;
        buffer[pos++] = g_HexChars[digit];
        fractional -= digit;
    }

    for (int i = 0; i < pos; ++i)
        fputc(buffer[i], file);
}

void vfprintf(fd_t file, const char* fmt, va_list args) {
    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;
    bool number = false;
    bool floating = false;

    while (*fmt)
    {
        switch (state)
        {
            case PRINTF_STATE_NORMAL:
                switch (*fmt)
                {
                    case '%':   state = PRINTF_STATE_LENGTH;
                                break;
                    default:    fputc(*fmt, file);
                                break;
                }
                break;

            case PRINTF_STATE_LENGTH:
                switch (*fmt)
                {
                    case 'h':   length = PRINTF_LENGTH_SHORT;
                                state = PRINTF_STATE_LENGTH_SHORT;
                                break;
                    case 'l':   length = PRINTF_LENGTH_LONG;
                                state = PRINTF_STATE_LENGTH_LONG;
                                break;
                    default:    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_LENGTH_SHORT:
                if (*fmt == 'h')
                {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;

            case PRINTF_STATE_LENGTH_LONG:
                if (*fmt == 'l')
                {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;

            case PRINTF_STATE_SPEC:
            PRINTF_STATE_SPEC_:
                switch (*fmt)
                {
                    case 'c':   fputc((char)va_arg(args, int), file);
                                break;

                    case 's':   fputs(va_arg(args, const char*), file);
                                break;

                    case '%':   fputc('%', file);
                                break;

                    case 'd':
                    case 'i':   radix = 10; sign = true; number = true; floating = false;
                                break;

                    case 'u':   radix = 10; sign = false; number = true; floating = false;
                                break;

                    case 'X':
                    case 'x':
                    case 'p':   radix = 16; sign = false; number = true; floating = false;
                                break;

                    case 'o':   radix = 8; sign = false; number = true; floating = false;
                                break;

                    case 'f':   radix = 10; sign = true; number = true; floating = true;
                                break;

                    // ignore invalid spec
                    default:    break;
                }
                
                if (number) {
                    if (floating) {
                        fprintf_double(file, va_arg(args, double), radix);
                    }
                    else if (sign) {
                        switch (length) {
                            case PRINTF_LENGTH_SHORT_SHORT:
                            case PRINTF_LENGTH_SHORT:
                            case PRINTF_LENGTH_DEFAULT:
                                fprintf_signed(file, va_arg(args, int), radix);
                                break;
                            case PRINTF_LENGTH_LONG:
                                fprintf_signed(file, va_arg(args, long), radix);
                                break;
                            case PRINTF_LENGTH_LONG_LONG:
                                fprintf_signed(file, va_arg(args, long long), radix);
                        }
                    }
                    else {
                        switch (length) {
                            case PRINTF_LENGTH_SHORT_SHORT:
                            case PRINTF_LENGTH_SHORT:
                            case PRINTF_LENGTH_DEFAULT:
                                fprintf_unsigned(file, va_arg(args, unsigned int), radix);
                                break;
                            case PRINTF_LENGTH_LONG:
                                fprintf_unsigned(file, va_arg(args, unsigned long), radix);
                                break;
                            case PRINTF_LENGTH_LONG_LONG:
                                fprintf_unsigned(file, va_arg(args, unsigned long long), radix);
                                break;
                        }
                    }
                }

                // reset state
                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                floating = false;
                break;
        }

        fmt++;
    }
}

void fprintf(fd_t file, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(file, fmt, args);
    va_end(args);
}

void fprint_buffer(fd_t file, const char* msg, const void* buffer, uint32_t count) {
    const uint8_t* u8Buffer = (const uint8_t*)buffer;
    
    fputs(msg, file);
    for (uint32_t i = 0; i < count; i++)
    {
        fputc(g_HexChars[u8Buffer[i] >> 4], file);
        fputc(g_HexChars[u8Buffer[i] & 0xF], file);
    }
    fputs("\n", file);
}

void putc(char c) {
    fputc(c, VFS_FD_STDOUT);
}

void puts(const char* str){
    fputs(str, VFS_FD_STDOUT);
}

void printf_unsigned(unsigned long long number, int radix) {
    fprintf_unsigned(VFS_FD_STDOUT, number, radix);
}

void printf_signed(long long number, int radix) {
    fprintf_signed(VFS_FD_STDOUT, number, radix);
}

void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(VFS_FD_STDOUT, fmt, args);
    va_end(args);
}

void print_buffer(const char* msg, const void* buffer, uint32_t count) {
    fprint_buffer(VFS_FD_STDOUT, msg, buffer, count);
}

void debugc(char c) {
    fputc(c, VFS_FD_DEBUG);
}

void debugs(const char* str) {
    fputs(str, VFS_FD_DEBUG);
}

void debugf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(VFS_FD_DEBUG, fmt, args);
    va_end(args);
}

void debug_buffer(const char* msg, const void* buffer, uint32_t count) {
    fprint_buffer(VFS_FD_DEBUG, msg, buffer, count);
}
