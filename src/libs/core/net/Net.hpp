#pragma once

#include <stdint.h>
#include <stddef.h>

#include <core/ZosDefs.hpp>

#include <core/std/vector.hpp>

#include <core/dev/RTL8139.hpp>

namespace Net {
    using Payload = std::slice<uint8_t>;

    enum PacketType : uint16_t {
        Ipv4 = 0x0800,
        Arp = 0x0806,
    };

    struct EthernetFrameParams {
        uint8_t* DestinationMAC{ nullptr };
        uint8_t* SourceMAC{ nullptr };
        PacketType EtherType{};
        std::vector<uint8_t> payload;
    };

    class EthernetFrame {
    public:
        EthernetFrame(Payload& packet);
        EthernetFrame(EthernetFrameParams& params);

        void Send(RTL8139* rtl8139);

        Payload GetPayload() { return payload; }
        PacketType GetEtherType() { return EtherType; }
        std::vector<uint8_t> DeepCopy() { return owned_payload; }

    private:
        uint8_t DestinationMAC[6]{};
        uint8_t SourceMAC[6]{};
        uint32_t Tag{};
        PacketType EtherType{};
        uint32_t CRC{};
        size_t PayloadOffset{};
        Payload payload;
        std::vector<uint8_t> owned_payload;

        void SetPayload(Payload& packet);
        bool HasDot1Q(Payload& packet);
        size_t EtherTypeOffset(Payload& packet);

    };
    
    class ParsedPacket {
    public:
        virtual constexpr PacketType Type() const = 0;
    };

    struct ParsedEthernetFrame {
        EthernetFrame* ethernet{ nullptr };
        ParsedPacket* packet{ nullptr };
    };

    struct ArpFrameParams;

    enum ArpOperation : uint16_t {
        Request = 1,
        Reply = 2,
    };

    struct ArpFrameParams {
        uint16_t HardwareType;
        uint16_t ProtocolType;
        uint8_t HardwareAddrSize;
        uint8_t ProtocolAddrSize;
        ArpOperation Operation;
        uint8_t* SenderHardwareAddr{};
        uint8_t* SenderProtocolAddr{};
        uint8_t* TargetHardwareAddr{};
        uint8_t* TargetProtocolAddr{};
    };

    class ArpFrame : public ParsedPacket {
    public:
        ArpFrame(Payload& packet);
        ArpFrame(ArpFrameParams& params);

        void Send(RTL8139* rtl8139);

        uint16_t GetHardwareType() const { return HardwareType; }
        uint16_t GetProtocolType() const { return ProtocolType; }
        uint8_t GetHardwareAddrSize() const { return HardwareAddrSize; }
        uint8_t GetProtocolAddrSize() const { return ProtocolAddrSize; }
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

    enum Ipv4Protocol : uint8_t {
        Tcp = 0x06,
        Udp = 0x11,
    };

    class Ipv4Frame : public ParsedPacket {
    public:
        Ipv4Frame(Payload& packet);

        uint16_t CalculateChecksum(Payload& data);


        uint8_t GetIHL() const { return IHL; }
        uint16_t GetPayloadSize() const { return TotalSize; }
        Ipv4Protocol GetProtocol() const { return Protocol; }
        uint8_t* GetSourceIP() { return &SourceIP[0]; }
        Payload& GetPayload() { return payload; }

        virtual constexpr PacketType Type() const override { return PacketType::Ipv4; }

    private:
        uint8_t IHL;
        uint16_t TotalSize;
        Ipv4Protocol Protocol;
        uint8_t SourceIP[4]{};
        Payload payload;
    };

    struct UdpFrame {
        std::vector<uint8_t> data{};
        
        UdpFrame(Payload& payload);
    };

    void GetPacket(std::vector<uint8_t>& out_packet, RTL8139* rtl8139);
};