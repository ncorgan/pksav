/*
 * Copyright (c) 2016 Nicholas Corgan (n.corgan@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#ifndef PKSAV_PRNG_H
#define PKSAV_PRNG_H

#include <pksav/config.h>

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static PKSAV_INLINE uint32_t arng_next(
    uint32_t seed
) {
    return (0x6C078965 * seed) + 1;
}

static PKSAV_INLINE uint32_t lcrng32_next(
    uint32_t seed
) {
    return (0x41C64E6D * seed) + 0x6073;
}

static PKSAV_INLINE uint64_t lcrng64_next(
    uint64_t seed
) {
    return (0x5D588B656C078965UL * seed) + 0x269EC3;
}

typedef struct {
    uint32_t nums[624];
    size_t index;
} pksav_mtrng_t;

PKSAV_API void pksav_mtrng_populate(
    pksav_mtrng_t* mtrng
);

PKSAV_API uint32_t pksav_mtrng_next(
    pksav_mtrng_t* mtrng
);

#ifdef __cplusplus
}
#endif

#endif /* PKSAV_PRNG_H */
