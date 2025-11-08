#pragma once

#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <array>
#include <optional>
#include <variant>
#include <span>

#include <core/ZosDefs.hpp>
#include <core/Debug.hpp>
#include <core/dev/RTL8139.hpp>

namespace Net {
    using Bytes = std::span<const uint8_t>;
    using WBytes = std::span<uint8_t>;

    enum class EtherType    : uint16_t { Ipv4 = 0x0800, Arp = 0x0806, Dot1Q = 0x8100, };
    enum class ArpOp        : uint16_t { Request = 1, Reply = 2, };
    enum class IpProto      : uint16_t { Tcp = 0x06, Udp = 0x11, };

    struct EthernetHeader {
        std::array<uint8_t, 6> DestinationMAC;
        std::array<uint8_t, 6> SourceMAC;
        uint16_t type_be;
    } PACKED;

    struct ArpHeader {
        uint16_t htype_be;
        uint16_t ptype_be;
        uint8_t hlen;
        uint8_t plen;
        uint16_t op_be;
    } PACKED;

    struct Ipv4Header {
        uint8_t ver_ihl;
        uint8_t dscp_ecn;
        uint16_t total_len_be;
        uint16_t id_be;
        uint16_t flags_frag_be;
        uint16_t ttl;
        uint8_t proto;
        uint16_t hdr_ck_be;
        uint32_t src_be;
        uint32_t dst_be;
    } PACKED;

    struct UdpHeader {
        uint16_t src_be;
        uint16_t dst_be;
        uint16_t len_be;
        uint16_t csum_be;
    } PACKED;

    // --- Views ---
    struct EthernetView {
        const EthernetHeader* hdr{};
        Bytes payload{};
        EtherType type{};
        bool dot1q{ false };
    };

    struct ArpView {
        const ArpHeader* hdr{};
        Bytes sha{}, spa{}, tha{}, tpa{};
        ArpOp op{};
    };

    struct Ipv4View {
        const Ipv4Header* hdr{};
        Bytes options{};
        Bytes payload{};
        IpProto proto{};
        uint8_t ihl_bytes{};
        uint16_t total_len{};
    };

    struct UdpView {
        const UdpHeader* hdr{};
        Bytes data{};   // UDP payload
    };

    using Parsed = std::variant<ArpView, Ipv4View>;

    std::optional<EthernetView> ParseEthernet(Bytes);
    std::optional<Parsed>       ParsePayload(const EthernetView&);
    std::optional<UdpView>      ParseUdp(const Ipv4View&);

    uint16_t Ipv4HeaderChecksum(Bytes);
    uint16_t FoldChecksum(uint32_t);

    struct EthernetBuilder {
        std::array<uint8_t, 6> DestinationMAC{}, SourceMAC{};
        EtherType type{ EtherType::Ipv4 };
        std::vector<uint8_t> payload;

        void send(RTL8139&) const;
    };

    struct ArpReplyBuilder {
        std::array<uint8_t, 6> sha{};
        std::array<uint8_t, 4> spa{};
        std::array<uint8_t, 6> tha{};
        std::array<uint8_t, 4> tpa{};

        EthernetBuilder ToEthernet(const std::array<uint8_t, 6>&,
                                   const std::array<uint8_t, 6>&);
    };

    bool ReceiveUdpPayload(std::vector<uint8_t>&, RTL8139& nic);
}