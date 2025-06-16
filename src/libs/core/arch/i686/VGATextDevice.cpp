#include "VGATextDevice.hpp"
#include "IO.hpp"

namespace arch {
    namespace i686 {

        static inline constexpr unsigned ScreenWidth = 80;
        static inline constexpr unsigned ScreenHeight = 25;
        static inline constexpr uint8_t DefaultColor = 0x07;

        uint8_t* VGATextDevice::m_ScreenBuffer = (uint8_t*)0xB8000;

        VGATextDevice::VGATextDevice() 
            : m_ScreenX{ 0 }, m_ScreenY{ 0 } {
            ClearScreen();
        }

        void VGATextDevice::PutChar(char c) {
            switch (c) {
                case '\n': 
                    m_ScreenY++;
                case '\r': 
                    m_ScreenX = 0; 
                    break;
                case '\t': 
                    for (int i = 0; i < 4 - (m_ScreenX % 4); i++) 
                        PutChar(' '); 
                    break;
                default:
                    PutChar(m_ScreenX, m_ScreenY, c);
                    m_ScreenX++;
                    break;
            }

            if (m_ScreenX >= ScreenWidth) {
                m_ScreenY++;
                m_ScreenX = 0;
            }

            if (m_ScreenY >= ScreenHeight) Scroll(1);

            SetCursor(m_ScreenX, m_ScreenY);
        }

        void VGATextDevice::ClearScreen() {
            for (int y = 0; y < ScreenHeight; y++ ) {
                for (int x = 0; x < ScreenWidth; x++) {
                    PutChar(x, y, '\0');
                    PutColor(x, y, DefaultColor);
                }
            }
            m_ScreenX = 0;
            m_ScreenY = 0;
            SetCursor(0, 0);
        }

        void VGATextDevice::PutChar(int x, int y, char c) {
            m_ScreenBuffer[2 * (y * ScreenWidth + x)] = c;
        }

        char VGATextDevice::GetChar(int x, int y) {
            return m_ScreenBuffer[2 * (y * ScreenWidth + x)];
        }

        void VGATextDevice::PutColor(int x, int y, uint8_t color) {
            m_ScreenBuffer[2 * (y * ScreenWidth + x) + 1] = color;
        }

        uint8_t VGATextDevice::GetColor(int x, int y) {
            return m_ScreenBuffer[2 * (y * ScreenWidth + x) + 1];
        }

        void VGATextDevice::SetCursor(int x, int y) {
            int pos = y * ScreenWidth + x;
            OutPortB(0x3D4, 0x0F);
            OutPortB(0x3D5, (uint8_t)(pos & 0xFF));
            OutPortB(0x3D4, 0x0E);
            OutPortB(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
        }

        void VGATextDevice::Scroll(int lines) {
            for (int y = lines; y < ScreenHeight; y++) {
                for (int x = 0; x < ScreenWidth; x++) {
                    PutChar(x, y - lines, GetChar(x, y));
                    PutColor(x, y - lines, GetColor(x, y));
                }
            }
            
            for (int y = ScreenHeight - lines; y < ScreenHeight; y++) {
                for (int x = 0; x < ScreenWidth; x++) {
                    PutChar(x, y, ' ');
                    PutColor(x, y, DefaultColor);
                }
            }

            m_ScreenY -= lines;
        }

        size_t VGATextDevice::Read(uint8_t* data, size_t size) {
            return -1;
        }

        size_t VGATextDevice::Write(const uint8_t* data, size_t size) {
            for (size_t i = 0; i < size; i++)                
                PutChar(data[i]);
            return size;
        }
    }
}
