/*
 * rewritten versions of some NET::CDP macro's (libcdp/src/encoding.c)
 * VOIDP_DIFF: fixed types
 * PUSH, END_TLV: use memcpy to make them strict alignment compatible
 * added support for LLDP tlv's (7/9 bits)
 * added support for EDP tlv's (0x99 marker)
 * added extended GRAB macros
 */

#define VOIDP_DIFF(P, Q) ((uintptr_t)((char *)(P) - (char *)(Q)))

typedef union {
    uint8_t uint8;
    uint16_t uint16;
    uint32_t uint32;
} tlv_t;

#define PUSH(v, t, func) \
	((length >= sizeof(t)) && \
	    ( \
		t = func(v), \
		memcpy(pos, &t, sizeof(t)), \
		length -= sizeof(t), \
		pos += sizeof(t), \
		1 \
	    ))
#define PUSH_UINT8(value) PUSH(value, type.uint8, )
#define PUSH_UINT16(value) PUSH(value, type.uint16, htons)
#define PUSH_UINT32(value) PUSH(value, type.uint32, htonl)
#define PUSH_BYTES(value, bytes) \
	((length >= (bytes)) && \
	    ( \
		memcpy(pos, value, (bytes) * sizeof(uint8_t)), \
		length -= (bytes), \
		pos += (bytes), \
		1 \
	    ))

#define GRAB(d, t, func) \
	((length >= sizeof(t)) && \
	    ( \
		d = func(*((t *)pos)), \
		length -= sizeof(t), \
		pos += sizeof(t), \
		1 \
            ))
#define GRAB_UINT8(d) GRAB(d, uint8_t, )
#define GRAB_UINT16(d) GRAB(d, uint16_t, ntohs)
#define GRAB_UINT32(d) GRAB(d, uint32_t, ntohl)
#define GRAB_STRING(d, b) \
	((length >= (b)) && \
	    ( \
		d = my_malloc((b) + 1), \
		memcpy((d), pos, (b) * sizeof(char)), \
		*(d + b) = '\0', \
		length -= (b), \
		pos += (b), \
		1 \
	    ))
#define SKIP(b) \
	((length >= (b)) && \
	    ( \
		length -= (b), \
		pos += (b), \
		1 \
	    ))

#define START_CDP_TLV(t) \
	( \
	    tlv = pos, \
	    PUSH_UINT16(t) && PUSH_UINT16(0) \
	)
#define END_CDP_TLV \
	( \
	    type.uint16 = htons(pos - tlv), \
	    memcpy((uint16_t *)tlv + 1, &type.uint16, sizeof(uint16_t)) \
	)
#define GRAB_CDP_TLV(t, l) \
	( \
	    tlv = pos, \
	    GRAB_UINT16(t) && GRAB_UINT16(l) && \
	    (l -= 2 * sizeof(uint16_t)) \
	)

#define START_LLDP_TLV(t) \
	( \
	    tlv = pos, \
	    PUSH_UINT16(t << 9) \
	)
#define END_LLDP_TLV \
	( \
	    memcpy(&type.uint16, tlv, sizeof(uint16_t)), \
	    type.uint16 |= htons((pos - (tlv + 2)) & 0x01ff), \
	    memcpy(tlv, &type.uint16, sizeof(uint16_t)) \
	)
#define GRAB_LLDP_TLV(t, l) \
	( \
	    tlv = pos, \
	    memcpy(&type.uint16, tlv, sizeof(uint16_t)), \
	    t = ntohs(type.uint16) >> 9, \
	    l = ntohs(type.uint16) & 0x01ff, \
	    SKIP(2) \
	)

#define EDP_TLV_MARKER   0x99
#define START_EDP_TLV(t) \
	( \
	    tlv = pos, \
	    PUSH_UINT8(EDP_TLV_MARKER) && PUSH_UINT8(t) && PUSH_UINT16(0) \
	)
#define END_EDP_TLV \
	( \
	    type.uint16 = htons(pos - tlv), \
	    memcpy((uint16_t *)tlv + 1, &type.uint16, sizeof(uint16_t)) \
	)
#define GRAB_EDP_TLV(t, l) \
	( \
	    tlv = pos, \
	    SKIP(1) && GRAB_UINT8(t) && GRAB_UINT16(l) &&\
	    (l -= 2 * sizeof(uint16_t)) \
	)

#define START_FDP_TLV(t) \
	( \
	    tlv = pos, \
	    PUSH_UINT16(t) && PUSH_UINT16(0) \
	)
#define END_FDP_TLV \
	( \
	    type.uint16 = htons(pos - tlv), \
	    memcpy((uint16_t *)tlv + 1, &type.uint16, sizeof(uint16_t)) \
	)
#define GRAB_FDP_TLV(t, l) \
	( \
	    tlv = pos, \
	    GRAB_UINT16(t) && GRAB_UINT16(l) &&\
	    (l -= 2 * sizeof(uint16_t)) \
	)
