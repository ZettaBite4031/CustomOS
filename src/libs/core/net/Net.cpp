#include "Net.hpp"

#include <core/Debug.hpp>


namespace Net {
    namespace {
        ParsedEthernetFrame ParsePacket(Payload& packet) {
            EthernetFrame* frame = new EthernetFrame(packet);
            Payload payload = frame->GetPayload();
            uint16_t ether_type = frame->GetEtherType();

            if (payload.size() < 20) {
                Debug::Error("ParsePacket", "Payload size (%zu) is too short to be a valid IPv4 packet", payload.size());
                return {};
            }

            ParsedEthernetFrame parsed_frame;
            parsed_frame.ethernet = frame;
            
            switch (ether_type) {
            case PacketType::Arp: {
                ArpFrame* arp_frame = new ArpFrame(payload);
                parsed_frame.packet = arp_frame;
            } break;
            case PacketType::Ipv4: {
                Ipv4Frame* ipv4_frame = new Ipv4Frame(payload);
                parsed_frame.packet = ipv4_frame;
            } break;
            default: 
                Debug::Warn("Net::ParsePacket", "Encountered unknown packet!");
                parsed_frame.packet = nullptr; break;
            }

            return parsed_frame;
        }

        bool HandlePacket(Net::ParsedPacket* packet, RTL8139* rtl8139, std::vector<uint8_t>& out_data) {
            if (!packet) {
                Debug::Error("Net::HandlePacket", "Unexpected packet type!");
                return false;
            }

            bool data_fully_received = false;
            switch(packet->Type()) {
            case Net::PacketType::Arp: {
                Net::ArpFrame* arp_frame = (Net::ArpFrame*)packet;
                Net::ArpFrameParams params{
                    .HardwareType = arp_frame->GetHardwareType(),
                    .ProtocolType = arp_frame->GetProtocolType(),
                    .HardwareAddrSize = arp_frame->GetHardwareAddrSize(),
                    .ProtocolAddrSize = arp_frame->GetProtocolAddrSize(),
                    .Operation = Net::ArpOperation::Reply,
                    .SenderHardwareAddr = rtl8139->GetMACAddress().data(),
                    .SenderProtocolAddr = arp_frame->GetTargetProtocolAddr(),
                    .TargetHardwareAddr = arp_frame->GetSenderHardwareAddr(),
                    .TargetProtocolAddr = arp_frame->GetSenderProtocolAddr(),
                };
                Net::ArpFrame response(params);
                response.Send(rtl8139);
            } break;
            case Net::PacketType::Ipv4: {
                Net::Ipv4Frame* ipv4_frame = (Net::Ipv4Frame*)packet;
                data_fully_received = true;
                switch (ipv4_frame->GetProtocol()) {
                case Ipv4Protocol::Udp: {
                    UdpFrame udp_frame(ipv4_frame->GetPayload());
                    out_data.resize(udp_frame.data.size());
                    for (int i = 0; i < out_data.size(); i++)
                        out_data[i] = udp_frame.data[i];
                } break;
                case Ipv4Protocol::Tcp: {
                    Debug::Error("Net::HandlePacket", "TCP not yet implemented!");
                }
                }
            } break;

            }
            return data_fully_received;
        }
    } // anon namespace

    // =================== ETHERNET FRAME ===================
    EthernetFrame::EthernetFrame(Payload& packet) {
        memcpy(&DestinationMAC, &packet[0], 6);
        memcpy(&SourceMAC, &packet[6], 6);
        if (HasDot1Q(packet)) memcpy(&Tag, &packet[12], sizeof(uint16_t));
        memcpy(&EtherType, &packet[EtherTypeOffset(packet)], sizeof(uint16_t));
        EtherType = (PacketType)std::ntohs((uint16_t)EtherType);
        PayloadOffset = EtherTypeOffset(packet) + sizeof(uint16_t);
        SetPayload(packet);
        //memcpy(&CRC, &packet[packet.size() - sizeof(uint32_t)], sizeof(uint32_t));
        //CRC = std::ntohs(CRC);
    }

    EthernetFrame::EthernetFrame(EthernetFrameParams& params) {
        memcpy(&DestinationMAC, &params.DestinationMAC[0], sizeof(DestinationMAC));
        memcpy(&SourceMAC, &params.SourceMAC[0], sizeof(SourceMAC));
        EtherType = params.EtherType;
        std::slice<uint8_t> slice(params.payload.data(), params.payload.size());
        payload = slice;
    }

    void EthernetFrame::Send(RTL8139* rtl8139) {
        size_t length = sizeof(DestinationMAC) + sizeof(SourceMAC) + sizeof(EtherType) + payload.size();
        if (length < 60) length = 60;
        std::vector<uint8_t> packet(length);
        memcpy(&packet[0], DestinationMAC, sizeof(DestinationMAC));
        memcpy(&packet[6], SourceMAC, sizeof(SourceMAC));
        uint16_t ether_type = std::htons((uint16_t)EtherType);
        memcpy(&packet[12], &ether_type, sizeof(ether_type));
        memcpy(&packet[14], payload.data(), payload.size());

        Payload data(packet.data(), packet.size());
        rtl8139->write(data);
    }

    void EthernetFrame::SetPayload(Payload& packet) {
        std::vector<uint8_t> deep_copy(packet.size() - PayloadOffset);
        memcpy(deep_copy.data(), &packet[PayloadOffset], packet.size() - PayloadOffset);
        payload = Payload(deep_copy.data(), deep_copy.size());
        this->owned_payload = std::move(deep_copy);
    }

