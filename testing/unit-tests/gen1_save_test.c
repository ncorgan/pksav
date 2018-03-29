/*
 * Copyright (c) 2017,2018 Nicholas Corgan (n.corgan@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

#include "c_test_common.h"
#include "test-utils.h"

#include <pksav/config.h>
#include <pksav/gen1.h>

#include <stdio.h>
#include <string.h>

#define STRBUFFER_LEN 64

static const struct pksav_gen1_save EMPTY_GEN1_SAVE =
{
    .save_type       = PKSAV_GEN1_SAVE_TYPE_NONE,
    .time_played_ptr = NULL,
    .options_ptr     = NULL,

    .item_storage =
    {
        .item_bag_ptr = NULL,
        .item_pc_ptr  = NULL
    },

    .pokemon_storage =
    {
        .party_ptr           = NULL,
        .box_ptrs            = {NULL},
        .current_box_num_ptr = NULL,
        .current_box_ptr     = NULL
    },

    .pokedex_lists =
    {
        .seen_ptr  = NULL,
        .owned_ptr = NULL
    },

    .trainer_info =
    {
        .id_ptr     = NULL,
        .name_ptr   = NULL,
        .money_ptr  = NULL,
        .badges_ptr = NULL
    },

    .misc_fields =
    {
        .rival_name_ptr         = NULL,
        .casino_coins_ptr       = NULL,
        .pikachu_friendship_ptr = NULL
    },

    .internal_ptr = NULL
};

/*
 * We don't care about the result of the function itself. As the buffer
 * is randomized, it will likely be false. This function is to make sure
 * running it on invalid input doesn't crash.
 */
static void pksav_gen1_get_buffer_save_type_on_random_buffer_test()
{
    enum pksav_error error = PKSAV_ERROR_NONE;

    uint8_t buffer[PKSAV_GEN1_SAVE_SIZE] = {0};
    for(size_t run_index = 0; run_index < 1000; ++run_index)
    {
        randomize_buffer(buffer, sizeof(buffer));

        enum pksav_gen1_save_type save_type = PKSAV_GEN1_SAVE_TYPE_NONE;
        error = pksav_gen1_get_buffer_save_type(
                    buffer,
                    sizeof(buffer),
                    &save_type
                );
        TEST_ASSERT_EQUAL(PKSAV_ERROR_NONE, error);
    }
}

static void pksav_gen1_get_buffer_save_type_test(
    const char* subdir,
    const char* save_name,
    enum pksav_gen1_save_type expected_save_type
)
{
    TEST_ASSERT_NOT_NULL(subdir);
    TEST_ASSERT_NOT_NULL(save_name);
    TEST_ASSERT_TRUE(expected_save_type >= PKSAV_GEN1_SAVE_TYPE_RED_BLUE);
    TEST_ASSERT_TRUE(expected_save_type <= PKSAV_GEN1_SAVE_TYPE_YELLOW);

    char filepath[256] = {0};
    uint8_t save_buffer[PKSAV_GEN1_SAVE_SIZE];
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

    if(read_file_into_buffer(filepath, save_buffer, PKSAV_GEN1_SAVE_SIZE))
    {
        TEST_FAIL_MESSAGE("Failed to read save into buffer.");
    }

    enum pksav_gen1_save_type save_type = PKSAV_GEN1_SAVE_TYPE_NONE;
    error = pksav_gen1_get_buffer_save_type(
                save_buffer,
                PKSAV_GEN1_SAVE_SIZE,
                &save_type
            );
    TEST_ASSERT_EQUAL(PKSAV_ERROR_NONE, error);
    TEST_ASSERT_EQUAL(expected_save_type, save_type);
}

static void pksav_gen1_get_file_save_type_test(
    const char* subdir,
    const char* save_name,
    enum pksav_gen1_save_type expected_save_type
)
{
    TEST_ASSERT_NOT_NULL(subdir);
    TEST_ASSERT_NOT_NULL(save_name);
    TEST_ASSERT_TRUE(expected_save_type >= PKSAV_GEN1_SAVE_TYPE_RED_BLUE);
    TEST_ASSERT_TRUE(expected_save_type <= PKSAV_GEN1_SAVE_TYPE_YELLOW);

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

    enum pksav_gen1_save_type save_type = PKSAV_GEN1_SAVE_TYPE_NONE;
    error = pksav_gen1_get_file_save_type(
                filepath,
                &save_type
            );
    TEST_ASSERT_EQUAL(PKSAV_ERROR_NONE, error);
    TEST_ASSERT_EQUAL(expected_save_type, save_type);
}

