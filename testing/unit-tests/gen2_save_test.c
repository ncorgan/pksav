/*
 * Copyright (c) 2017-2018 Nicholas Corgan (n.corgan@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

#include "c_test_common.h"
#include "test-utils.h"

#include "util/fs.h"

#include <pksav/config.h>
#include <pksav/gen2/save.h>
#include <pksav/gen2/text.h>
#include <pksav/math/bcd.h>

#include <stdio.h>
#include <string.h>

#define STRBUFFER_LEN 64

static const struct pksav_gen2_save EMPTY_GEN2_SAVE =
{
    .save_type = PKSAV_GEN2_SAVE_TYPE_NONE,

    .save_time =
    {
        .time_played_ptr      = NULL,
        .daylight_savings_ptr = NULL
    },

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
        .id_ptr           = NULL,
        .name_ptr         = NULL,
        .gender_ptr       = NULL,
        .palette_ptr      = NULL,
        .money_ptr        = NULL,
        .johto_badges_ptr = NULL,
        .kanto_badges_ptr = NULL
    },

    .misc_fields =
    {
        .rival_name_ptr = NULL
    },

    .internal_ptr = NULL
};

/*
 * We don't care about the result of the function itself. As the buffer
 * is randomized, it will likely be false. This function is to make sure
 * running it on invalid input doesn't crash.
 */
static void pksav_gen2_get_buffer_save_type_on_random_buffer_test()
{
    enum pksav_error error = PKSAV_ERROR_NONE;

    uint8_t buffer[PKSAV_GEN2_SAVE_SIZE] = {0};
    for(size_t run_index = 0; run_index < 1000; ++run_index)
    {
        randomize_buffer(buffer, sizeof(buffer));

        enum pksav_gen2_save_type save_type = PKSAV_GEN2_SAVE_TYPE_NONE;
        error = pksav_gen2_get_buffer_save_type(
                    buffer,
                    sizeof(buffer),
                    &save_type
                );
        PKSAV_TEST_ASSERT_SUCCESS(error);
    }
}

static void pksav_gen2_get_buffer_save_type_test(
    const char* subdir,
    const char* save_name,
    enum pksav_gen2_save_type expected_save_type
)
{
    TEST_ASSERT_NOT_NULL(subdir);
    TEST_ASSERT_NOT_NULL(save_name);
    TEST_ASSERT_TRUE(expected_save_type >= PKSAV_GEN2_SAVE_TYPE_GS);
    TEST_ASSERT_TRUE(expected_save_type <= PKSAV_GEN2_SAVE_TYPE_CRYSTAL);

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

    uint8_t* save_buffer = NULL;
    size_t save_size = 0;
    if(pksav_fs_read_file_to_buffer(filepath, &save_buffer, &save_size))
    {
        TEST_FAIL_MESSAGE("Failed to read save into buffer.");
    }
    TEST_ASSERT_TRUE(save_size >= PKSAV_GEN2_SAVE_SIZE);

    enum pksav_gen2_save_type save_type = PKSAV_GEN2_SAVE_TYPE_NONE;
    error = pksav_gen2_get_buffer_save_type(
                save_buffer,
                PKSAV_GEN2_SAVE_SIZE,
                &save_type
            );
    PKSAV_TEST_ASSERT_SUCCESS(error);
    TEST_ASSERT_EQUAL(expected_save_type, save_type);

    free(save_buffer);
}

static void pksav_gen2_get_file_save_type_test(
    const char* subdir,
    const char* save_name,
    enum pksav_gen2_save_type expected_save_type
)
{
    TEST_ASSERT_NOT_NULL(subdir);
    TEST_ASSERT_NOT_NULL(save_name);
    TEST_ASSERT_TRUE(expected_save_type >= PKSAV_GEN2_SAVE_TYPE_GS);
    TEST_ASSERT_TRUE(expected_save_type <= PKSAV_GEN2_SAVE_TYPE_CRYSTAL);

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

    enum pksav_gen2_save_type save_type = PKSAV_GEN2_SAVE_TYPE_NONE;
    error = pksav_gen2_get_file_save_type(
                filepath,
                &save_type
            );
    PKSAV_TEST_ASSERT_SUCCESS(error);
    TEST_ASSERT_EQUAL(expected_save_type, save_type);
}