    bool EthernetFrame::HasDot1Q(Payload& packet) {
        uint16_t tag;
        memcpy(&tag, &packet[12], sizeof(uint16_t));
        return tag == 0x8100;
    }

    size_t EthernetFrame::EtherTypeOffset(Payload& packet) {
        return HasDot1Q(packet) ? 16 : 12;
    }

    // =================== ARP FRAME ===================
    ArpFrame::ArpFrame(Payload& packet) {
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

    ArpFrame::ArpFrame(ArpFrameParams& params) {
        HardwareType = params.HardwareType;
        ProtocolType = params.ProtocolType;
        HardwareAddrSize = params.HardwareAddrSize;
        ProtocolAddrSize = params.ProtocolAddrSize;
        Operation = params.Operation;
        memcpy(&SenderHardwareAddr, params.SenderHardwareAddr, sizeof(SenderHardwareAddr));
        memcpy(&SenderProtocolAddr, params.SenderProtocolAddr, sizeof(SenderProtocolAddr));
        memcpy(&TargetHardwareAddr, params.TargetHardwareAddr, sizeof(TargetHardwareAddr));
        memcpy(&TargetProtocolAddr, params.TargetProtocolAddr, sizeof(TargetProtocolAddr));
    }

    void ArpFrame::Send(RTL8139* rtl8139) {
        size_t length = sizeof(HardwareType) + sizeof(ProtocolType) + sizeof(HardwareAddrSize) + sizeof(ProtocolAddrSize) + sizeof(Operation) + 
                        sizeof(SenderHardwareAddr) + sizeof(SenderProtocolAddr) + sizeof(TargetHardwareAddr) + sizeof(TargetProtocolAddr);
        std::vector<uint8_t> payload(length);
        HardwareType = std::htons(HardwareType);
        memcpy(&payload[0], &HardwareType, sizeof(HardwareType));
        ProtocolType = std::htons(ProtocolType);
        memcpy(&payload[2], &ProtocolType, sizeof(ProtocolType));
        memcpy(&payload[4], &HardwareAddrSize, sizeof(HardwareAddrSize));
        memcpy(&payload[5], &ProtocolAddrSize, sizeof(ProtocolAddrSize));
        uint16_t operation = std::htons((uint16_t)Operation);
        memcpy(&payload[6], &operation, sizeof(Operation));
        memcpy(&payload[8], &SenderHardwareAddr, sizeof(SenderHardwareAddr));
        memcpy(&payload[14], &SenderProtocolAddr, sizeof(SenderProtocolAddr));
        memcpy(&payload[18], &TargetHardwareAddr, sizeof(TargetHardwareAddr));
        memcpy(&payload[24], &TargetProtocolAddr, sizeof(TargetProtocolAddr));
        EthernetFrameParams params {
            .DestinationMAC = TargetHardwareAddr,
            .SourceMAC = SenderHardwareAddr,
            .EtherType = PacketType::Arp,
            .payload = payload,
        };
        EthernetFrame frame(params);
        frame.Send(rtl8139);
    }

    // =================== IPv4 FRAME ===================
    Ipv4Frame::Ipv4Frame(Payload& packet) {
        IHL = packet[0] & 0x0F;
        memcpy(&TotalSize, &packet[2], sizeof(TotalSize));
        TotalSize = std::ntohs(TotalSize);
        Protocol = (Ipv4Protocol)packet[9];
        memcpy(&SourceIP, &packet[12], sizeof(SourceIP));
        size_t ipv4_length = IHL * 4;
        if (TotalSize > packet.size()) {
            Debug::Error("Ipv4Frame", "Totalsize (%d) exceeds packet's actual size (%zu)", TotalSize, packet.size());
            return;
        }
        if ((ipv4_length + 8) > packet.size()) {
            Debug::Error("Ipv4Frame", "IHL Header Length (%d) exceeds packet's actual size (%zu)", ipv4_length, packet.size());
            return;
        }
        //SetPayload(packet, ipv4_length, TotalSize - ipv4_length);
        payload = Payload(&packet[ipv4_length], TotalSize - ipv4_length);
    }

    uint16_t Ipv4Frame::CalculateChecksum(Payload& data) {
        Assert(data.size() % 2 == 0);
        
        uint16_t checksum = 0;

        for (int i = 0; i < data.size(); i += 2) {
            uint16_t val = *(uint16_t*)&payload[i];
            val = std::htons(val);
            checksum += val;
        }

        return ~checksum;
    }

    // =================== UDP FRAME ===================
    UdpFrame::UdpFrame(Payload& payload) {
        uint16_t length;
        memcpy(&length, &payload[4], sizeof(length));
        length = std::ntohs(length);

        if (length > payload.size()) {
            Debug::Error("UdpFrame", "UDP length exceeds IPv4 payload");
            return;
        }

        if (payload.size() < 8 || payload.size() < length) {
            Debug::Error("UdpFrame", "Received invalid UDP frame!");
            return;
        }

        uint16_t payload_size = length - 8;
        data.resize(payload_size);
        for (int i = 0; i < payload_size; i++) {
            data[i] = payload[8+i];
        }
    }

    void GetPacket(std::vector<uint8_t>& out_packet, RTL8139* rtl8139) {
        Net::ParsedEthernetFrame* frame = new ParsedEthernetFrame;
        rtl8139->read([frame](Net::Payload packet) { 
            *frame = Net::ParsePacket(packet); });

        while (!HandlePacket(frame->packet, rtl8139, out_packet)) {
            rtl8139->read([frame](Net::Payload packet) { 
                *frame = Net::ParsePacket(packet); });
        }

        delete frame->ethernet;
        delete frame->packet;
        delete frame;
    }
}