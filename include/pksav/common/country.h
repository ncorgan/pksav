/*!
 * @file    pksav/common/country.h
 * @ingroup PKSav
 * @brief   Native values for storing a country of origin.
 *
 * Copyright (c) 2017 Nicholas Corgan (n.corgan@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#ifndef PKSAV_COMMON_COUNTRY_H
#define PKSAV_COMMON_COUNTRY_H

typedef enum
{
    PKSAV_COUNTRY_JAPAN = 1,
    PKSAV_COUNTRY_ANGUILLA = 8,
    PKSAV_COUNTRY_ANTIGUA_AND_BARBUDA = 9,
    PKSAV_COUNTRY_ARGENTINA = 10,
    PKSAV_COUNTRY_ARUBA = 11,
    PKSAV_COUNTRY_BAHAMAS = 12,
    PKSAV_COUNTRY_BARBADOS = 13,
    PKSAV_COUNTRY_BELIZE = 14,
    PKSAV_COUNTRY_BOLIVIA = 15,
    PKSAV_COUNTRY_BRAZIL = 16,
    PKSAV_COUNTRY_BRITISH_VIRGIN_ISLANDS = 17,
    PKSAV_COUNTRY_CANADA = 18,
    PKSAV_COUNTRY_CAYMAN_ISLANDS = 19,
    PKSAV_COUNTRY_CHILE = 20,
    PKSAV_COUNTRY_COLOMBIA = 21,
    PKSAV_COUNTRY_COSTA_RICA = 22,
    PKSAV_COUNTRY_DOMINICA = 23,
    PKSAV_COUNTRY_DOMINICAN_REPUBLIC = 24,
    PKSAV_COUNTRY_ECUADOR = 25,
    PKSAV_COUNTRY_EL_SALVADOR = 26,
    PKSAV_COUNTRY_FRENCH_GUIANA = 27,
    PKSAV_COUNTRY_GRENADA = 28,
    PKSAV_COUNTRY_GUADELOUPE = 29,
    PKSAV_COUNTRY_GUATEMALA = 30,
    PKSAV_COUNTRY_GUYANA = 31,
    PKSAV_COUNTRY_HAITI = 32,
    PKSAV_COUNTRY_HONDURAS = 33,
    PKSAV_COUNTRY_JAMAICA = 34,
    PKSAV_COUNTRY_MARTINIQUE = 35,
    PKSAV_COUNTRY_MEXICO = 36,
    PKSAV_COUNTRY_MONTSERRAT = 37,
    PKSAV_COUNTRY_NETHERLANDS_ANTILLES = 38,
    PKSAV_COUNTRY_NICARAGUA = 39,
    PKSAV_COUNTRY_PANAMA = 40,
    PKSAV_COUNTRY_PARAGUAY = 41,
    PKSAV_COUNTRY_PERU = 42,
    PKSAV_COUNTRY_ST_KITTS_AND_NEVIS = 43,
    PKSAV_COUNTRY_ST_LUCIA = 44,
    PKSAV_COUNTRY_ST_VINCENT_AND_THE_GRENADINES = 45,
    PKSAV_COUNTRY_SURINAME = 46,
    PKSAV_COUNTRY_TRINIDAD_AND_TOBAGO = 47,
    PKSAV_COUNTRY_TURKS_AND_CAICOS_ISLANDS = 48,
    PKSAV_COUNTRY_UNITED_STATES = 49,
    PKSAV_COUNTRY_URUGUAY = 50,
    PKSAV_COUNTRY_US_VIRGIN_ISLANDS = 51,
    PKSAV_COUNTRY_VENEZUELA = 52,
    PKSAV_COUNTRY_ALBANIA = 64,
    PKSAV_COUNTRY_AUSTRALIA = 65,
    PKSAV_COUNTRY_AUSTRIA = 66,
    PKSAV_COUNTRY_BELGIUM = 67,
    PKSAV_COUNTRY_BOSNIA_AND_HERZEGOVINA = 68,
    PKSAV_COUNTRY_BOTSWANA = 69,
    PKSAV_COUNTRY_BULGARIA = 70,
    PKSAV_COUNTRY_CROATIA = 71,
    PKSAV_COUNTRY_CYPRUS = 72,
    PKSAV_COUNTRY_CZECH_REPUBLIC = 73,
    PKSAV_COUNTRY_DENMARK_KINGDOM_OF = 74,
    PKSAV_COUNTRY_ESTONIA = 75,
    PKSAV_COUNTRY_FINLAND = 76,
    PKSAV_COUNTRY_FRANCE = 77,
    PKSAV_COUNTRY_GERMANY = 78,
    PKSAV_COUNTRY_GREECE = 79,
    PKSAV_COUNTRY_HUNGARY = 80,
    PKSAV_COUNTRY_ICELAND = 81,
    PKSAV_COUNTRY_IRELAND = 82,
    PKSAV_COUNTRY_ITALY = 83,
    PKSAV_COUNTRY_LATVIA = 84,
    PKSAV_COUNTRY_LESOTO = 85,
    PKSAV_COUNTRY_LIECHTENSTEIN = 86,
    PKSAV_COUNTRY_LITHUANIA = 87,
    PKSAV_COUNTRY_LUXEMBOURG = 88,
    PKSAV_COUNTRY_MACEDONIA_REPUBLIC_OF = 89,
    PKSAV_COUNTRY_MALTA = 90,
    PKSAV_COUNTRY_MONTENEGRO = 91,
    PKSAV_COUNTRY_MOZAMBIQUE = 92,
    PKSAV_COUNTRY_NAMIBIA = 93,
    PKSAV_COUNTRY_NETHERLANDS = 94,
    PKSAV_COUNTRY_NEW_ZEALAND = 95,
    PKSAV_COUNTRY_NORWAY = 96,
    PKSAV_COUNTRY_POLAND = 97,
    PKSAV_COUNTRY_PORTUGAL = 98,
    PKSAV_COUNTRY_ROMANIA = 99,
    PKSAV_COUNTRY_RUSSIA = 100,
    PKSAV_COUNTRY_SERBIA_AND_KOSOVO = 101,
    PKSAV_COUNTRY_SLOVAKIA = 102,
    PKSAV_COUNTRY_SLOVENIA = 103,
    PKSAV_COUNTRY_SOUTH_AFRICA = 104,
    PKSAV_COUNTRY_SPAIN = 105,
    PKSAV_COUNTRY_SWAZILAND = 106,
    PKSAV_COUNTRY_SWEDEN = 107,
    PKSAV_COUNTRY_SWITZERLAND = 108,
    PKSAV_COUNTRY_TURKEY = 109,
    PKSAV_COUNTRY_UNITED_KINGDOM = 110,
    PKSAV_COUNTRY_ZAMBIA = 111,
    PKSAV_COUNTRY_ZIMBABWE = 112,
    PKSAV_COUNTRY_AZERBAIJAN = 113,
    PKSAV_COUNTRY_MAURITANIA = 114,
    PKSVA_COUNTRY_MALI = 115,
    PKSAV_COUNTRY_NIGER = 116,
    PKSAV_COUNTRY_CHAD = 117,
    PKSAV_COUNTRY_SUDAN = 118,
    PKSAV_COUNTRY_ERITREA = 119,
    PKSAV_COUNTRY_DJIBOUTI = 120,
    PKSAV_COUNTRY_SOMALIA = 121,
    PKSAV_COUNTRY_ANDORRA = 122,
    PKSAV_COUNTRY_GIBRALTAR = 123,
    PKSAV_COUNTRY_GUERNSEY = 124,
    PKSAV_COUNTRY_ISLE_OF_MAN = 125,
    PKSAV_COUNTRY_JERSEY = 126,
    PKSAV_COUNTRY_MONACO = 127,
    PKSAV_COUNTRY_TAIWAN = 128,
    PKSAV_COUNTRY_SOUTH_KOREA = 136,
    PKSAV_COUNTRY_HONG_KONG = 144,
    PKSAV_COUNTRY_SINGAPORE = 153,
    PKSAV_COUNTRY_MALAYSIA = 156,
    PKSAV_COUNTRY_CHINA = 160,
    PKSAV_COUNTRY_UAE = 168,
    PKSAV_COUNTRY_INDIA = 169,
    PKSAV_COUNTRY_SAUDI_ARABIA = 174,
    PKSAV_COUNTRY_SAN_MARINO = 184,
    PKSAV_COUNTRY_VATICAN_CITY = 185,
    PKSAV_COUNTRY_BERMUDA = 186
} pksav_country_t;

#endif /* PKSAV_COMMON_COUNTRY_H */
