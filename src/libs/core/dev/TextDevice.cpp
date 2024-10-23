#include "TextDevice.hpp"


enum class FormatState { 
    Normal          = 0,
    Length          = 1,
    LengthShort     = 2,
    LengthLong      = 3,
    Spec            = 4,
};

enum class FormatLength {
    Default         = 0,
    ShortShort      = 1,
    Short           = 2,
    Long            = 3,
    LongLong        = 4,
};

TextDevice::TextDevice(CharacterDevice* dev) 
    : m_Device(dev) { }

bool TextDevice::Write(char c) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&c);
    return m_Device->Write(p, sizeof(c)) == sizeof(c);
}

bool TextDevice::Write(const char* str) {
    bool ok = true;
    while (*str && ok) {
        ok = ok && Write(*str);
        str++;
    }
    return ok;
}

bool TextDevice::VWriteF(const char* fmt, va_list args) {
    FormatState state = FormatState::Normal;
    FormatLength length = FormatLength::Default;
    int base = 10;
    bool ok = true;
    bool number = false;
    bool sign = false;

    while (*fmt && ok) {
        switch (state) {
            case FormatState::Normal:
                switch (*fmt) {
                    case '%':   state = FormatState::Length;
                                break;
                    default:    ok = ok && Write(*fmt);
                                break;
                }
                break;

            case FormatState::Length:
                switch (*fmt)
                {
                    case 'h':   length = FormatLength::Short;
                                state = FormatState::LengthShort;
                                break;
                    case 'l':   length = FormatLength::Long;
                                state = FormatState::LengthLong;
                                break;
                    default:    goto FormatState_Spec;
                }
                break;

            case FormatState::LengthShort:
                if (*fmt == 'h')
                {
                    length = FormatLength::ShortShort;
                    state = FormatState::Spec;
                }
                else goto FormatState_Spec;
                break;

            case FormatState::LengthLong:
                if (*fmt == 'l')
                {
                    length = FormatLength::LongLong;
                    state = FormatState::Spec;
                }
                else goto FormatState_Spec;
                break;

            case FormatState::Spec:
            FormatState_Spec:
                switch (*fmt)
                {
                    case 'c':   ok = ok && Write((char)va_arg(args, int));
                                break;

                    case 's':   ok = ok && Write(va_arg(args, const char*));
                                break;

                    case '%':   ok = ok && Write('%');
                                break;

                    case 'd':
                    case 'i':   base = 10; sign = true; number = true;
                                break;

                    case 'u':   base = 10; sign = false; number = true;
                                break;

                    case 'X':
                    case 'x':
                    case 'p':   base = 16; sign = false; number = true;
                                break;

                    case 'o':   base = 8; sign = false; number = true;
                                break;

                    // ignore invalid spec
                    default:    break;
                }

                if (number) {
                    if (sign) {
                        switch (length) {
                        case FormatLength::ShortShort:
                        case FormatLength::Short:
                        case FormatLength::Default:     
                            ok = ok && Write(va_arg(args, int), base);
                            break;

                        case FormatLength::Long:        
                            ok = ok && Write(va_arg(args, long), base);
                            break;

                        case FormatLength::LongLong:    
                            ok = ok && Write(va_arg(args, long long), base);
                            break;
                        }
                    } else {
                        switch (length) {
                        case FormatLength::ShortShort:
                        case FormatLength::Short:
                        case FormatLength::Default:     
                            ok = ok && Write(va_arg(args, unsigned int), base);
                            break;
                                                        
                        case FormatLength::Long:        
                            ok = ok && Write(va_arg(args, unsigned  long), base);
                            break;

                        case FormatLength::LongLong:    
                            ok = ok && Write(va_arg(args, unsigned  long long), base);
                            break;
                        }
                    }
                }

                // reset state
                state = FormatState::Normal;
                length = FormatLength::Default;
                sign = false;
                base = 10;
                number = false;
                break;
        }

        fmt++;
    }

    return ok;
}

bool TextDevice::WriteF(const char* fmt, ...) {
    bool ok = true;
    va_list args;
    va_start(args, fmt);
    ok = ok && VWriteF(fmt, args);
    va_end(args);
    return ok;
}

bool TextDevice::WriteBuffer(const char* msg, const void* buffer, size_t size) {
    const uint8_t* u8Buffer = (const uint8_t*)buffer;
    
    bool ok = true;
    ok = ok && Write(msg);
    for (uint32_t i = 0; i < size && ok; i++)
    {
        ok = ok && Write(m_HexChars[u8Buffer[i] >> 4]);
        ok = ok && Write(m_HexChars[u8Buffer[i] & 0xF]);
    }
    ok = ok && Write("\n");
    return ok;
}

template<typename T>
bool TextDevice::Write(T num, int base) {
    bool ok = true;
    typename MakeUnsigned<T>::Type uNum;
    if (IsSigned<T>() && num < 0) {
        ok = ok && Write('-');
        uNum = -num;
    } else uNum = num;

    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do {
        typename MakeUnsigned<T>::Type rem = uNum % base;
        uNum /= base;
        buffer[pos++] = m_HexChars[rem];
    } while (uNum > 0);

    // print number in reverse order
    while (--pos >= 0)
        ok = ok && Write(buffer[pos]);
    
    return ok;
}