static void validate_gen1_string(
    const uint8_t* buffer,
    size_t buffer_len
)
{
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE(buffer_len > 0);

    char strbuffer[STRBUFFER_LEN] = {0};
    enum pksav_error error = pksav_gen1_import_text(
                                 buffer,
                                 strbuffer,
                                 buffer_len
                             );
    TEST_ASSERT_EQUAL(PKSAV_ERROR_NONE, error);
    TEST_ASSERT_TRUE(strlen(strbuffer) > 0ULL);
}

static void validate_gen1_item_bag(
    const struct pksav_gen1_item_bag* item_bag_ptr
)
{
    TEST_ASSERT_NOT_NULL(item_bag_ptr);
    TEST_ASSERT_TRUE(item_bag_ptr->count <= PKSAV_GEN1_ITEM_BAG_SIZE);

    // Don't validate item contents. Generation I games are buggy enough
    // to make this unreliable.
}

static void validate_bcd(
    const uint8_t* buffer,
    size_t num_bytes,
    uint32_t max_value
)
{
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE(num_bytes > 0);
    TEST_ASSERT_TRUE(max_value > 0);

    uint32_t bcd_value = 0;
    enum pksav_error error = pksav_from_bcd(
                                 buffer,
                                 num_bytes,
                                 &bcd_value
                             );
    TEST_ASSERT_EQUAL(PKSAV_ERROR_NONE, error);
    TEST_ASSERT_TRUE(bcd_value <= max_value);
}

static void validate_gen1_item_pc(
    const struct pksav_gen1_item_pc* item_pc_ptr
)
{
    TEST_ASSERT_NOT_NULL(item_pc_ptr);
    TEST_ASSERT_TRUE(item_pc_ptr->count <= PKSAV_GEN1_ITEM_PC_SIZE);

    // Don't validate item contents. Generation I games are buggy enough
    // to make this unreliable.
}

static void validate_gen1_pokemon_party(
    const struct pksav_gen1_pokemon_party* party_ptr
)
{
    TEST_ASSERT_NOT_NULL(party_ptr);
    TEST_ASSERT_TRUE(party_ptr->count <= PKSAV_GEN1_PARTY_NUM_POKEMON);

    if(party_ptr->count < PKSAV_GEN1_PARTY_NUM_POKEMON)
    {
        TEST_ASSERT_EQUAL(0xFF, party_ptr->species[party_ptr->count]);
    }

    for(size_t party_index = 0; party_index < party_ptr->count; ++party_index)
    {
        validate_gen1_string(
            party_ptr->otnames[party_index],
            10
        );
        validate_gen1_string(
            party_ptr->nicknames[party_index],
            10
        );
    }
}

static void validate_gen1_pokemon_box(
    const struct pksav_gen1_pokemon_box* pokemon_box_ptr
)
{
    TEST_ASSERT_NOT_NULL(pokemon_box_ptr);
    TEST_ASSERT_TRUE(pokemon_box_ptr->count <= PKSAV_GEN1_BOX_NUM_POKEMON);

    if(pokemon_box_ptr->count < PKSAV_GEN1_BOX_NUM_POKEMON)
    {
        TEST_ASSERT_EQUAL(0xFF, pokemon_box_ptr->species[pokemon_box_ptr->count]);

        for(size_t box_index = 0; box_index < pokemon_box_ptr->count; ++box_index)
        {
            validate_gen1_string(
                pokemon_box_ptr->nicknames[box_index],
                10
            );
            validate_gen1_string(
                pokemon_box_ptr->otnames[box_index],
                10
            );
        }
    }
}

