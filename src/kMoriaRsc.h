/*********************************************************************
 * kMoria - Moria adapted for the PalmPilot.                         *
 * Copyright (C) 2000 Bridget Spitznagel                             *
 *                                                                   *
 * This program is derived from Umoria 5.5.2.  Moria is              *
 *   Copyright (c) 1989-94 James E. Wilson, Robert A. Koeneke        *
 *                                                                   *
 * This program is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU General Public License       *
 * as published by the Free Software Foundation; either version 2    *
 * of the License, or (at your option) any later version.            *
 *                                                                   *
 * This program is distributed in the hope that it will be useful,   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License for more details.                      *
 *                                                                   *
 * You should have received a copy of the GNU General Public License *
 * along with this program; if not, write to                         *
 * The Free Software Foundation, Inc.,                               *
 * 59 Temple Place - Suite 330,                                      *
 * Boston, MA  02111-1307, USA.                                      *
 *********************************************************************/
#define MainForm              1000
#define ItsyFont              1009
#define MainFormMenu          1010
#define menu_mainMap          1011
#define menu_mainSettings     1012
#define menu_mainButtons      1013
#define menu_mainMoveInstruct 1014
#define menu_mainAbout        1015
#define menu_mainFont         1016
#define menu_mainRecall       1017
#define menu_mainStats        1018
#define menu_mainMsgs         1019

#define AboutStr              1021
#define QuitP                 1022
#define MapForm               1023
#define LowMageP              1024
#define LowPriestP            1025

#define PrefsForm             1100
#define list_prf_1            1101
#define list_prf_2            1103
#define btn_prf_ok            1105
#define btn_prf_cancel        1106
#define check_prf_1           1111
#define check_prf_2           1112
#define check_prf_3           1113
#define check_prf_4           1114
#define check_prf_5           1115
#define check_prf_6           1116
#define check_prf_7           1117
#define check_prf_8           1118
#define check_prf_9           1119
#define check_prf_10          1120
#define check_prf_11          1121
#define check_prf_12          1122

#define InvForm      1200
#define list_if      1201
// do NOT change these three numbers
#define btn_if_frob  1202
#define btn_if_drop  1203
#define btn_if_throw 1204
#define btn_if_eq    1205
#define btn_if_cancel 1206

#define EquForm      1220
#define list_eq      1221
#define btn_eq_off   1202
#define btn_eq_drop  1203
//#define btn_eq_throw 1204
#define btn_eq_if    1205
#define btn_eq_cancel 1206

#define StoreForm    1230
#define list_st      1231
#define btn_st_buy   1232
#define btn_st_prev  1233
#define btn_st_next  1234
#define btn_st_sell  1235
#define btn_st_exit  1236
#define HaggleForm   1240
#define field_hag    1241
#define btn_hag_all  1242
#define btn_hag_ok   1243
#define btn_hag_exit 1244
#define btn_hag_incr 1245
#define btn_hag_boot 1246

#define BookForm         1260
#define list_book        1261
#define btn_book_ok      1262
#define btn_book_cancel  1263
#define SpellForm        1264
#define list_spell	 1265
#define btn_spell_ok     1266  
#define btn_spell_cancel 1267
#define GainSpForm       1270
#define list_gain        1271
#define btn_gain_ok      1272
#define btn_gain_cancel  1273
#define btn_gain_tog     1274


#define RecallForm         1300
#define field_recall       1301
#define repeat_recall_up   1302
#define repeat_recall_down 1303
#define btn_recall_ok      1304

#define MsgLogForm         1310
#define field_ml           1311
#define repeat_ml_up       1312
#define repeat_ml_down     1313
#define btn_ml_ok          1314

#define AboutForm            3020
#define btn_about_ok         3021
#define bitmap_moria         3022
#define bitmap_cat           3023
#define btn_about_more       3024


// Character generation.
#define CharGenStr       3090
#define CharRaceForm     3091
#define CharGenderForm   3092
#define CharClassForm    3093
#define CharSummaryForm  3094
#define pbtn_race_0      3100
#define pbtn_race_1      3101
#define pbtn_race_2      3102
#define pbtn_race_3      3103
#define pbtn_race_4      3104
#define pbtn_race_5      3105
#define pbtn_race_6      3106
#define pbtn_race_7      3107
#define btn_race_ok      3109
#define pbtn_gender_0    3110
#define pbtn_gender_1    3111
#define btn_gender_no    3112
#define btn_gender_ok    3119
#define pbtn_class_0     3120
#define pbtn_class_1     3121
#define pbtn_class_2     3122
#define pbtn_class_3     3123
#define pbtn_class_4     3124
#define pbtn_class_5     3125
#define btn_class_no     3126
#define btn_class_reroll 3127
#define btn_class_ok     3128
#define btn_class_ok2    3129
#define field_charsum    3130
#define btn_charsum_no   3138
#define btn_charsum_ok   3139

// Do not mess with these numbers.  They are an ascii-related hack.
#define MAGIC_MENU_NUMBER 4000
#define menu_cmd_space   4032
#define menu_cmd_minus   4045
#define menu_cmd_lt      4060
#define menu_cmd_gt      4062
#define menu_cmd_look    4063
#define menu_cmd_G       4071
#define menu_cmd_quit    4081
#define menu_cmd_R       4082
#define menu_cmd_S       4083
#define menu_cmd_disarm  4094
#define menu_cmd_a       4097
#define menu_cmd_c       4099
#define menu_cmd_d       4100
#define menu_cmd_e       4101
#define menu_cmd_f       4102
#define menu_cmd_i       4105
#define menu_cmd_m       4109
#define menu_cmd_o       4111
#define menu_cmd_p       4112
#define menu_cmd_s       4115
#define menu_cmd_t       4116
#define menu_cmd_x       4120
#define menu_cmd_z       4122
