/*
 * Copyright (c) 2016 Nicholas Corgan (n.corgan@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#ifndef PKSAV_GEN6_ITEMS_H
#define PKSAV_GEN6_ITEMS_H

#include <stdint.h>

#pragma pack(push,1)

typedef struct {
    uint16_t index;
    uint16_t count;
} pksav_gen6_item_t;

typedef struct {
    pksav_gen6_item_t items[310];
    pksav_gen6_item_t key_items[83];
    pksav_gen6_item_t tms_hms[109];
    pksav_gen6_item_t medicine[48];
    pksav_gen6_item_t berries[64];
    uint8_t unknown[0x66];
    uint16_t checksum;
} pksav_gen6_bag_t;

#pragma pack(pop)

#endif /* PKSAV_GEN6_ITEMS_H */