
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <check.h>

#include "../src/common.h"
#include "../src/util.h"
#include "../src/proto/protos.h"

extern uint8_t loglevel;
uint32_t options = OPT_DEBUG;

START_TEST(test_lldp_check) {
    struct master_msg msg;
    struct ether_hdr ether;
    static uint8_t lldp_dst[] = LLDP_MULTICAST_ADDR;

    msg.len = ETHER_MIN_LEN;
    fail_unless (lldp_check(msg.msg, msg.len) == NULL,
	    "empty packets should generate a NULL");

    memcpy(ether.dst, lldp_dst, ETHER_ADDR_LEN);
    memcpy(msg.msg, &ether, sizeof(ether));
    fail_unless (lldp_check(msg.msg, msg.len) == NULL,
	    "packets without an ethertype should generate a NULL");

    ether.type = htons(ETHERTYPE_LLDP);
    memcpy(msg.msg, &ether, sizeof(ether));
    fail_unless (lldp_check(msg.msg, msg.len) == msg.msg + sizeof(ether),
	    "valid packets should return a correct ptr");
}
END_TEST

START_TEST(test_cdp_check) {
    struct master_msg msg;
    struct ether_hdr ether;
    struct ether_llc llc;
    static uint8_t cdp_dst[] = CDP_MULTICAST_ADDR;
    static uint8_t cdp_org[] = LLC_ORG_CISCO;

    msg.len = ETHER_MIN_LEN;
    fail_unless (cdp_check(msg.msg, msg.len) == NULL,
	    "empty packets should generate a NULL");

    memcpy(ether.dst, cdp_dst, ETHER_ADDR_LEN);
    memcpy(msg.msg, &ether, sizeof(ether));
    fail_unless (cdp_check(msg.msg, msg.len) == NULL,
	    "packets without an llc header should generate a NULL");

    memcpy(llc.org, cdp_org, sizeof(llc.org));
    memcpy(msg.msg + sizeof(ether), &llc, sizeof(llc));
    fail_unless (cdp_check(msg.msg, msg.len) == NULL,
	    "packets with an invalid llc header should generate a NULL");

    llc.protoid = htons(LLC_PID_CDP);
    memcpy(msg.msg + sizeof(ether), &llc, sizeof(llc));
    fail_unless (cdp_check(msg.msg, msg.len) == 
		 msg.msg + sizeof(ether) + sizeof(llc),
	    "valid packets should return a correct ptr");
}
END_TEST

START_TEST(test_edp_check) {
    struct master_msg msg;
    struct ether_hdr ether;
    struct ether_llc llc;
    static uint8_t edp_dst[] = EDP_MULTICAST_ADDR;
    static uint8_t edp_org[] = LLC_ORG_EXTREME;

    msg.len = ETHER_MIN_LEN;
    fail_unless (edp_check(msg.msg, msg.len) == NULL,
	    "empty packets should generate a NULL");

    memcpy(ether.dst, edp_dst, ETHER_ADDR_LEN);
    memcpy(msg.msg, &ether, sizeof(ether));
    fail_unless (edp_check(msg.msg, msg.len) == NULL,
	    "packets without an llc header should generate a NULL");

    memcpy(llc.org, edp_org, sizeof(llc.org));
    memcpy(msg.msg + sizeof(ether), &llc, sizeof(llc));
    fail_unless (edp_check(msg.msg, msg.len) == NULL,
	    "packets with an invalid llc header should generate a NULL");

    llc.protoid = htons(LLC_PID_EDP);
    memcpy(msg.msg + sizeof(ether), &llc, sizeof(llc));
    fail_unless (edp_check(msg.msg, msg.len) == 
		 msg.msg + sizeof(ether) + sizeof(llc),
	    "valid packets should return a correct ptr");
}
END_TEST

START_TEST(test_fdp_check) {
    struct master_msg msg;
    struct ether_hdr ether;
    struct ether_llc llc;
    static uint8_t fdp_dst[] = FDP_MULTICAST_ADDR;
    static uint8_t fdp_org[] = LLC_ORG_FOUNDRY;

    msg.len = ETHER_MIN_LEN;
    fail_unless (fdp_check(msg.msg, msg.len) == NULL,
	    "empty packets should generate a NULL");

    memcpy(ether.dst, fdp_dst, ETHER_ADDR_LEN);
    memcpy(msg.msg, &ether, sizeof(ether));
    fail_unless (fdp_check(msg.msg, msg.len) == NULL,
	    "packets without an llc header should generate a NULL");

    memcpy(llc.org, fdp_org, sizeof(llc.org));
    memcpy(msg.msg + sizeof(ether), &llc, sizeof(llc));
    fail_unless (fdp_check(msg.msg, msg.len) == NULL,
	    "packets with an invalid llc header should generate a NULL");

    llc.protoid = htons(LLC_PID_FDP);
    memcpy(msg.msg + sizeof(ether), &llc, sizeof(llc));
    fail_unless (fdp_check(msg.msg, msg.len) == 
		 msg.msg + sizeof(ether) + sizeof(llc),
	    "valid packets should return a correct ptr");
}
END_TEST