static void validate_gen2_string(
    const uint8_t* buffer,
    size_t buffer_len
)
{
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE(buffer_len > 0);

    char strbuffer[STRBUFFER_LEN] = {0};
    enum pksav_error error = pksav_gen2_import_text(
                                 buffer,
                                 strbuffer,
                                 buffer_len
                             );
    PKSAV_TEST_ASSERT_SUCCESS(error);
    TEST_ASSERT_TRUE(strlen(strbuffer) > 0ULL);
}

static void validate_gen2_item_bag(
    const struct pksav_gen2_item_bag* item_bag_ptr
)
{
    TEST_ASSERT_NOT_NULL(item_bag_ptr);

    // TODO: validate pockets
}

static void validate_bcd(
    const uint8_t* buffer,
    size_t num_bytes,
    size_t max_value
)
{
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE(num_bytes > 0);
    TEST_ASSERT_TRUE(max_value > 0);

    size_t bcd_value = 0;
    enum pksav_error error = pksav_import_bcd(
                                 buffer,
                                 num_bytes,
                                 &bcd_value
                             );
    PKSAV_TEST_ASSERT_SUCCESS(error);
    TEST_ASSERT_TRUE(bcd_value <= max_value);
}

static void validate_gen2_item_pc(
    const struct pksav_gen2_item_pc* item_pc_ptr
)
{
    TEST_ASSERT_NOT_NULL(item_pc_ptr);
    TEST_ASSERT_TRUE(item_pc_ptr->count <= PKSAV_GEN2_ITEM_PC_SIZE);

    // Don't validate item contents. Generation I games are buggy enough
    // to make this unreliable.
}

static void validate_gen2_pokemon_party(
    const struct pksav_gen2_pokemon_party* party_ptr
)
{
    TEST_ASSERT_NOT_NULL(party_ptr);
    TEST_ASSERT_TRUE(party_ptr->count <= PKSAV_GEN2_PARTY_NUM_POKEMON);

    if(party_ptr->count < PKSAV_GEN2_PARTY_NUM_POKEMON)
    {
        TEST_ASSERT_EQUAL(0xFF, party_ptr->species[party_ptr->count]);
    }

    for(size_t party_index = 0; party_index < party_ptr->count; ++party_index)
    {
        if(party_ptr->species[party_index] != PKSAV_GEN2_POKEMON_EGG_INDEX)
        {
            validate_gen2_string(
                party_ptr->otnames[party_index],
                10
            );
            validate_gen2_string(
                party_ptr->nicknames[party_index],
                10
            );
        }
    }
}

static void validate_gen2_pokemon_box(
    const struct pksav_gen2_pokemon_box* pokemon_box_ptr
)
{
    TEST_ASSERT_NOT_NULL(pokemon_box_ptr);
    TEST_ASSERT_TRUE(pokemon_box_ptr->count <= PKSAV_GEN2_BOX_NUM_POKEMON);

    if(pokemon_box_ptr->count < PKSAV_GEN2_BOX_NUM_POKEMON)
    {
        TEST_ASSERT_EQUAL(0xFF, pokemon_box_ptr->species[pokemon_box_ptr->count]);

        for(size_t box_index = 0; box_index < pokemon_box_ptr->count; ++box_index)
        {
            if(pokemon_box_ptr->species[box_index] != PKSAV_GEN2_POKEMON_EGG_INDEX)
            {
                validate_gen2_string(
                    pokemon_box_ptr->nicknames[box_index],
                    10
                );
                validate_gen2_string(
                    pokemon_box_ptr->otnames[box_index],
                    10
                );
            }
        }
    }
}

