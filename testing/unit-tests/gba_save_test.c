/*
 * Copyright (c) 2017-2018 Nicholas Corgan (n.corgan@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

#include "c_test_common.h"
#include "test-utils.h"

#include "gba/save_internal.h"
#include "util/fs.h"

#include <pksav/config.h>
#include <pksav/gba.h>

#include <stdio.h>
#include <string.h>

/*
 * Testing TODO: make sure saving alternates between slots
 */

#define STRBUFFER_LEN (64)

static const struct pksav_gba_save EMPTY_GBA_SAVE =
{
    .save_type = PKSAV_GBA_SAVE_TYPE_NONE,

    .p_time_played = NULL,

    .item_storage =
    {
        .p_bag = NULL,
        .p_pc  = NULL
    },

    .pokemon_storage =
    {
        .p_party = NULL,
        .p_pc    = NULL
    },

    .pokedex =
    {
        .p_seenA  = NULL,
        .p_seenB  = NULL,
        .p_seenC  = NULL,
        .p_owned  = NULL,
        .p_rse_nat_pokedex_unlockedA  = NULL,
        .p_frlg_nat_pokedex_unlockedA = NULL,
        .p_nat_pokedex_unlockedB      = NULL,
        .p_nat_pokedex_unlockedC      = NULL
    },

    .player_info =
    {
        .p_id     = NULL,
        .p_name   = NULL,
        .p_gender = NULL,
        .p_money  = NULL,
    },

    .misc_fields =
    {
        .p_casino_coins = NULL,
        .p_roamer = NULL,
        {},
        {},
        {}
    },

    .p_internal = NULL
};

/*
 * We don't care about the result of the function itself. As the buffer
 * is randomized, it will likely be false. This function is to make sure
 * running it on invalid input doesn't crash.
 */
static void pksav_gba_get_buffer_save_type_on_random_buffer_test()
{
    enum pksav_error error = PKSAV_ERROR_NONE;

    uint8_t buffer[PKSAV_GBA_SAVE_SIZE] = {0};
    for(size_t run_index = 0; run_index < FUZZING_TEST_NUM_ITERATIONS; ++run_index)
    {
        randomize_buffer(buffer, sizeof(buffer));

        enum pksav_gba_save_type save_type = PKSAV_GBA_SAVE_TYPE_NONE;
        error = pksav_gba_get_buffer_save_type(
                    buffer,
                    sizeof(buffer),
                    &save_type
                );
        PKSAV_TEST_ASSERT_SUCCESS(error);
    }
}

static void pksav_gba_get_buffer_save_type_test(
    const char* subdir,
    const char* save_name,
    enum pksav_gba_save_type expected_save_type
)
{
    TEST_ASSERT_NOT_NULL(subdir);
    TEST_ASSERT_NOT_NULL(save_name);
    TEST_ASSERT_TRUE(expected_save_type >= PKSAV_GBA_SAVE_TYPE_RS);
    TEST_ASSERT_TRUE(expected_save_type <= PKSAV_GBA_SAVE_TYPE_FRLG);

    char filepath[256] = {0};
    enum pksav_error error = PKSAV_ERROR_NONE;

    char* pksav_test_saves = getenv("PKSAV_TEST_SAVES");
    if(!pksav_test_saves)
    {
        TEST_FAIL_MESSAGE("Failed to get test save directory.");
    }

    snprintf(
        filepath, sizeof(filepath),
        "%s%s%s%s%s",
        pksav_test_saves, FS_SEPARATOR, subdir, FS_SEPARATOR, save_name
    );

    size_t filesize = 0;
    if(pksav_fs_filesize(filepath, &filesize))
    {
        TEST_FAIL_MESSAGE("Failed to get save file size.");
    }
    TEST_ASSERT_TRUE(filesize >= PKSAV_GBA_SAVE_SIZE);

    uint8_t* save_buffer = NULL;
    if(pksav_fs_read_file_to_buffer(filepath, &save_buffer, &filesize))
    {
        TEST_FAIL_MESSAGE("Failed to read save into buffer.");
    }
    TEST_ASSERT_TRUE(filesize >= PKSAV_GBA_SAVE_SIZE);

    enum pksav_gba_save_type save_type = PKSAV_GBA_SAVE_TYPE_NONE;
    error = pksav_gba_get_buffer_save_type(
                save_buffer,
                filesize,
                &save_type
            );
    PKSAV_TEST_ASSERT_SUCCESS(error);
    TEST_ASSERT_EQUAL(expected_save_type, save_type);

    free(save_buffer);
}