START_TEST(test_ndp_check) {
    struct master_msg msg;
    struct ether_hdr ether;
    struct ether_llc llc;
    static uint8_t ndp_dst[] = NDP_MULTICAST_ADDR;
    static uint8_t ndp_org[] = LLC_ORG_NORTEL;

    msg.len = ETHER_MIN_LEN;
    fail_unless (ndp_check(msg.msg, msg.len) == NULL,
	    "empty packets should generate a NULL");

    memcpy(ether.dst, ndp_dst, ETHER_ADDR_LEN);
    memcpy(msg.msg, &ether, sizeof(ether));
    fail_unless (ndp_check(msg.msg, msg.len) == NULL,
	    "packets without an llc header should generate a NULL");

    memcpy(llc.org, ndp_org, sizeof(llc.org));
    memcpy(msg.msg + sizeof(ether), &llc, sizeof(llc));
    fail_unless (ndp_check(msg.msg, msg.len) == NULL,
	    "packets with an invalid llc header should generate a NULL");

    llc.protoid = htons(LLC_PID_NDP_HELLO);
    memcpy(msg.msg + sizeof(ether), &llc, sizeof(llc));
    fail_unless (ndp_check(msg.msg, msg.len) == 
		 msg.msg + sizeof(ether) + sizeof(llc),
	    "valid packets should return a correct ptr");
}
END_TEST

void read_packet(struct master_msg *msg, const char *path) {
    int fd;

    memset(msg->msg, 0, ETHER_MAX_LEN);
    msg->len = 0;
    msg->ttl = 0;
    memset(msg->peer, 0, IFDESCRSIZE);

    if ((fd = open(path, O_RDONLY)) == -1)
	my_fatal("failed to open %s", path);
    msg->len = read(fd, msg->msg, ETHER_MAX_LEN);
}

START_TEST(test_lldp_peer) {
    struct master_msg msg;

    read_packet(&msg, "proto/lldp/00.empty");
    fail_unless (lldp_peer(&msg) == 0, "empty packets should return 0");

    read_packet(&msg, "proto/lldp/01.chassis_id.broken");
    fail_unless (lldp_peer(&msg) == 0, "broken packets should return 0");
    read_packet(&msg, "proto/lldp/02.chassis_id.only");
    fail_unless (lldp_peer(&msg) == 0, "incomplete packets should return 0");
    read_packet(&msg, "proto/lldp/03.chassis_id.missing");
    fail_unless (lldp_peer(&msg) == 0, "incomplete packets should return 0");

    read_packet(&msg, "proto/lldp/11.port_id.broken");
    fail_unless (lldp_peer(&msg) == 0, "broken packets should return 0");
    read_packet(&msg, "proto/lldp/12.port_id.only");
    fail_unless (lldp_peer(&msg) == 0, "incomplete packets should return 0");
    read_packet(&msg, "proto/lldp/13.port_id.missing");
    fail_unless (lldp_peer(&msg) == 0, "incomplete packets should return 0");

    read_packet(&msg, "proto/lldp/21.ttl.broken");
    fail_unless (lldp_peer(&msg) == 0, "broken packets should return 0");
    read_packet(&msg, "proto/lldp/22.ttl.only");
    fail_unless (lldp_peer(&msg) == 0, "incomplete packets should return 0");
    read_packet(&msg, "proto/lldp/23.ttl.missing");
    fail_unless (lldp_peer(&msg) == 0, "incomplete packets should return 0");

    read_packet(&msg, "proto/lldp/31.system_name.dup");
    fail_unless (lldp_peer(&msg) == 0, "broken packets should return 0");
    read_packet(&msg, "proto/lldp/32.system_name.broken");
    fail_unless (lldp_peer(&msg) == 0, "broken packets should return 0");

    read_packet(&msg, "proto/lldp/96.tlv.unknown");
    fail_unless (lldp_peer(&msg) == 0, "unknown tlv's should return 0");
    read_packet(&msg, "proto/lldp/97.tlv.broken");
    fail_unless (lldp_peer(&msg) == 0, "broken packets should return 0");
    read_packet(&msg, "proto/lldp/98.end.broken");
    fail_unless (lldp_peer(&msg) == 0, "broken packets should return 0");
    read_packet(&msg, "proto/lldp/99.end.missing");
    fail_unless (lldp_peer(&msg) == 0, "incomplete packets should return 0");

    read_packet(&msg, "proto/lldp/41.good.small");
    fail_unless (lldp_peer(&msg) == 41, "packet length should be 41");
    fail_unless (msg.ttl == 120, "ttl should be 120");
    fail_unless (strcmp(msg.peer, "test") == 0, "system name should be 'test'");
    read_packet(&msg, "proto/lldp/42.good.big");
    fail_unless (lldp_peer(&msg) == 263, "packet length should be 263");
    fail_unless (msg.ttl == 120, "ttl should be 120");
    fail_unless (strcmp(msg.peer, "Summit300-48") == 0,
		"system name should be 'Summit300-48'");
    read_packet(&msg, "proto/lldp/43.good.lldpmed");
    fail_unless (lldp_peer(&msg) == 268, "packet length should be 41");
    fail_unless (msg.ttl == 120, "ttl should be 120");
    fail_unless (strcmp(msg.peer, "ProCurve Switch 2600-8-PWR") == 0,
		"system name should be 'ProCurve Switch 2600-8-PWR'");
}
END_TEST

Suite * proto_suite (void) {
    Suite *s = suite_create("libproto");
    loglevel = DEBUG;

    // proto_check test cases
    TCase *tc_check = tcase_create("proto_check");
    tcase_add_test(tc_check, test_lldp_check);
    tcase_add_test(tc_check, test_cdp_check);
    tcase_add_test(tc_check, test_edp_check);
    tcase_add_test(tc_check, test_fdp_check);
    tcase_add_test(tc_check, test_ndp_check);
    suite_add_tcase(s, tc_check);

    // proto_peer test cases
    TCase *tc_peer = tcase_create("proto_peer");
    tcase_add_test(tc_peer, test_lldp_peer);
    suite_add_tcase(s, tc_peer);

    return s;
}

int main (void) {
    int number_failed;
    Suite *s = proto_suite ();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