static void compare_gen2_saves(
    struct pksav_gen2_save* gen2_save1_ptr,
    struct pksav_gen2_save* gen2_save2_ptr
)
{
    TEST_ASSERT_NOT_NULL(gen2_save1_ptr);
    TEST_ASSERT_NOT_NULL(gen2_save2_ptr);

    TEST_ASSERT_EQUAL(
        gen2_save1_ptr->save_type,
        gen2_save2_ptr->save_type
    );

    // Save time
    struct pksav_gen2_save_time* save_time1_ptr = &gen2_save1_ptr->save_time;
    struct pksav_gen2_save_time* save_time2_ptr = &gen2_save2_ptr->save_time;

    TEST_ASSERT_EQUAL_MEMORY(
        save_time1_ptr->time_played_ptr,
        save_time2_ptr->time_played_ptr,
        sizeof(*save_time1_ptr->time_played_ptr)
    );
    TEST_ASSERT_EQUAL(
        *save_time1_ptr->daylight_savings_ptr,
        *save_time2_ptr->daylight_savings_ptr
    );

    // Item storage
    struct pksav_gen2_item_storage* item_storage1_ptr = &gen2_save1_ptr->item_storage;
    struct pksav_gen2_item_storage* item_storage2_ptr = &gen2_save2_ptr->item_storage;

    TEST_ASSERT_EQUAL_MEMORY(
        item_storage1_ptr->item_bag_ptr,
        item_storage2_ptr->item_bag_ptr,
        sizeof(*item_storage1_ptr->item_bag_ptr)
    );
    TEST_ASSERT_EQUAL_MEMORY(
        item_storage1_ptr->item_pc_ptr,
        item_storage2_ptr->item_pc_ptr,
        sizeof(*item_storage1_ptr->item_pc_ptr)
    );

    // Pokémon storage
    struct pksav_gen2_pokemon_storage* pokemon_storage1_ptr = &gen2_save1_ptr->pokemon_storage;
    struct pksav_gen2_pokemon_storage* pokemon_storage2_ptr = &gen2_save2_ptr->pokemon_storage;

    TEST_ASSERT_EQUAL_MEMORY(
        pokemon_storage1_ptr->party_ptr,
        pokemon_storage2_ptr->party_ptr,
        sizeof(*pokemon_storage1_ptr->party_ptr)
    );
    for(size_t box_index = 0; box_index < PKSAV_GEN2_NUM_POKEMON_BOXES; ++box_index)
    {
        TEST_ASSERT_EQUAL_MEMORY(
            pokemon_storage1_ptr->box_ptrs[box_index],
            pokemon_storage2_ptr->box_ptrs[box_index],
            sizeof(*pokemon_storage1_ptr->box_ptrs[box_index])
        );
    }
    TEST_ASSERT_EQUAL_MEMORY(
        pokemon_storage1_ptr->box_names_ptr,
        pokemon_storage2_ptr->box_names_ptr,
        sizeof(*pokemon_storage1_ptr->box_names_ptr)
    );
    TEST_ASSERT_EQUAL(
        *pokemon_storage1_ptr->current_box_num_ptr,
        *pokemon_storage2_ptr->current_box_num_ptr
    );
    TEST_ASSERT_EQUAL_MEMORY(
        pokemon_storage1_ptr->current_box_ptr,
        pokemon_storage2_ptr->current_box_ptr,
        sizeof(*pokemon_storage1_ptr->current_box_ptr)
    );

    // Pokédex lists
    struct pksav_gen2_pokedex_lists* pokedex_lists1_ptr = &gen2_save1_ptr->pokedex_lists;
    struct pksav_gen2_pokedex_lists* pokedex_lists2_ptr = &gen2_save2_ptr->pokedex_lists;
    const size_t pokedex_buffer_size = (251 / 8) + 1;

    TEST_ASSERT_EQUAL_MEMORY(
        pokedex_lists1_ptr->seen_ptr,
        pokedex_lists2_ptr->seen_ptr,
        pokedex_buffer_size
    );
    TEST_ASSERT_EQUAL_MEMORY(
        pokedex_lists1_ptr->owned_ptr,
        pokedex_lists2_ptr->owned_ptr,
        pokedex_buffer_size
    );

    // Trainer info
    struct pksav_gen2_trainer_info* trainer_info1_ptr = &gen2_save1_ptr->trainer_info;
    struct pksav_gen2_trainer_info* trainer_info2_ptr = &gen2_save2_ptr->trainer_info;

    TEST_ASSERT_EQUAL(
        *trainer_info1_ptr->id_ptr,
        *trainer_info2_ptr->id_ptr
    );
    TEST_ASSERT_EQUAL_MEMORY(
        trainer_info1_ptr->name_ptr,
        trainer_info2_ptr->name_ptr,
        7
    );
    if(gen2_save1_ptr->save_type == PKSAV_GEN2_SAVE_TYPE_CRYSTAL)
    {
        TEST_ASSERT_EQUAL(
            *trainer_info1_ptr->gender_ptr,
            *trainer_info2_ptr->gender_ptr
        );
    }
    TEST_ASSERT_EQUAL(
        *trainer_info1_ptr->palette_ptr,
        *trainer_info2_ptr->palette_ptr
    );
    TEST_ASSERT_EQUAL_MEMORY(
        trainer_info1_ptr->money_ptr,
        trainer_info2_ptr->money_ptr,
        3
    );
    TEST_ASSERT_EQUAL(
        *trainer_info1_ptr->johto_badges_ptr,
        *trainer_info2_ptr->johto_badges_ptr
    );
    TEST_ASSERT_EQUAL(
        *trainer_info1_ptr->kanto_badges_ptr,
        *trainer_info2_ptr->kanto_badges_ptr
    );

    // Trainer info
    struct pksav_gen2_misc_fields* misc_fields1_ptr = &gen2_save1_ptr->misc_fields;
    struct pksav_gen2_misc_fields* misc_fields2_ptr = &gen2_save2_ptr->misc_fields;

    TEST_ASSERT_EQUAL_MEMORY(
        misc_fields1_ptr->rival_name_ptr,
        misc_fields2_ptr->rival_name_ptr,
        7
    );
}