static void gen1_save_test(
    const char* subdir,
    const char* save_name
)
{
    TEST_ASSERT_NOT_NULL(subdir);
    TEST_ASSERT_NOT_NULL(save_name);

    char original_filepath[256] = {0};
    char tmp_save_filepath[256] = {0};
    struct pksav_gen1_save gen1_save = EMPTY_GEN1_SAVE;
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
    snprintf(
        tmp_save_filepath, sizeof(tmp_save_filepath),
        "%s%spksav_%d_%s",
        get_tmp_dir(), FS_SEPARATOR, get_pid(), save_name
    );

    error = pksav_gen1_load_save_from_file(
                original_filepath,
                &gen1_save
            );
    TEST_ASSERT_EQUAL(PKSAV_ERROR_NONE, error);

    // Validate fields. No pointers should be NULL (except the Pikachu friendship
    // pointer for Red/Blue), and some fields have a specific set of valid values.
    TEST_ASSERT_NOT_NULL(gen1_save.internal_ptr);

    TEST_ASSERT_NOT_EQUAL(PKSAV_GEN1_SAVE_TYPE_NONE, gen1_save.save_type);
    TEST_ASSERT_NOT_NULL(gen1_save.time_played_ptr);
    TEST_ASSERT_NOT_NULL(gen1_save.options_ptr);

    validate_gen1_item_bag(gen1_save.item_storage.item_bag_ptr);
    validate_gen1_item_pc(gen1_save.item_storage.item_pc_ptr);
    validate_gen1_pokemon_party(gen1_save.pokemon_storage.party_ptr);

    for(size_t box_index = 0; box_index < PKSAV_GEN1_NUM_POKEMON_BOXES; ++box_index)
    {
        validate_gen1_pokemon_box(
            gen1_save.pokemon_storage.box_ptrs[box_index]
        );
    }

    TEST_ASSERT_NOT_NULL(gen1_save.pokemon_storage.current_box_num_ptr);
    uint8_t current_box_num = (*gen1_save.pokemon_storage.current_box_num_ptr
                            & PKSAV_GEN1_CURRENT_POKEMON_BOX_NUM_MASK);
    TEST_ASSERT_TRUE(current_box_num <= PKSAV_GEN1_NUM_POKEMON_BOXES);

    validate_gen1_pokemon_box(gen1_save.pokemon_storage.current_box_ptr);

    TEST_ASSERT_NOT_NULL(gen1_save.pokedex_lists.seen_ptr);
    TEST_ASSERT_NOT_NULL(gen1_save.pokedex_lists.owned_ptr);

    TEST_ASSERT_NOT_NULL(gen1_save.trainer_info.id_ptr);
    validate_gen1_string(
        gen1_save.trainer_info.name_ptr,
        7
    );
    validate_bcd(
        gen1_save.trainer_info.money_ptr,
        3,
        999999
    );
    TEST_ASSERT_NOT_NULL(gen1_save.trainer_info.badges_ptr);

    validate_gen1_string(
        gen1_save.misc_fields.rival_name_ptr,
        7
    );
    validate_bcd(
        gen1_save.misc_fields.casino_coins_ptr,
        2,
        9999
    );
    if(gen1_save.save_type == PKSAV_GEN1_SAVE_TYPE_RED_BLUE)
    {
        TEST_ASSERT_NULL(gen1_save.misc_fields.pikachu_friendship_ptr);
    }
    else
    {
        TEST_ASSERT_NOT_NULL(gen1_save.misc_fields.pikachu_friendship_ptr);
    }

    // Make sure loading and saving are perfectly symmetrical.
    error = pksav_gen1_save_save(
                tmp_save_filepath,
                &gen1_save
            );
    TEST_ASSERT_EQUAL(PKSAV_ERROR_NONE, error);

    bool files_differ = false;
    if(do_files_differ(original_filepath, tmp_save_filepath, &files_differ))
    {
        if(delete_file(tmp_save_filepath))
        {
            TEST_FAIL_MESSAGE("Comparing the two saves failed. Failed to clean up temp file.");
        }
        TEST_FAIL_MESSAGE("Comparing the two saves failed.");
    }

    if(delete_file(tmp_save_filepath))
    {
        TEST_FAIL_MESSAGE("Failed to clean up temp file.");
    }
    TEST_ASSERT_FALSE(files_differ);

    // Make sure setting the current box works as expected.
    for(uint8_t box_index = 0; box_index < PKSAV_GEN1_NUM_POKEMON_BOXES; ++box_index)
    {
        error = pksav_gen1_pokemon_storage_set_current_box(
                    &gen1_save.pokemon_storage,
                    box_index
                );
        TEST_ASSERT_EQUAL(PKSAV_ERROR_NONE, error);

        uint8_t current_box_num = *gen1_save.pokemon_storage.current_box_num_ptr;
        current_box_num &= PKSAV_GEN1_CURRENT_POKEMON_BOX_NUM_MASK;
        TEST_ASSERT_EQUAL(box_index, current_box_num);
        TEST_ASSERT_EQUAL_MEMORY(
            gen1_save.pokemon_storage.box_ptrs[box_index],
            gen1_save.pokemon_storage.current_box_ptr,
            sizeof(struct pksav_gen1_pokemon_box)
        );
    }

    // Free the save and make sure all fields are set to NULL or default.
    error = pksav_gen1_free_save(&gen1_save);
    TEST_ASSERT_EQUAL(PKSAV_ERROR_NONE, error);

    TEST_ASSERT_EQUAL(PKSAV_GEN1_SAVE_TYPE_NONE, gen1_save.save_type);
    TEST_ASSERT_NULL(gen1_save.time_played_ptr);
    TEST_ASSERT_NULL(gen1_save.options_ptr);

    TEST_ASSERT_NULL(gen1_save.item_storage.item_bag_ptr);
    TEST_ASSERT_NULL(gen1_save.item_storage.item_pc_ptr);

    TEST_ASSERT_NULL(gen1_save.pokemon_storage.party_ptr);
    for(size_t box_index = 0; box_index < PKSAV_GEN1_NUM_POKEMON_BOXES; ++box_index)
    {
        TEST_ASSERT_NULL(gen1_save.pokemon_storage.box_ptrs[box_index]);
    }
    TEST_ASSERT_NULL(gen1_save.pokemon_storage.current_box_num_ptr);
    TEST_ASSERT_NULL(gen1_save.pokemon_storage.current_box_ptr);

    TEST_ASSERT_NULL(gen1_save.pokedex_lists.seen_ptr);
    TEST_ASSERT_NULL(gen1_save.pokedex_lists.owned_ptr);

    TEST_ASSERT_NULL(gen1_save.trainer_info.id_ptr);
    TEST_ASSERT_NULL(gen1_save.trainer_info.name_ptr);
    TEST_ASSERT_NULL(gen1_save.trainer_info.money_ptr);
    TEST_ASSERT_NULL(gen1_save.trainer_info.badges_ptr);

    TEST_ASSERT_NULL(gen1_save.misc_fields.rival_name_ptr);
    TEST_ASSERT_NULL(gen1_save.misc_fields.casino_coins_ptr);
    TEST_ASSERT_NULL(gen1_save.misc_fields.pikachu_friendship_ptr);

    TEST_ASSERT_NULL(gen1_save.internal_ptr);
}

