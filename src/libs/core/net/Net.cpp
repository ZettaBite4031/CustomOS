#include "Net.hpp"

#include <bit>
#include <algorithm>
#include <core/cpp/Memory.hpp>
#include <core/std/byte_order.hpp>

namespace Net {
    static inline uint16_t be16(const void* p) {
        uint16_t v; Memory::Copy(&v, p, sizeof(v)); return std::ntohs(v);
    }
    
    static inline uint32_t be32(const void* p) {
        uint32_t v; Memory::Copy(&v, p, sizeof(v)); return std::ntohl(v);
    }

    std::optional<EthernetView> ParseEthernet(Bytes b) {
        if (b.size() < 14) return std::nullopt;
        auto* eh = reinterpret_cast<const EthernetHeader*>(b.data());

        size_t off = 12;
        bool has_vlan = (std::ntohs(eh->type_be) == static_cast<uint16_t>(EtherType::Dot1Q));
        if (has_vlan) {
            if (b.size() < 18) return std::nullopt;
            off += 4;
        }
        if (b.size() < off + 2) return std::nullopt;

        uint16_t et_be;
        Memory::Copy(&et_be, b.data()+off, 2);
        EtherType et = static_cast<EtherType>(std::ntohs(et_be));

        EthernetView v;
        v.hdr = eh;
        v.dot1q = has_vlan;
        v.type = et;
        v.payload = b.subspan(off + 2);
        return v;
    }

    std::optional<ArpView> ParseArp(Bytes b) {
        if (b.size() < sizeof(ArpHeader)) return std::nullopt;
        auto ah = reinterpret_cast<const ArpHeader*>(b.data());
        uint16_t hlen = ah->hlen, plen = ah->plen;
        size_t need = sizeof(ArpHeader) + 2*hlen + 2*plen;
        if (b.size() < need) return std::nullopt;

        size_t off = sizeof(ArpHeader);

        Bytes sha = b.subspan(off, hlen); off += hlen;
        Bytes spa = b.subspan(off, plen); off += plen;
        Bytes tha = b.subspan(off, hlen); off += hlen;
        Bytes tpa = b.subspan(off, plen); 

        ArpView v;
        v.hdr = ah;
        v.sha = sha;
        v.spa = spa;
        v.tha = tha;
        v.tpa = tpa;
        v.op = static_cast<ArpOp>(std::ntohs(ah->op_be));
        return v;
    }

    std::optional<Ipv4View> ParseIpv4(Bytes b) {
        if (b.size() < sizeof(Ipv4Header)) return std::nullopt;
        auto* ih = reinterpret_cast<const Ipv4Header*>(b.data());
        uint8_t ihl = (ih->ver_ihl & 0x0F);
        uint8_t ver = (ih->ver_ihl >> 4);
        if (ver != 4) return std::nullopt;
        uint8_t ihl_bytes = ihl * 4;
        if (ihl_bytes < 20 || b.size() < ihl_bytes) return std::nullopt;

        uint16_t total = std::ntohs(ih->total_len_be);
        if (total < ihl_bytes || b.size() < total) return std::nullopt;

        Ipv4View v;
        v.hdr = ih;
        v.ihl_bytes = ihl_bytes;
        v.total_len = total;
        v.options = ihl_bytes > 20 ? b.subspan(20, ihl_bytes - 20) : Bytes{};
        v.payload = b.subspan(ihl_bytes, total - ihl_bytes);
        v.proto = static_cast<IpProto>(ih->proto);
        return v;
    }

    std::optional<Parsed> ParsePayload(const EthernetView& eth) {
        switch (eth.type) {
            case EtherType::Arp: {
                auto a = ParseArp(eth.payload);
                if (!a) return std::nullopt;
                return *a;
            }
            case EtherType::Ipv4: {
                auto ip = ParseIpv4(eth.payload);
                if (!ip) return std::nullopt;
                return *ip;
            }
            default: return std::nullopt;
        }
    }

    std::optional<UdpView> ParseUdp(const Ipv4View& ip) {
        if (ip.proto != IpProto::Udp) return std::nullopt;
        if (ip.payload.size() < sizeof(UdpHeader)) return std::nullopt;
        auto* uh = reinterpret_cast<const UdpHeader*>(ip.payload.data());
        uint16_t len = std::ntohs(uh->len_be);
        if (len < 8 || ip.payload.size() < len) return std::nullopt;

        UdpView v;
        v.hdr = uh;
        v.data = ip.payload.subspan(8, len - 8);
        return v;
    }