static void gen2_save_test(
    struct pksav_gen2_save* gen2_save_ptr,
    enum pksav_gen2_save_type expected_save_type,
    const char* original_filepath,
    const char* save_name
)
{
    TEST_ASSERT_NOT_NULL(gen2_save_ptr);
    TEST_ASSERT_NOT_NULL(original_filepath);
    TEST_ASSERT_NOT_NULL(save_name);

    char tmp_save_filepath[256] = {0};
    enum pksav_error error = PKSAV_ERROR_NONE;

    snprintf(
        tmp_save_filepath, sizeof(tmp_save_filepath),
        "%s%spksav_%d_%s",
        get_tmp_dir(), FS_SEPARATOR, get_pid(), save_name
    );

    // Validate fields. No pointers should be NULL (except the trainer gender
    // pointer for Gold/Silver), and some fields have a specific set of valid values.
    TEST_ASSERT_NOT_NULL(gen2_save_ptr->internal_ptr);

    TEST_ASSERT_EQUAL(expected_save_type, gen2_save_ptr->save_type);
    TEST_ASSERT_NOT_NULL(gen2_save_ptr->save_time.time_played_ptr);
    TEST_ASSERT_NOT_NULL(gen2_save_ptr->save_time.daylight_savings_ptr);

    validate_gen2_item_bag(gen2_save_ptr->item_storage.item_bag_ptr);
    validate_gen2_item_pc(gen2_save_ptr->item_storage.item_pc_ptr);
    validate_gen2_pokemon_party(gen2_save_ptr->pokemon_storage.party_ptr);

    for(size_t box_index = 0; box_index < PKSAV_GEN2_NUM_POKEMON_BOXES; ++box_index)
    {
        validate_gen2_string(
            gen2_save_ptr->pokemon_storage.box_names_ptr->names[box_index],
            9
        );
        validate_gen2_pokemon_box(
            gen2_save_ptr->pokemon_storage.box_ptrs[box_index]
        );
    }

    TEST_ASSERT_NOT_NULL(gen2_save_ptr->pokemon_storage.current_box_num_ptr);
    TEST_ASSERT_TRUE(*gen2_save_ptr->pokemon_storage.current_box_num_ptr <=
                     PKSAV_GEN2_NUM_POKEMON_BOXES);

    validate_gen2_pokemon_box(gen2_save_ptr->pokemon_storage.current_box_ptr);

    TEST_ASSERT_NOT_NULL(gen2_save_ptr->pokedex_lists.seen_ptr);
    TEST_ASSERT_NOT_NULL(gen2_save_ptr->pokedex_lists.owned_ptr);

    TEST_ASSERT_NOT_NULL(gen2_save_ptr->trainer_info.id_ptr);
    validate_gen2_string(
        gen2_save_ptr->trainer_info.name_ptr,
        7
    );

    if(gen2_save_ptr->save_type == PKSAV_GEN2_SAVE_TYPE_CRYSTAL)
    {
        TEST_ASSERT_NOT_NULL(gen2_save_ptr->trainer_info.gender_ptr);
    }
    else
    {
        TEST_ASSERT_NULL(gen2_save_ptr->trainer_info.gender_ptr);
    }

    TEST_ASSERT_NOT_NULL(gen2_save_ptr->trainer_info.palette_ptr);
    validate_bcd(
        gen2_save_ptr->trainer_info.money_ptr,
        3,
        999999
    );
    TEST_ASSERT_NOT_NULL(gen2_save_ptr->trainer_info.johto_badges_ptr);
    TEST_ASSERT_NOT_NULL(gen2_save_ptr->trainer_info.kanto_badges_ptr);

    validate_gen2_string(
        gen2_save_ptr->misc_fields.rival_name_ptr,
        7
    );

    // Make sure loading and saving are perfectly symmetrical. As the checksum
    // is not guaranteed to be set for some reason, compare each part.
    error = pksav_gen2_save_save(
                tmp_save_filepath,
                gen2_save_ptr
            );
    PKSAV_TEST_ASSERT_SUCCESS(error);

    struct pksav_gen2_save tmp_gen2_save = EMPTY_GEN2_SAVE;
    error = pksav_gen2_load_save_from_file(
                tmp_save_filepath,
                &tmp_gen2_save
            );
    PKSAV_TEST_ASSERT_SUCCESS(error);

    compare_gen2_saves(
        gen2_save_ptr,
        &tmp_gen2_save
    );

    error = pksav_gen2_free_save(&tmp_gen2_save);
    PKSAV_TEST_ASSERT_SUCCESS(error);

    if(delete_file(tmp_save_filepath))
    {
        TEST_FAIL_MESSAGE("Failed to clean up temp file.");
    }

    // Make sure setting the current box works as expected.
    for(uint8_t box_index = 0; box_index < PKSAV_GEN2_NUM_POKEMON_BOXES; ++box_index)
    {
        error = pksav_gen2_pokemon_storage_set_current_box(
                    &gen2_save_ptr->pokemon_storage,
                    box_index
                );
        PKSAV_TEST_ASSERT_SUCCESS(error);

        TEST_ASSERT_EQUAL(
            box_index,
            *gen2_save_ptr->pokemon_storage.current_box_num_ptr
        );
        TEST_ASSERT_EQUAL_MEMORY(
            gen2_save_ptr->pokemon_storage.box_ptrs[box_index],
            gen2_save_ptr->pokemon_storage.current_box_ptr,
            sizeof(struct pksav_gen2_pokemon_box)
        );
    }

    // Free the save and make sure all fields are set to NULL or default.
    error = pksav_gen2_free_save(gen2_save_ptr);
    PKSAV_TEST_ASSERT_SUCCESS(error);

    TEST_ASSERT_EQUAL(PKSAV_GEN2_SAVE_TYPE_NONE, gen2_save_ptr->save_type);

    TEST_ASSERT_NULL(gen2_save_ptr->save_time.time_played_ptr);
    TEST_ASSERT_NULL(gen2_save_ptr->save_time.daylight_savings_ptr);

    TEST_ASSERT_NULL(gen2_save_ptr->item_storage.item_bag_ptr);
    TEST_ASSERT_NULL(gen2_save_ptr->item_storage.item_pc_ptr);

    TEST_ASSERT_NULL(gen2_save_ptr->pokemon_storage.party_ptr);
    for(size_t box_index = 0; box_index < PKSAV_GEN2_NUM_POKEMON_BOXES; ++box_index)
    {
        TEST_ASSERT_NULL(gen2_save_ptr->pokemon_storage.box_ptrs[box_index]);
    }
    TEST_ASSERT_NULL(gen2_save_ptr->pokemon_storage.current_box_num_ptr);
    TEST_ASSERT_NULL(gen2_save_ptr->pokemon_storage.current_box_ptr);

    TEST_ASSERT_NULL(gen2_save_ptr->pokedex_lists.seen_ptr);
    TEST_ASSERT_NULL(gen2_save_ptr->pokedex_lists.owned_ptr);

    TEST_ASSERT_NULL(gen2_save_ptr->trainer_info.id_ptr);
    TEST_ASSERT_NULL(gen2_save_ptr->trainer_info.name_ptr);
    TEST_ASSERT_NULL(gen2_save_ptr->trainer_info.money_ptr);
    TEST_ASSERT_NULL(gen2_save_ptr->trainer_info.johto_badges_ptr);
    TEST_ASSERT_NULL(gen2_save_ptr->trainer_info.kanto_badges_ptr);

    TEST_ASSERT_NULL(gen2_save_ptr->misc_fields.rival_name_ptr);

    TEST_ASSERT_NULL(gen2_save_ptr->internal_ptr);
}