static void pksav_buffer_is_red_save_test()
{
    pksav_gen1_get_buffer_save_type_test(
        "red_blue",
        "pokemon_red.sav",
        PKSAV_GEN1_SAVE_TYPE_RED_BLUE
    );
}

static void pksav_file_is_red_save_test()
{
    pksav_gen1_get_file_save_type_test(
        "red_blue",
        "pokemon_red.sav",
        PKSAV_GEN1_SAVE_TYPE_RED_BLUE
    );
}

static void red_save_test()
{
    gen1_save_test("red_blue", "pokemon_red.sav");
}

static void pksav_buffer_is_yellow_save_test()
{
    pksav_gen1_get_buffer_save_type_test(
        "yellow",
        "pokemon_yellow.sav",
        PKSAV_GEN1_SAVE_TYPE_YELLOW
    );
}

static void pksav_file_is_yellow_save_test()
{
    pksav_gen1_get_file_save_type_test(
        "yellow",
        "pokemon_yellow.sav",
        PKSAV_GEN1_SAVE_TYPE_YELLOW
    );
}

static void yellow_save_test()
{
    gen1_save_test("yellow", "pokemon_yellow.sav");
}

PKSAV_TEST_MAIN(
    PKSAV_TEST(pksav_gen1_get_buffer_save_type_on_random_buffer_test)

    PKSAV_TEST(pksav_buffer_is_red_save_test)
    PKSAV_TEST(pksav_file_is_red_save_test)
    PKSAV_TEST(red_save_test)

    PKSAV_TEST(pksav_buffer_is_yellow_save_test)
    PKSAV_TEST(pksav_file_is_yellow_save_test)
    PKSAV_TEST(yellow_save_test)
)
