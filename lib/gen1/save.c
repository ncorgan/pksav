/*
 * Copyright (c) 2016 Nicholas Corgan (n.corgan@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

#include <pksav/gen1/save.h>

#include <stdio.h>
#include <string.h>
#include <wchar.h>

static uint8_t _pksav_get_gen1_save_checksum(
    const uint8_t* data
) {
    uint8_t checksum = 255;
    for(uint16_t i = 0x2598; i < PKSAV_GEN1_CHECKSUM; ++i) {
        checksum -= data[i];
    }

    return checksum;
}

static bool _pksav_file_is_gen1_save(
    const uint8_t* data
) {
    uint8_t checksum = data[PKSAV_GEN1_CHECKSUM];
    uint8_t actual_checksum = _pksav_get_gen1_save_checksum(data);

    return (checksum == actual_checksum);
}

bool pksav_file_is_gen1_save(
    const char* filepath
) {
    FILE* gen1_save = fopen(filepath, "r");
    if(!gen1_save) {
        return false;
    }

    fseek(gen1_save, 0, SEEK_END);

    if(ftell(gen1_save) < PKSAV_GEN1_SAVE_SIZE) {
        fclose(gen1_save);
        return false;
    }

    uint8_t* gen1_save_data = malloc(PKSAV_GEN1_SAVE_SIZE);
    fseek(gen1_save, 0, SEEK_SET);
    size_t num_read = fread((void*)gen1_save_data, 1, PKSAV_GEN1_SAVE_SIZE, gen1_save);
    fclose(gen1_save);

    bool ret = false;
    if(num_read == PKSAV_GEN1_SAVE_SIZE) {
        ret = _pksav_file_is_gen1_save(gen1_save_data);
    }

    free(gen1_save_data);
    return ret;
}

pksav_error_t pksav_gen1_save_load(
    const char* filepath,
    pksav_gen1_save_t* gen1_save
) {
    // Read the file and make sure it's valid
    FILE* gen1_save_file = fopen(filepath, "r");
    if(!gen1_save_file) {
        return PKSAV_ERROR_FILE_IO;
    }

    fseek(gen1_save_file, 0, SEEK_END);

    if(ftell(gen1_save_file) < PKSAV_GEN1_SAVE_SIZE) {
        fclose(gen1_save_file);
        return PKSAV_ERROR_INVALID_SAVE;
    }

    gen1_save->raw = malloc(PKSAV_GEN1_SAVE_SIZE);
    fseek(gen1_save_file, 0, SEEK_SET);
    size_t num_read = fread((void*)gen1_save->raw, 1, PKSAV_GEN1_SAVE_SIZE, gen1_save_file);
    fclose(gen1_save_file);
    if(num_read != PKSAV_GEN1_SAVE_SIZE) {
        return PKSAV_ERROR_FILE_IO;
    }

    if(!_pksav_file_is_gen1_save(gen1_save->raw)) {
        free(gen1_save->raw);
        return PKSAV_ERROR_INVALID_SAVE;
    }

    /*
     * Check if this save is for the Yellow version. The only way to check this is to check the
     * Pikachu Friendship field, which isn't used in Red/Blue. This is usually fine but will fail
     * if the trainer's Pikachu despises the trainer enough to have a friendship value of 0, which
     * is unlikely but technically possible.
     */
    gen1_save->yellow = (gen1_save->raw[PKSAV_GEN1_PIKACHU_FRIENDSHIP] > 0);

    // Set pointers
    gen1_save->pokemon_party = (pksav_gen1_pokemon_party_t*)&gen1_save->raw[PKSAV_GEN1_POKEMON_PARTY];

    for(uint8_t i = 0; i < 6; ++i) {
        uint16_t offset = PKSAV_GEN1_POKEMON_PC_FIRST_HALF + (sizeof(pksav_gen1_pokemon_box_t)*i);
        gen1_save->pokemon_boxes[i] = (pksav_gen1_pokemon_box_t*)&gen1_save->raw[offset];
    }
    for(uint8_t i = 6; i < 12; ++i) {
        uint16_t offset = PKSAV_GEN1_POKEMON_PC_SECOND_HALF + (sizeof(pksav_gen1_pokemon_box_t)*(i-6));
        gen1_save->pokemon_boxes[i] = (pksav_gen1_pokemon_box_t*)&gen1_save->raw[offset];
    }

    gen1_save->pokedex_seen = &gen1_save->raw[PKSAV_GEN1_POKEDEX_SEEN];
    gen1_save->pokedex_owned = &gen1_save->raw[PKSAV_GEN1_POKEDEX_OWNED];
    gen1_save->item_bag = (pksav_gen1_item_bag_t*)&gen1_save->raw[PKSAV_GEN1_ITEM_BAG];
    gen1_save->item_pc = (pksav_gen1_item_pc_t*)&gen1_save->raw[PKSAV_GEN1_ITEM_PC];
    gen1_save->time_played = (pksav_gen1_time_t*)&gen1_save->raw[PKSAV_GEN1_TIME_PLAYED];
    gen1_save->money = &gen1_save->raw[PKSAV_GEN1_MONEY];
    gen1_save->casino_coins = &gen1_save->raw[PKSAV_GEN1_CASINO_COINS];
    gen1_save->trainer_id = (uint16_t*)&gen1_save->raw[PKSAV_GEN1_PLAYER_ID];
    gen1_save->trainer_name = &gen1_save->raw[PKSAV_GEN1_PLAYER_NAME];
    gen1_save->rival_name = &gen1_save->raw[PKSAV_GEN1_RIVAL_NAME];
    gen1_save->badges = &gen1_save->raw[PKSAV_GEN1_BADGES];
    gen1_save->pikachu_friendship = &gen1_save->raw[PKSAV_GEN1_PIKACHU_FRIENDSHIP];

    return PKSAV_ERROR_NONE;
}

pksav_error_t pksav_gen1_save_save(
    const char* filepath,
    pksav_gen1_save_t* gen1_save
) {
    // Make sure we can write to this file
    FILE* gen1_save_file = fopen(filepath, "w");
    if(!gen1_save_file) {
        return PKSAV_ERROR_FILE_IO;
    }

    // Set checksum
    gen1_save->raw[PKSAV_GEN1_CHECKSUM] = _pksav_get_gen1_save_checksum(gen1_save->raw);

    // Write to file
    fwrite((void*)gen1_save->raw, 1, PKSAV_GEN1_SAVE_SIZE, gen1_save_file);
    fclose(gen1_save_file);

    return PKSAV_ERROR_NONE;
}
