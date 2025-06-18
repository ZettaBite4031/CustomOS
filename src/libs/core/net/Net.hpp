#pragma once

#include <stdint.h>
#include <stddef.h>

#include <core/ZosDefs.hpp>

#include <core/std/vector.hpp>

#include <core/Debug.hpp>

namespace Net {
    using Payload = std::slice<uint8_t>;

    enum PacketType {
        Arp,
        Ipv4,
        Udp,
        Tcp,
    };

    class EthernetFrame {
    public:
        EthernetFrame(Payload& packet) {
            memcpy(&DestinationMAC, &packet[0], 6);
            memcpy(&SourceMAC, &packet[6], 6);
            if (HasDot1Q(packet)) memcpy(&Tag, &packet[12], sizeof(uint16_t));
            memcpy(&EtherType, &packet[EtherTypeOffset(packet)], sizeof(uint16_t));
            EtherType = std::ntohs(EtherType);
            PayloadOffset = EtherTypeOffset(packet) + sizeof(uint16_t);
            SetPayload(packet);
            memcpy(&CRC, &packet[packet.size() - sizeof(uint32_t)], sizeof(uint32_t));
            CRC = std::ntohs(CRC);
        }

        void Dump() {
            Debug::HexDump("Destination MAC:", &DestinationMAC, 6);
            Debug::HexDump("Source MAC:", &SourceMAC, 6);
            Debug::HexDump("Tag: ", &Tag, sizeof(Tag));
            Debug::HexDump("EtherType:", &EtherType, sizeof(EtherType));
            Debug::HexDump("CRC:", &CRC, sizeof(CRC));
            Debug::HexDump("Payload:", payload.data(), payload.size());
        }

        Payload GetPayload() { return payload; }
        uint16_t GetEtherType() { return EtherType; }

    private:
        uint8_t DestinationMAC[6]{};
        uint8_t SourceMAC[6]{};
        uint32_t Tag{};
        uint16_t EtherType{};
        uint32_t CRC{};
        size_t PayloadOffset{};

        void SetPayload(Payload& packet) {
            size_t end = packet.size() - 4; // don't include CRC
            payload = Payload(&packet[PayloadOffset], end - PayloadOffset);
        }

        bool HasDot1Q(Payload& packet) {
            uint16_t tag;
            memcpy(&tag, &packet[12], sizeof(uint16_t));
            return tag == 0x8100;
        }

        size_t EtherTypeOffset(Payload& packet) {
            return HasDot1Q(packet) ? 16 : 12;
        }

        Payload payload;
    };
    
    class ParsedPacket {
    public:
        virtual constexpr PacketType Type() const = 0;
    };

    struct ParsedEthernetFrame {
        EthernetFrame* ethernet;
        ParsedPacket* packet;
    };

    class ArpFrame : public ParsedPacket {
    public:
        enum ArpOperation : uint16_t {
            Request = 1,
            Reply = 2,
        };

        ArpFrame(Payload& packet) {
            memcpy(&HardwareType, &packet[0], sizeof(HardwareType));
            HardwareType = std::ntohs(HardwareType);
            memcpy(&ProtocolType, &packet[2], sizeof(ProtocolType));
            ProtocolType = std::ntohs(ProtocolType);
            memcpy(&HardwareAddrSize, &packet[4], sizeof(HardwareAddrSize));
            memcpy(&ProtocolAddrSize, &packet[5], sizeof(ProtocolAddrSize));
            uint16_t operation;
            memcpy(&operation, &packet[6], sizeof(operation));
            Operation = (ArpOperation)std::ntohs(operation);
            memcpy(&SenderHardwareAddr, &packet[8], sizeof(SenderHardwareAddr));
            memcpy(&SenderProtocolAddr, &packet[14], sizeof(SenderProtocolAddr));
            memcpy(&TargetHardwareAddr, &packet[18], sizeof(TargetHardwareAddr));
            memcpy(&TargetProtocolAddr, &packet[24], sizeof(TargetProtocolAddr));
        }

        void Dump() {
            Debug::HexDump("Hardware Type:", &HardwareType, sizeof(HardwareType));
            Debug::HexDump("Protocol Type:", &ProtocolType, sizeof(ProtocolType));
            Debug::HexDump("Hardware Addr Size:", &HardwareAddrSize, sizeof(HardwareAddrSize));
            Debug::HexDump("Protocol Addr Size:", &ProtocolAddrSize, sizeof(ProtocolAddrSize));
            Debug::HexDump("Operation:", &Operation, sizeof(Operation));
            Debug::HexDump("Sender Hardware Addr:", &SenderHardwareAddr, sizeof(SenderHardwareAddr));
            Debug::HexDump("Sender Protocol Addr:", &SenderProtocolAddr, sizeof(SenderProtocolAddr));
            Debug::HexDump("Target Hardware Addr:", &TargetHardwareAddr, sizeof(TargetHardwareAddr));
            Debug::HexDump("Target Protocol Addr:", &TargetProtocolAddr, sizeof(TargetProtocolAddr));
        }

        ArpOperation GetOperation() const { return Operation; }

        uint8_t* GetSenderHardwareAddr() { return SenderHardwareAddr; }
        uint8_t* GetSenderProtocolAddr() { return SenderProtocolAddr; }
        uint8_t* GetTargetHardwareAddr() { return TargetHardwareAddr; }
        uint8_t* GetTargetProtocolAddr() { return TargetProtocolAddr; }

        virtual constexpr PacketType Type() const override { return PacketType::Arp; };
    private:
        uint16_t HardwareType{};
        uint16_t ProtocolType{};
        uint8_t HardwareAddrSize{};
        uint8_t ProtocolAddrSize{};
        ArpOperation Operation{};
        uint8_t SenderHardwareAddr[6]{};
        uint8_t SenderProtocolAddr[4]{};
        uint8_t TargetHardwareAddr[6]{};
        uint8_t TargetProtocolAddr[4]{};
    };

    ParsedEthernetFrame ParsePacket(Payload& packet) {
        EthernetFrame* frame = new EthernetFrame(packet);
        Payload payload = frame->GetPayload();
        uint16_t ether_type = frame->GetEtherType();

        ParsedEthernetFrame parsed_frame;
        parsed_frame.ethernet = frame;
        
        switch (ether_type) {
        case 0x0806: {
            ArpFrame* arp_frame = new ArpFrame(payload);
            parsed_frame.packet = arp_frame;
        } break;
        case 0x0800: {

            parsed_frame.packet = nullptr;
        } break;
        default: 
            parsed_frame.packet = nullptr; break;
        }

        return parsed_frame;
    }


};