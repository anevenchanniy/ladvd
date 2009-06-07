/*
 $Id$
*/

#include "common.h"
#include "util.h"
#include "proto/fdp.h"
#include "proto/cdp.h"
#include "proto/tlv.h"

size_t fdp_packet(void *packet, struct netif *netif, struct sysinfo *sysinfo) {

    struct ether_hdr ether;
    struct ether_llc llc;
    struct fdp_header fdp;

    char *tlv;
    char *pos = packet;
    size_t length = ETHER_MAX_LEN;
    tlv_t type;

    void *fdp_start, *cap_str;
    uint8_t addr_count = 0;
    struct netif *master;

    const uint8_t fdp_dst[] = FDP_MULTICAST_ADDR;
    const uint8_t llc_org[] = LLC_ORG_FOUNDRY;
    const struct cdp_proto fdp_protos[] = {
	ADDR_PROTO_CLNP, ADDR_PROTO_IPV4, ADDR_PROTO_IPV6,
    };

    // fixup master netif
    if (netif->master != NULL)
	master = netif->master;
    else
	master = netif;

    // ethernet header
    memcpy(ether.dst, fdp_dst, ETHER_ADDR_LEN);
    memcpy(ether.src, netif->hwaddr, ETHER_ADDR_LEN);
    pos += sizeof(struct ether_hdr);

    // llc snap header
    llc.dsap = llc.ssap = 0xaa;
    llc.control = 0x03;
    memcpy(llc.org, llc_org, sizeof(llc.org));
    llc.protoid = htons(LLC_PID_FDP);
    memcpy(pos, &llc, sizeof(struct ether_llc));
    pos += sizeof(struct ether_llc);

    // fdp header
    fdp.version = FDP_VERSION;
    fdp.ttl = LADVD_TTL;
    fdp.checksum = 0;
    memcpy(pos, &fdp, sizeof(struct fdp_header));
    fdp_start = pos;

    // update tlv counters
    pos += sizeof(struct fdp_header);
    length -= VOIDP_DIFF(pos, packet);


    // device id
    if (!(
	START_FDP_TLV(FDP_TYPE_DEVICE_ID) &&
	PUSH_BYTES(sysinfo->hostname, strlen(sysinfo->hostname))
    ))
	return 0;
    END_FDP_TLV;


    // port id
    if (!(
	START_FDP_TLV(FDP_TYPE_PORT_ID) &&
	PUSH_BYTES(netif->name, strlen(netif->name))
    ))
	return 0;
    END_FDP_TLV;


    // capabilities
    if (sysinfo->cap_active & CAP_ROUTER)
	cap_str = "Router";
    else if (sysinfo->cap_active & CAP_SWITCH)
	cap_str = "Switch";
    else if (sysinfo->cap_active & CAP_BRIDGE)
	cap_str = "Bridge";
    else
	cap_str = "Host";

    if (!(
	START_FDP_TLV(FDP_TYPE_CAPABILITIES) &&
	PUSH_BYTES(cap_str, strlen(cap_str))
    ))
	return 0;
    END_FDP_TLV;


    // version
    if (!(
	START_FDP_TLV(FDP_TYPE_SW_VERSION) &&
	PUSH_BYTES(sysinfo->uts_str, strlen(sysinfo->uts_str))
    ))
	return 0;
    END_FDP_TLV;


    // platform
    if (!(
	START_FDP_TLV(FDP_TYPE_PLATFORM) &&
	PUSH_BYTES(sysinfo->uts.sysname, strlen(sysinfo->uts.sysname))
    ))
	return 0;
    END_FDP_TLV;


    // management addrs
    if (master->ipaddr4 != 0)
	addr_count++;
    if (!IN6_IS_ADDR_UNSPECIFIED((struct in6_addr *)master->ipaddr6)) 
	addr_count++;

    if (addr_count > 0) {
	if (!(
	    START_FDP_TLV(FDP_TYPE_ADDRESS) &&
	    PUSH_UINT32(addr_count)
	))
	    return 0;

	if (master->ipaddr4 != 0) {
	    if (!(
		PUSH_UINT8(fdp_protos[CDP_ADDR_PROTO_IPV4].protocol_type) &&
		PUSH_UINT8(fdp_protos[CDP_ADDR_PROTO_IPV4].protocol_length) &&
		PUSH_BYTES(fdp_protos[CDP_ADDR_PROTO_IPV4].protocol,
			   fdp_protos[CDP_ADDR_PROTO_IPV4].protocol_length) &&
		PUSH_UINT16(sizeof(master->ipaddr4)) &&
		PUSH_BYTES(&master->ipaddr4, sizeof(master->ipaddr4))
	    ))
		return 0;
	}

	if (!IN6_IS_ADDR_UNSPECIFIED((struct in6_addr *)master->ipaddr6)) {
	    if (!(
		PUSH_UINT8(fdp_protos[CDP_ADDR_PROTO_IPV6].protocol_type) &&
		PUSH_UINT8(fdp_protos[CDP_ADDR_PROTO_IPV6].protocol_length) &&
		PUSH_BYTES(fdp_protos[CDP_ADDR_PROTO_IPV6].protocol,
			   fdp_protos[CDP_ADDR_PROTO_IPV6].protocol_length) &&
		PUSH_UINT16(sizeof(master->ipaddr6)) &&
		PUSH_BYTES(master->ipaddr6, sizeof(master->ipaddr6))
	    ))
		return 0;
	}

	END_FDP_TLV;
    }


    // fdp header
    fdp.checksum = my_chksum(fdp_start, VOIDP_DIFF(pos, fdp_start), 0);
    memcpy(fdp_start, &fdp, sizeof(struct fdp_header));

    // ethernet header
    ether.length = htons(VOIDP_DIFF(pos, packet + sizeof(struct ether_hdr)));
    memcpy(packet, &ether, sizeof(struct ether_hdr));

    // packet length
    return(VOIDP_DIFF(pos, packet));
}