static void pksav_gba_get_file_save_type_test(
    const char* subdir,
    const char* save_name,
    enum pksav_gba_save_type expected_save_type
)
{
    TEST_ASSERT_NOT_NULL(subdir);
    TEST_ASSERT_NOT_NULL(save_name);
    TEST_ASSERT_TRUE(expected_save_type >= PKSAV_GBA_SAVE_TYPE_RS);
    TEST_ASSERT_TRUE(expected_save_type <= PKSAV_GBA_SAVE_TYPE_FRLG);

    char filepath[256] = {0};
    enum pksav_error error = PKSAV_ERROR_NONE;

    char* pksav_test_saves = getenv("PKSAV_TEST_SAVES");
    if(!pksav_test_saves)
    {
        TEST_FAIL_MESSAGE("Failed to get test save directory.");
    }

    snprintf(
        filepath, sizeof(filepath),
        "%s%s%s%s%s",
        pksav_test_saves, FS_SEPARATOR, subdir, FS_SEPARATOR, save_name
    );

    enum pksav_gba_save_type save_type = PKSAV_GBA_SAVE_TYPE_NONE;
    error = pksav_gba_get_file_save_type(
                filepath,
                &save_type
            );
    PKSAV_TEST_ASSERT_SUCCESS(error);
    TEST_ASSERT_EQUAL(expected_save_type, save_type);
}

static void gba_save_test(
    struct pksav_gba_save* p_gba_save,
    enum pksav_gba_save_type expected_save_type,
    const char* original_filepath,
    const char* save_name
)
{
    TEST_ASSERT_NOT_NULL(p_gba_save);
    TEST_ASSERT_NOT_NULL(original_filepath);
    TEST_ASSERT_NOT_NULL(save_name);

    char tmp_save_filepath[256] = {0};
    enum pksav_error error = PKSAV_ERROR_NONE;

    snprintf(
        tmp_save_filepath, sizeof(tmp_save_filepath),
        "%s%spksav_%d_%s",
        get_tmp_dir(), FS_SEPARATOR, get_pid(), save_name
    );

    // Validate fields. Most pointers should not be NULL, and some fields have
    // a specific set of valid values.
    TEST_ASSERT_NOT_NULL(p_gba_save->p_internal);

    TEST_ASSERT_EQUAL(expected_save_type, p_gba_save->save_type);

    // Free the save and make sure all fields are set to NULL or default.
    error = pksav_gba_free_save(p_gba_save);
    PKSAV_TEST_ASSERT_SUCCESS(error);

    TEST_ASSERT_EQUAL(PKSAV_GBA_SAVE_TYPE_NONE, p_gba_save->save_type);

    TEST_ASSERT_NULL(p_gba_save->p_time_played);

    TEST_ASSERT_NULL(p_gba_save->item_storage.p_bag);
    TEST_ASSERT_NULL(p_gba_save->item_storage.p_pc);

    TEST_ASSERT_NULL(p_gba_save->pokemon_storage.p_party);
    TEST_ASSERT_NULL(p_gba_save->pokemon_storage.p_pc);

    TEST_ASSERT_NULL(p_gba_save->pokedex.p_seenA);
    TEST_ASSERT_NULL(p_gba_save->pokedex.p_seenB);
    TEST_ASSERT_NULL(p_gba_save->pokedex.p_seenC);
    TEST_ASSERT_NULL(p_gba_save->pokedex.p_owned);
    TEST_ASSERT_NULL(p_gba_save->pokedex.p_rse_nat_pokedex_unlockedA);
    TEST_ASSERT_NULL(p_gba_save->pokedex.p_frlg_nat_pokedex_unlockedA);
    TEST_ASSERT_NULL(p_gba_save->pokedex.p_nat_pokedex_unlockedB);
    TEST_ASSERT_NULL(p_gba_save->pokedex.p_nat_pokedex_unlockedC);

    TEST_ASSERT_NULL(p_gba_save->player_info.p_id);
    TEST_ASSERT_NULL(p_gba_save->player_info.p_name);
    TEST_ASSERT_NULL(p_gba_save->player_info.p_money);

    //TEST_ASSERT_NULL(p_gba_save->misc_fields.p_rival_name);
    TEST_ASSERT_NULL(p_gba_save->misc_fields.p_casino_coins);

    TEST_ASSERT_NULL(p_gba_save->p_internal);
}