static void gen2_save_from_buffer_test(
    const char* subdir,
    const char* save_name,
    enum pksav_gen2_save_type expected_save_type
)
{
    TEST_ASSERT_NOT_NULL(subdir);
    TEST_ASSERT_NOT_NULL(save_name);

    char original_filepath[256] = {0};
    struct pksav_gen2_save gen2_save = EMPTY_GEN2_SAVE;
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

    uint8_t* save_buffer = NULL;
    size_t save_size = 0;
    if(pksav_fs_read_file_to_buffer(original_filepath, &save_buffer, &save_size))
    {
        TEST_FAIL_MESSAGE("Failed to read save into buffer.");
    }
    TEST_ASSERT_TRUE(save_size >= PKSAV_GEN2_SAVE_SIZE);

    error = pksav_gen2_load_save_from_buffer(
                save_buffer,
                save_size,
                &gen2_save
            );
    PKSAV_TEST_ASSERT_SUCCESS(error);

    // This test will free the save.
    gen2_save_test(
        &gen2_save,
        expected_save_type,
        original_filepath,
        save_name
    );

    free(save_buffer);
}

static void gen2_save_from_file_test(
    const char* subdir,
    const char* save_name,
    enum pksav_gen2_save_type expected_save_type
)
{
    TEST_ASSERT_NOT_NULL(subdir);
    TEST_ASSERT_NOT_NULL(save_name);

    char original_filepath[256] = {0};
    struct pksav_gen2_save gen2_save = EMPTY_GEN2_SAVE;
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

    error = pksav_gen2_load_save_from_file(
                original_filepath,
                &gen2_save
            );
    PKSAV_TEST_ASSERT_SUCCESS(error);

    // This test will free the save.
    gen2_save_test(
        &gen2_save,
        expected_save_type,
        original_filepath,
        save_name
    );
}