    uint16_t FoldChecksum(uint32_t sum) {
        while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
        return static_cast<uint16_t>(~sum);
    }

    uint16_t Ipv4HeaderChecksum(Bytes hdr) {
        if (hdr.size() % 2) return 0;
        uint32_t sum = 0;
        for (std::size_t i = 0; i < hdr.size(); i += 2) {
            uint16_t w = (hdr[i] << 8) | hdr[i + 1];
            sum += w;
        }
        return FoldChecksum(sum);
    }

    void EthernetBuilder::send(RTL8139& nic) const {
        size_t len = 14 + payload.size();
        if (len < 60) len = 60; // ethernet minimum
        std::vector<uint8_t> frame(len, 0);

        Memory::Copy(&frame.data()[0], DestinationMAC.data(), 6);
        Memory::Copy(&frame.data()[6], SourceMAC.data(), 6);
        uint16_t be = std::htons(static_cast<uint16_t>(type));
        Memory::Copy(&frame.data()[12], &be, 2);
        Memory::Copy(&frame.data()[14], payload.data(), payload.size());

        Bytes out(frame.data(), frame.size());
        nic.write(out);
    }

    EthernetBuilder ArpReplyBuilder::ToEthernet(const std::array<uint8_t, 6>& src_mac, const std::array<uint8_t, 6>& dst_mac) {
        std::vector<uint8_t> p;
        p.resize(sizeof(ArpHeader) + 20); // 20 = sizeof dst/src macs and ips
        ArpHeader ah{};
        ah.htype_be = std::htons(1);
        ah.ptype_be = std::htons(static_cast<uint16_t>(EtherType::Ipv4));
        ah.hlen = 6; ah.plen = 4;
        ah.op_be = std::htons(static_cast<uint16_t>(ArpOp::Reply));
        Memory::Copy(p.data(), &ah, sizeof(ah));
        std::size_t off = sizeof(ArpHeader);
        Memory::Copy(&p.data()[off], sha.data(), 6); off += 6;
        Memory::Copy(&p.data()[off], spa.data(), 4); off += 4;
        Memory::Copy(&p.data()[off], tha.data(), 6); off += 6;
        Memory::Copy(&p.data()[off], tpa.data(), 4);

        EthernetBuilder eb;
        eb.DestinationMAC = dst_mac;
        eb.SourceMAC = src_mac;
        eb.type = EtherType::Arp;
        eb.payload = std::move(p);
        return eb;
    }

    bool AttemptArpReply(const EthernetView& eth, const ArpView& arp, RTL8139& nic) {
        if (arp.op != ArpOp::Request) return false;
        if (arp.sha.size() != 6 || arp.spa.size() != 4 || arp.tha.size() != 6 || arp.tpa.size() != 4) return false;

        std::array<uint8_t, 6> local_mac;
        Memory::Copy(local_mac.data(), nic.GetMACAddress().data(), 6);

        ArpReplyBuilder rb;
        Memory::Copy(rb.sha.data(), local_mac.data(), 6);
        Memory::Copy(rb.spa.data(), arp.tpa.data(), 4);
        Memory::Copy(rb.tha.data(), arp.sha.data(), 6);
        Memory::Copy(rb.tpa.data(), arp.spa.data(), 4);

        std::array<uint8_t, 6> dst;
        Memory::Copy(dst.data(), arp.sha.data(), 6);

        auto eb = rb.ToEthernet(local_mac, dst);
        eb.send(nic);
        return true;
    }

    bool ReceiveUdpPayload(std::vector<uint8_t>& out, RTL8139& nic) {
        bool got = false;
        nic.read([&](Bytes pkt) {
            auto eth = ParseEthernet(pkt);
            if (!eth) return;
            auto parsed = ParsePayload(*eth);
            if (!parsed) return;

            if (auto ip = std::get_if<Ipv4View>(&*parsed)) {
                if (auto udp = ParseUdp(*ip)) {
                    out.assign(udp->data.begin(), udp->data.end());
                    got = true;
                }
            } else if (auto arp = std::get_if<ArpView>(&*parsed)) {
                AttemptArpReply(*eth, *arp, nic);
            }
        });
        return got;
    }

} // namespace Net