static void gba_save_from_buffer_test(
    const char* subdir,
    const char* save_name,
    enum pksav_gba_save_type expected_save_type
)
{
    TEST_ASSERT_NOT_NULL(subdir);
    TEST_ASSERT_NOT_NULL(save_name);

    char original_filepath[256] = {0};
    struct pksav_gba_save gba_save = EMPTY_GBA_SAVE;
    enum pksav_error error = PKSAV_ERROR_NONE;

    char* pksav_test_saves = getenv("PKSAV_TEST_SAVES");
    if(!pksav_test_saves)
    {
        TEST_FAIL_MESSAGE("Failed to get test save directory.");
    }

    snprintf(
        original_filepath, sizeof(original_filepath),
        "%s%s%s%s%s",
        pksav_test_saves, FS_SEPARATOR, subdir, FS_SEPARATOR, save_name
    );

    size_t filesize = 0;
    if(pksav_fs_filesize(original_filepath, &filesize))
    {
        TEST_FAIL_MESSAGE("Failed to get save file size.");
    }
    TEST_ASSERT_TRUE(filesize >= PKSAV_GBA_SAVE_SIZE);

    uint8_t* save_buffer = NULL;
    if(pksav_fs_read_file_to_buffer(original_filepath, &save_buffer, &filesize))
    {
        TEST_FAIL_MESSAGE("Failed to read save into buffer.");
    }
    TEST_ASSERT_TRUE(filesize >= PKSAV_GBA_SAVE_SIZE);

    error = pksav_gba_load_save_from_buffer(
                save_buffer,
                filesize,
                &gba_save
            );
    PKSAV_TEST_ASSERT_SUCCESS(error);

    // This test will free the save.
    gba_save_test(
        &gba_save,
        expected_save_type,
        original_filepath,
        save_name
    );

    free(save_buffer);
}

static void gba_save_from_file_test(
    const char* subdir,
    const char* save_name,
    enum pksav_gba_save_type expected_save_type
)
{
    TEST_ASSERT_NOT_NULL(subdir);
    TEST_ASSERT_NOT_NULL(save_name);

    char original_filepath[256] = {0};
    struct pksav_gba_save gba_save = EMPTY_GBA_SAVE;
    enum pksav_error error = PKSAV_ERROR_NONE;

    char* pksav_test_saves = getenv("PKSAV_TEST_SAVES");
    if(!pksav_test_saves)
    {
        TEST_FAIL_MESSAGE("Failed to get test save directory.");
    }

    snprintf(
        original_filepath, sizeof(original_filepath),
        "%s%s%s%s%s",
        pksav_test_saves, FS_SEPARATOR, subdir, FS_SEPARATOR, save_name
    );

    error = pksav_gba_load_save_from_file(
                original_filepath,
                &gba_save
            );
    PKSAV_TEST_ASSERT_SUCCESS(error);

    // This test will free the save.
    gba_save_test(
        &gba_save,
        expected_save_type,
        original_filepath,
        save_name
    );
}

static void convenience_macro_test()
{
    struct pksav_gba_pc_pokemon pc_pokemon;
    memset(&pc_pokemon, 0, sizeof(pc_pokemon));

    struct pksav_gba_pokemon_blocks* p_blocks = &pc_pokemon.blocks;

    //uint16_t* p_origin_info = &p_blocks->misc.origin_info;
    uint32_t* p_ribbons_obedience = &p_blocks->misc.ribbons_obedience;

    *p_ribbons_obedience |= PKSAV_GBA_CONTEST_RIBBON_SUPER; // Cool
    // None for Beauty
    *p_ribbons_obedience |= (PKSAV_GBA_CONTEST_RIBBON_HYPER << PKSAV_GBA_CUTE_RIBBONS_OFFSET);
    *p_ribbons_obedience |= (PKSAV_GBA_CONTEST_RIBBON_NORMAL << PKSAV_GBA_SMART_RIBBONS_OFFSET);
    *p_ribbons_obedience |= (PKSAV_GBA_CONTEST_RIBBON_SUPER << PKSAV_GBA_TOUGH_RIBBONS_OFFSET);

    // Origin info (TODO: add enums, set, and test

    uint16_t ball = \
        PKSAV_GBA_POKEMON_BALL(p_blocks->misc.origin_info);
    uint16_t origin_game = \
        PKSAV_GBA_POKEMON_ORIGIN_GAME(p_blocks->misc.origin_info);

    (void)ball;
    (void)origin_game;

    // Ribbons

    uint32_t cool_contest_level   = PKSAV_GBA_COOL_CONTEST_LEVEL(*p_ribbons_obedience);
    uint32_t beauty_contest_level = PKSAV_GBA_BEAUTY_CONTEST_LEVEL(*p_ribbons_obedience);
    uint32_t cute_contest_level   = PKSAV_GBA_CUTE_CONTEST_LEVEL(*p_ribbons_obedience);
    uint32_t smart_contest_level  = PKSAV_GBA_SMART_CONTEST_LEVEL(*p_ribbons_obedience);
    uint32_t tough_contest_level  = PKSAV_GBA_TOUGH_CONTEST_LEVEL(*p_ribbons_obedience);

    TEST_ASSERT_EQUAL(PKSAV_GBA_CONTEST_RIBBON_SUPER,  cool_contest_level);
    TEST_ASSERT_EQUAL(PKSAV_GBA_CONTEST_RIBBON_NONE,   beauty_contest_level);
    TEST_ASSERT_EQUAL(PKSAV_GBA_CONTEST_RIBBON_HYPER,  cute_contest_level);
    TEST_ASSERT_EQUAL(PKSAV_GBA_CONTEST_RIBBON_NORMAL, smart_contest_level);
    TEST_ASSERT_EQUAL(PKSAV_GBA_CONTEST_RIBBON_SUPER,  tough_contest_level);
}

