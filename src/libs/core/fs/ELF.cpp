#include "ELF.hpp"

#include <core/Debug.hpp>
#include <core/cpp/Memory.hpp>

ELF::ELF(File* elf_file_handle) {
    uint8_t* headerBuffer = reinterpret_cast<uint8_t*>(&m_Header);
    if (elf_file_handle->Read(headerBuffer, sizeof(m_Header)) != sizeof(m_Header)) {
        Debug::Error("ELF", "Could not read ELFHeader");
        return;
    }

    if (!ValidateHeader()) {
        Debug::Error("ELF", "Failed to validate ELF Header.");
        return;
    }

    
}

bool ELF::ValidateHeader() {
    bool ok = true;
    if (Memory::Compare(m_Header.Magic, ELF_MAGIC, 4) != 0) {
        Debug::Error("ELF", "Read file's magic is not ELF Magic.");
        ok = false;
    }
    if (m_Header.Bitness != static_cast<uint8_t>(ELFBitness::BITS32)) {
        Debug::Error("ELF", "ELF is not 32-bit.");
        ok = false;
    }
    if (m_Header.Endianness != static_cast<uint8_t>(ELFEndianness::LITTLE)) {
        Debug::Error("ELF", "ELF is not little-endian.");
        ok = false;
    }
    if (m_Header.ELFVersion != 1) {
        Debug::Error("ELF", "ELF Version is wrong. Expected: 0x1 | Actual: %X", m_Header.ELFVersion);
        ok = false;
    }
    if (m_Header.HeaderVersion != 1) {
        Debug::Error("ELF", "ELF Header Version is wrong. Expected: 0x1 | Actual: %X", m_Header.HeaderVersion);
        ok = false;
    }
    if (m_Header.ELFType != static_cast<uint8_t>(ELFType::EXECUTABLE)) {
        Debug::Error("ELF", "ELF Type is wrong. Expected: 0x2 | Actual: %X", m_Header.ELFType);
        ok = false;
    }
    if (m_Header.InstructionSet != static_cast<uint8_t>(ELFInstructionSet::X86)) {
        Debug::Error("ELF", "ELF Instruction set is wrong. Expected: 0x3 | Actual: %X", m_Header.InstructionSet);
        ok = false;
    }
    return ok;
}