static void pksav_buffer_is_gold_save_test()
{
    pksav_gen2_get_buffer_save_type_test(
        "gold_silver",
        "pokemon_gold.sav",
        PKSAV_GEN2_SAVE_TYPE_GS
    );
}

static void pksav_file_is_gold_save_test()
{
    pksav_gen2_get_file_save_type_test(
        "gold_silver",
        "pokemon_gold.sav",
        PKSAV_GEN2_SAVE_TYPE_GS
    );
}

static void gold_save_from_buffer_test()
{
    gen2_save_from_buffer_test(
        "gold_silver",
        "pokemon_gold.sav",
        PKSAV_GEN2_SAVE_TYPE_GS
    );
}

static void gold_save_from_file_test()
{
    gen2_save_from_file_test(
        "gold_silver",
        "pokemon_gold.sav",
        PKSAV_GEN2_SAVE_TYPE_GS
    );
}

static void pksav_buffer_is_crystal_save_test()
{
    pksav_gen2_get_buffer_save_type_test(
        "crystal",
        "pokemon_crystal.sav",
        PKSAV_GEN2_SAVE_TYPE_CRYSTAL
    );
}

static void pksav_file_is_crystal_save_test()
{
    pksav_gen2_get_file_save_type_test(
        "crystal",
        "pokemon_crystal.sav",
        PKSAV_GEN2_SAVE_TYPE_CRYSTAL
    );
}

static void crystal_save_from_buffer_test()
{
    gen2_save_from_buffer_test(
        "crystal",
        "pokemon_crystal.sav",
        PKSAV_GEN2_SAVE_TYPE_CRYSTAL
    );
}

static void crystal_save_from_file_test()
{
    gen2_save_from_file_test(
        "crystal",
        "pokemon_crystal.sav",
        PKSAV_GEN2_SAVE_TYPE_CRYSTAL
    );
}

PKSAV_TEST_MAIN(
    PKSAV_TEST(pksav_gen2_get_buffer_save_type_on_random_buffer_test)

    PKSAV_TEST(pksav_buffer_is_gold_save_test)
    PKSAV_TEST(pksav_file_is_gold_save_test)
    PKSAV_TEST(gold_save_from_buffer_test)
    PKSAV_TEST(gold_save_from_file_test)

    PKSAV_TEST(pksav_buffer_is_crystal_save_test)
    PKSAV_TEST(pksav_file_is_crystal_save_test)
    PKSAV_TEST(crystal_save_from_buffer_test)
    PKSAV_TEST(crystal_save_from_file_test)
)