static void pksav_buffer_is_ruby_save_test()
{
    pksav_gba_get_buffer_save_type_test(
        "ruby_sapphire",
        "pokemon_ruby.sav",
        PKSAV_GBA_SAVE_TYPE_RS
    );
}

static void pksav_file_is_ruby_save_test()
{
    pksav_gba_get_file_save_type_test(
        "ruby_sapphire",
        "pokemon_ruby.sav",
        PKSAV_GBA_SAVE_TYPE_RS
    );
}

static void ruby_save_from_buffer_test()
{
    gba_save_from_buffer_test(
        "ruby_sapphire",
        "pokemon_ruby.sav",
        PKSAV_GBA_SAVE_TYPE_RS
    );
}

static void ruby_save_from_file_test()
{
    gba_save_from_file_test(
        "ruby_sapphire",
        "pokemon_ruby.sav",
        PKSAV_GBA_SAVE_TYPE_RS
    );
}

static void pksav_buffer_is_emerald_save_test()
{
    pksav_gba_get_buffer_save_type_test(
        "emerald",
        "pokemon_emerald.sav",
        PKSAV_GBA_SAVE_TYPE_EMERALD
    );
}

static void pksav_file_is_emerald_save_test()
{
    pksav_gba_get_file_save_type_test(
        "emerald",
        "pokemon_emerald.sav",
        PKSAV_GBA_SAVE_TYPE_EMERALD
    );
}

static void emerald_save_from_buffer_test()
{
    gba_save_from_buffer_test(
        "emerald",
        "pokemon_emerald.sav",
        PKSAV_GBA_SAVE_TYPE_EMERALD
    );
}

static void emerald_save_from_file_test()
{
    gba_save_from_file_test(
        "emerald",
        "pokemon_emerald.sav",
        PKSAV_GBA_SAVE_TYPE_EMERALD
    );
}

static void pksav_buffer_is_firered_save_test()
{
    pksav_gba_get_buffer_save_type_test(
        "firered_leafgreen",
        "pokemon_firered.sav",
        PKSAV_GBA_SAVE_TYPE_FRLG
    );
}

static void pksav_file_is_firered_save_test()
{
    pksav_gba_get_file_save_type_test(
        "firered_leafgreen",
        "pokemon_firered.sav",
        PKSAV_GBA_SAVE_TYPE_FRLG
    );
}

static void firered_save_from_buffer_test()
{
    gba_save_from_buffer_test(
        "firered_leafgreen",
        "pokemon_firered.sav",
        PKSAV_GBA_SAVE_TYPE_FRLG
    );
}

static void firered_save_from_file_test()
{
    gba_save_from_file_test(
        "firered_leafgreen",
        "pokemon_firered.sav",
        PKSAV_GBA_SAVE_TYPE_FRLG
    );
}

PKSAV_TEST_MAIN(
    PKSAV_TEST(pksav_gba_get_buffer_save_type_on_random_buffer_test)

    PKSAV_TEST(convenience_macro_test)

    PKSAV_TEST(pksav_buffer_is_ruby_save_test)
    PKSAV_TEST(pksav_file_is_ruby_save_test)
    PKSAV_TEST(ruby_save_from_buffer_test)
    PKSAV_TEST(ruby_save_from_file_test)

    PKSAV_TEST(pksav_buffer_is_emerald_save_test)
    PKSAV_TEST(pksav_file_is_emerald_save_test)
    PKSAV_TEST(emerald_save_from_buffer_test)
    PKSAV_TEST(emerald_save_from_file_test)

    PKSAV_TEST(pksav_buffer_is_firered_save_test)
    PKSAV_TEST(pksav_file_is_firered_save_test)
    PKSAV_TEST(firered_save_from_buffer_test)
    PKSAV_TEST(firered_save_from_file_test)
)
