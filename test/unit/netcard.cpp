#include <gtest/gtest.h>
#include <sg/hdw/netcard.h>

void callback(struct sg_net_card_info* inf, void* ctx)
{
    printf("if: %s\n", inf->name);
    printf("\tmac: %s\n", inf->mac);
    printf("\tmtu: %d\n", inf->mtu);
    printf("\tipv4: %s\n", inf->lan_ipv4);
    printf("\tmask v4: %s\n", inf->net_mask_ipv4);
    printf("\tbroadcast v4: %s\n", inf->broadcast_ipv4);
    printf("\tipv6: %s\n", inf->lan_ipv6);
    printf("\tmask v6: %s\n", inf->net_mask_ipv6);
    printf("\tbroadcast v6: %s\n", inf->broadcast_ipv6);
}

TEST(test_net_card, list) { ASSERT_EQ(0, sg_net_card_scan(callback, NULL, 1)); }