char * fdp_check(void *packet, size_t length) {
    struct ether_hdr ether;
    struct ether_llc llc;
    const uint8_t fdp_dst[] = FDP_MULTICAST_ADDR;
    const uint8_t fdp_org[] = LLC_ORG_FOUNDRY;

    assert(packet);
    assert(length > (sizeof(ether) + sizeof(llc)));
    assert(length <= ETHER_MAX_LEN);

    memcpy(&ether, packet, sizeof(ether));
    memcpy(&llc, packet + sizeof(ether), sizeof(llc));

    if ((memcmp(ether.dst, fdp_dst, ETHER_ADDR_LEN) == 0) &&
	(memcmp(llc.org, fdp_org, sizeof(llc.org)) == 0) &&
	(llc.protoid == htons(LLC_PID_FDP))) {
	    return(packet + sizeof(ether) + sizeof(llc));
    } 
    return(NULL);
}

size_t fdp_peer(struct master_msg *msg) {
    char *packet = NULL;
    size_t length;
    struct fdp_header fdp;

    char *tlv;
    char *pos;

    uint16_t tlv_type;
    uint16_t tlv_length;

    char *hostname = NULL;

    assert(msg);

    packet = msg->msg;
    length = msg->len;

    assert(packet);
    assert((pos = fdp_check(packet, length)) != NULL);
    length -= VOIDP_DIFF(pos, packet);
    if (length < sizeof(fdp)) {
	my_log(INFO, "missing FDP header");
	return 0;
    }

    memcpy(&fdp, pos, sizeof(fdp));
    if (fdp.version != 1) {
	my_log(INFO, "unsupported FDP version");
	return 0;
    }
    msg->ttl = fdp.ttl;

    // update tlv counters
    pos += sizeof(fdp);
    length -= sizeof(fdp);

    while (length) {
	if (!GRAB_FDP_TLV(tlv_type, tlv_length)) {
	    my_log(INFO, "Corrupt FDP packet: invalid TLV");
	    return 0;
	}

	switch(tlv_type) {
	case FDP_TYPE_DEVICE_ID:
		if (!GRAB_STRING(hostname, tlv_length)) {
		    my_log(INFO, "Corrupt FDP packet: invalid Device ID TLV");
		    return 0;
		}
		strlcpy(msg->peer, hostname, IFDESCRSIZE);
		free(hostname);
		break;
	default:
		my_log(INFO, "unknown TLV: type %d, length %d, leaves %d",
			    tlv_type, tlv_length, length);
		if (!SKIP(tlv_length)) {
		    my_log(INFO, "Corrupt FDP packet: invalid TLV length");
		    return 0;
		}
		break;
	}
    }

    // return the packet length
    return(VOIDP_DIFF(pos, packet));
}
