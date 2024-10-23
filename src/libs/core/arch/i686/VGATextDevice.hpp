#pragma once

#include <core/dev/BlockDevice.hpp>

namespace arch {
    namespace i686 {
        class VGATextDevice : public CharacterDevice {
        public:
            VGATextDevice();
            virtual size_t Read(uint8_t* data, size_t size) override;
            virtual size_t Write(const uint8_t* data, size_t size) override;

            void ClearScreen();

        private:
            void PutChar(int x, int y, char c);
            void PutColor(int x, int y, uint8_t color);
            char GetChar(int x, int y);
            uint8_t GetColor(int x, int y);
            void SetCursor(int x, int y);
            void Scroll(int lines);
            void PutChar(char c);

            static uint8_t* m_ScreenBuffer;
            int m_ScreenX;
            int m_ScreenY;
        };
    }
}
