#include <stdint.h>
#include <core/ZosDefs.hpp>
#include <core/cpp/System.hpp>
#include <hal/hal.hpp>

#include <boot/bootparams.h>

#include <core/Debug.hpp>

#include <core/std/vector.hpp>

#include <core/cpp/Memory.hpp>
#include <core/dev/MBR.hpp>

#include <core/fs/FATFileSystem.hpp>
#include <core/dev/RangeBlockDevice.hpp>
#include <core/arch/i686/Disk.hpp>

#include <core/cpp/String.hpp>

#include <core/arch/i686/PCI.hpp>

#include <core/dev/RTL8139.hpp>

#include <core/arch/i686/Timer.hpp>
#include <core/arch/i686/RTC.hpp>

#include <core/net/Net.hpp>

#pragma region 
// libgcc function which calls all global constructors.
// Usually this is done in assembly, but since we skipped that part, we do it here.
extern "C" void _init();

// Test to make sure the global constructors are called.
// Notice how this is never called anywhere in *MY* code.
void __attribute__((constructor)) test_constructor() {
    Debug::Debug("Constructor", "Global Constructors called.");
}

#pragma endregion

void EoH(int exit_code) {
    exit(exit_code);
    HALT;
}

// Kernel Main
extern "C" void KernelEntry(BootParams* bootParams) {
    // Call all global instructors
    _init();
    HAL_Initialize(bootParams);

    Debug::Info("Kernel Main", "Kernel Initialization Success!");

    RTC::Time time{};
    RTC::GetTime(time);
    time.hour -= 4;
    RTC::SetTime(time);
    RTC::GetTime(time);
    RTC::LogTime("Kernel Main", time);

    IOAllocator KernelIOAllocator{};
    IORange disk_pio_range = KernelIOAllocator.RequestIORange(0x1F0, 8, false);
    Disk disk{ bootParams->BootDevice, &disk_pio_range, true };
    if (!disk.Initialize()) {
        Debug::Critical("Kernel Main", "Disk initialization failed!");
        EoH(1);
    }
    
    BlockDevice* partition;
    RangeBlockDevice partitionRange;
    if (bootParams->BootDevice < 0x80) {
        partition = &disk;
    } else {
        MBR_entry* entry = ToLinear<MBR_entry*>(reinterpret_cast<uint32_t>(bootParams->PartitionLocation));
        partitionRange.Initialize(&disk, entry->LBA_Start * Disk::BytesPerSector, entry->SectorCount * Disk::BytesPerSector);
        partition = &partitionRange;
    }

    FATFileSystem fatfs;
    if (!fatfs.Initialize(partition)) {
        Debug::Critical("Kernel Main", "Failed to initialize FATFS");
        EoH(1);
    }
    
    /* { // File system demo
        const char* file_path = "/folder/demo.txt";
        File* test = fatfs.Open(file_path, FileOpenMode::Read);
        if (!test) {
            Debug::Critical("Kernel Main", "Failed to open %s", file_path);
            EoH(1);
        }
        std::vector<uint8_t> text_data(test->Size());
        test->Read(text_data.data(), text_data.size());
        text_data.emplace_back('\0');
        Debug::Info("Kernel Main", "%s contents:\n%s", file_path, text_data.data());
        const char* text = "I'm sooooo bored~";
        test->Write(reinterpret_cast<const uint8_t*>(text), strlen(text));
        test->Seek(0, SeekPos::Set);
        text_data.clear();
        text_data.resize(test->Size());
        test->Read(text_data.data(), text_data.size());
        text_data[text_data.size()] = '\0';
        Debug::Info("Kernel Main", "%s contents:\n%s", file_path, text_data.data());
    } */

    IORange pci_io{ KernelIOAllocator.RequestIORange(PCI::PCI_CONFIG_ADDRESS, 8, false) };
    PCI pci{ &pci_io };
    PCIDevice rtl8139_dev{ pci.FindDevice(0x10EC, 0x8139) };
    rtl8139_dev.PrintIDs();
    GeneralPCIDevice rtl8139_pci = rtl8139_dev.Upgrade();
    PCIDevice::MmapRange rtl8139_mmap{ rtl8139_pci.FindMmapRange() };

    RTL8139 rtl8139{ &rtl8139_pci, rtl8139_mmap, true };
    auto mac = rtl8139.GetMACAddress();
    Debug::Info("Kernel Main", "ZOS MAC: %02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    const char* test_string = "A long test string that is over sixty four bytes so the kernel doesn't crash lmaooo";
    auto slice = std::slice<uint8_t>((uint8_t*)const_cast<char*>(test_string), strlen(test_string) + 1 /* For null byte */);
    //rtl8139.write(slice);
    while (true) {
        rtl8139.read([rtl8139](Net::Payload packet) {
            Net::ParsedEthernetFrame frame = Net::ParsePacket(packet);
            switch (frame.packet->Type()) {
            case Net::PacketType::Arp: {
                Net::ArpFrame* packet = (Net::ArpFrame*)frame.packet;
                switch (packet->GetOperation()) {
                case Net::ArpFrame::ArpOperation::Request: {
                    uint8_t temp_mac[6];
                    uint8_t temp_ip[4];
                    memcpy(temp_mac, packet->GetSenderHardwareAddr(), 6);
                    memcpy(temp_ip, packet->GetSenderProtocolAddr(), 4);
                    memcpy(packet->GetSenderProtocolAddr(), packet->GetTargetProtocolAddr(), 4);
                    memcpy(packet->GetSenderHardwareAddr(), rtl8139.GetMACAddress().data(), 6);
                    memcpy(packet->GetTargetHardwareAddr(), temp_mac, 6);
                    memcpy(packet->GetTargetProtocolAddr, temp_ip, 4);
                    // TODO: Refactor this into a params struct or something to generate ARP Frames and Ethernet Frames into byte pointers or something it's 12:30 and I'm tired, boss.
                } break;
                case Net::ArpFrame::ArpOperation::Reply: {
                    Debug::Error("Kernel Main", "ARP replies have not been implemented yet!");
                } break;
                default: break;
                } 
            } break;
            default: break;
            }
        });
    }


    Debug::Info("Kernel Main", "Now we sleep for 2.5 seconds and then exit!");
    sleep(2500);
    Debug::Info("Kernel Main", "We woke up!");


    EoH(0);
}
