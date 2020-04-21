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
// main.c
// default section
// forms
Boolean About_Form_HandleEvent(EventPtr e);
Boolean Inv_Form_HandleEvent(EventPtr e);
void invOrEqu_drop(Short item);
void refill_lamp(Short ignored);
Boolean Equ_Form_HandleEvent(EventPtr e);
Boolean Recall_Form_HandleEvent(EventPtr e);
Boolean CharRace_Form_HandleEvent(EventPtr e);
Boolean CharGender_Form_HandleEvent(EventPtr e);
Boolean CharClass_Form_HandleEvent(EventPtr e);
Boolean CharSummary_Form_HandleEvent(EventPtr e);
Boolean Store_Form_HandleEvent(EventPtr e);
Boolean Haggle_Form_HandleEvent(EventPtr e);
void haggle_form_enable_autoinc(Short incr) SEC_4;
void haggle_form_boot(Boolean kick_out, Char *str) SEC_4;
void store_list_tens(Short dir) SEC_4;
Boolean Book_Form_HandleEvent(EventPtr e) SEC_4;
Boolean Spell_Form_HandleEvent(EventPtr e) SEC_4;
Boolean GainSp_Form_HandleEvent(EventPtr e) SEC_4;
Boolean MsgLog_Form_HandleEvent(EventPtr e) SEC_5;
Boolean Prefs_Form_HandleEvent(EventPtr e) SEC_5;
void useMoriaPrefs() SEC_5;
void writeMoriaPrefs() SEC_5;
Boolean Map_Form_HandleEvent(EventPtr e) SEC_5;


// dungeon.c
void enter_new_level() SEC_5;
void start_of_turn() SEC_5;
void end_of_turn() SEC_5;
Boolean do_command(Char com_val) SEC_5;
void do_xy_command(Short x, Short y) SEC_5;
void do_dir_command(Short dir, Short XXX_item) SEC_5;

// display.c
// default section
Boolean toggle_itsy();
void where_in_dungeon(Short scr_x, Short scr_y, Short *dun_x, Short *dun_y);
void clear_screen();
void print(Short ch, Short col, Short row);
//void setscreen(Short col, Short row, Short ch);
void recalc_screen();
void refresh();
void relativize_move(Short *x, Short *y) SEC_5;
void change_visible_xy(Short x, Short y);
void slide_dir(Short dir);
//
void count_message(Char *buf);
void message(Char *buf);
//#define unmessage() message(0)
Boolean message_clear();
void alert_message(Char *buf);
void preempt_messages();
void show_messages();
void alloc_message_log();
void print_stats(UChar statsmode);
Boolean panel_contains(Short a, Short b); // FAKE!
Boolean get_panel(Short row, Short col, Boolean foo); // FAKE!

// misc1.c
// all section1
void seed_random() SEC_1;
void set_seed(ULong seed) SEC_1;
void reset_seed() SEC_1;
Int randint(Int y) SEC_1;
Int rund(Int y) SEC_1;
Short randnor(Short mean, Short stand) SEC_1;// not 'correctly' implemented at presnt
Short bit_pos(ULong *test) SEC_1;
Boolean in_bounds(Short y, Short x) SEC_1;
Short distance(Short y1, Short x1, Short y2, Short x2) SEC_1;
Short damroll(Short num, Short sides) SEC_1;
Short pdamroll(UChar *array) SEC_1;
Boolean los(Short fromY, Short fromX, Short toY, Short toX) SEC_1;
UChar loc_symbol(Short y, Short x) SEC_1;
Boolean test_light(Short y, Short x) SEC_1;
Boolean compact_monsters() SEC_1;
void add_food(Short num) SEC_1;
Short popm() SEC_1;
Short max_hp(UChar *array) SEC_1;
Boolean place_monster(Short y, Short x, Short z, Boolean slp) SEC_1;
void place_win_monster() SEC_1;
Short get_mons_num(Short level) SEC_1;
void alloc_monster(Short num, Short dist, Boolean sleep) SEC_1;
Boolean summon_monster(Short *y, Short *x, Boolean slp) SEC_1;
Boolean summon_undead(Short *y, Short *x) SEC_1;
Short popt() SEC_1;
void pusht(UChar x) SEC_1;
Boolean magik(Short chance) SEC_1;
Short m_bonus(Short base, Short max_std, Short level) SEC_1;

// generate.c
// all section1
void generate_cave() SEC_1;

// lock.c
// default section
void data_record_create();
void data_record_lock(Boolean unlocked); // call at startup and shutdown
//void create_cave_rec();
void blank_cave();
void cave_set(Short row, Short col, Short element, UChar val);
//void create_tlist_rec();
void blank_tlist();
// mess with objects-on-level
#define tlist_set_p1 invy_set_p1
#define tlist_set_cost invy_set_cost
#define tlist_set_flags invy_set_flags
#define tlist_set_name2 invy_set_name2
//void tlist_set_p1(Short index, Short p1);
//void tlist_set_flags(Short index, ULong flags);
void tlist_mv(Short from, Short to);
void invcopy(inven_type *to_rec, Short to_index, Short from_index);//was desc.c
void storeinvcopy(inven_record *to_rec, Short to_index, Short from_index);
void blank_mlist();
void blank_mlist();
void mlist_set_short(Short mptr, Short offset, Short val);
void mlist_set_uchar(Short mptr, Short offset, UChar val);
void mlist_set_ml(Short mptr, Boolean ml);
void mlist_mv(Short from, Short to);
// mess with inventory
void invy_set_number(inven_type *iptr, Short index, UChar n) SEC_5; // fixed
void invy_set_flags(inven_type *iptr, Short index, ULong n) SEC_5;
void invy_set_p1(inven_type *iptr, Short index, Short n) SEC_5;
void invy_set_cost(inven_type *iptr, Short index, Long cost) SEC_5;
void invy_set_hitdamac(inven_type *iptr, Short index,
			    Char tohit, Char todam, Char toac) SEC_5;
void invy_set_index(inven_type *iptr, Short index, UShort val) SEC_5;
void invy_delete(Short item_val) SEC_5;
void invy_set_name2(inven_type *iptr, Short index, UChar val) SEC_5;
void invy_insert(Short locn, inven_type *copyme) SEC_5;
void invy_swap(Short item_a, Short item_b) SEC_5;
void invy_set_ident(inven_type *iptr, Short index, UChar ident) SEC_5;
void blank_objident() SEC_5;
// mess with identification status
void object_ident_or(Short index, UChar or_me) SEC_5;
void object_ident_nand(Short index, UChar nand_me) SEC_5;
// mess with the monster memory (c_recall)
void recall_cmove_or(Short index, ULong or_me);
void recall_set_cmove(Short index, ULong val);
void recall_spells_or(Short index, ULong val);
void recall_set_spells(Short index, ULong val);
void recall_increment_kills(Short index);
void recall_increment_deaths(Short index);
void recall_cdefense_or(Short index, UShort or_me);
void recall_set_wake(Short index, UChar val);
void recall_set_ignore(Short index, UChar val);
void recall_increment_attacks(Short index, Short attindex);
Char *get_haggle_string(Short i) SEC_4;


// sets.c
// these could practically be macros, or something
// all section1
Boolean is_room(UChar element) SEC_1;
Boolean is_corr(UChar element) SEC_1;
Boolean is_floor(UChar element) SEC_1;
Boolean is_corrodes(inven_type *item) SEC_1;
Boolean is_flammable(inven_type *item) SEC_1;
Boolean is_frost_destroy(inven_type *item) SEC_1;
Boolean is_acid_affect(inven_type *item) SEC_1;
Boolean is_lightning_destroy(inven_type *item) SEC_1;
Boolean is_null(inven_type *item) SEC_1;
Boolean is_acid_destroy(inven_type *item) SEC_1;
Boolean is_fire_destroy(inven_type *item) SEC_1;
Boolean is_large(treasure_type *item) SEC_1;
Boolean is_general_store(UChar element) SEC_1;
Boolean is_armory(UChar element) SEC_1;
Boolean is_weaponsmith(UChar element) SEC_1;
Boolean is_temple(UChar element) SEC_1;
Boolean is_alchemist(UChar element) SEC_1;
Boolean is_magic_shop(UChar element) SEC_1;
Boolean is_store_buy(Short storeno, UChar element) SEC_1;

// misc2.c
// all section2
void magic_treasure(Short x, Short level) SEC_2;
void set_options() SEC_2;

// misc3.c
// all section1 (so far)
void place_trap(Short y, Short x, Short subval) SEC_1;
void place_rubble(Short y, Short x) SEC_1;
void place_gold(Short y, Short x) SEC_1;
Short get_obj_num(Short level, Boolean must_be_small) SEC_1;
void place_object(Short y, Short x, Boolean must_be_small) SEC_1;
void alloc_object(UChar alloc_set, Short typ, Short num) SEC_1;
void random_object(Short y, Short x, Short num) SEC_1;
Short stat_adj(Short stat) SEC_1;
Short chr_adj() SEC_1;
Short con_adj() SEC_1;
UChar modify_stat (Short stat, Short amount) SEC_1;
void set_use_stat(Short stat) SEC_1;
Boolean inc_stat(Short stat) SEC_1;
Boolean dec_stat(Short stat) SEC_1;
Boolean res_stat (Short stat) SEC_1;
void bst_stat (Short stat, Short amount) SEC_1;
Short tohit_adj() SEC_1;
Short toac_adj() SEC_1;
Short todis_adj() SEC_1;
Short todam_adj() SEC_1;
void draw_cave() SEC_1;
void inven_destroy(Short item_val) SEC_1;
void take_one_item(inven_type *s_ptr, inven_type *i_ptr) SEC_1;
void inven_drop(Short item_val, Boolean drop_all) SEC_1;
Short inven_damage(Boolean (*typ)(), Short perc) SEC_1;
//Short inven_damage(UChar typ, Short perc) SEC_1;
Short weight_limit() SEC_1;
Boolean inven_similar(inven_type *a_ptr, inven_type *b_ptr) SEC_1;
Boolean inven_check_num (inven_type *t_ptr) SEC_1;
Boolean inven_check_weight(inven_type *i_ptr) SEC_1;
void check_strength() SEC_1;
Short inven_carry(inven_type *i_ptr) SEC_1;
Short spell_chance(Short spell) SEC_4;
     //...
void calc_spells(Short stat) SEC_4;
void gain_spells() SEC_4;
void gain_spell_helper() SEC_4;
void calc_mana(Short stat) SEC_4;
void calc_hitpoints() SEC_4;
     //...
void insert_lnum(Char *object_str, const Char *mtc_str,
		 Long number, Boolean signp) SEC_1;
void insert_str(Char *object_str, Char *mtc_str, Char *insert) SEC_1;
Short attack_blows(Short weight, Short *wtohit) SEC_1;
Short tot_dam(inven_type *i_ptr, Short tdam, Short monster) SEC_1;
Short critical_blow(Short weight, Short plus, Short dam,
		    Short attack_type) SEC_1;
Boolean mmove(Short dir, Short *y, Short *x) SEC_1;
Boolean player_saves() SEC_1;
Boolean find_range(Short item1, Short item2, Short *j, Short *k) SEC_1;
void teleport(Short dis) SEC_1;

// misc4.c
void scribe_object() SEC_3;
void add_inscribe(inven_type *i_ptr, Short index, UChar type) SEC_3;
void inscribe(Short iptr, Char *str) SEC_3;
void check_view() SEC_3;


// moria1.c
void change_speed(Short num) SEC_1;
void py_bonuses(inven_type *t_ptr, Short factor) SEC_1;
void calc_bonuses() SEC_1;
Char *describe_use(Short i) SEC_1;
void takeoff(Short item_val, Short posn) SEC_1;
Boolean no_light() SEC_1;
void move_rec(Short y1, Short x1, Short y2, Short x2) SEC_1;
void light_room(Short y, Short x) SEC_1;
void lite_spot(Short y, Short x) SEC_1;
//static void sub1_move_light(Short y1, Short x1, Short y2, Short x2);
//static void sub3_move_light(Short y1, Short x1, Short y2, Short x2);
void move_light(Short y1, Short x1, Short y2, Short x2) SEC_1;
void disturb(Boolean se, Boolean li) SEC_1;
void search_on() SEC_1;
void search_off() SEC_1;
void rest() SEC_5;
void rest_off() SEC_1;
Boolean test_hit(Short bth, Short level, Short pth, Short ac,
		 Short attack_typ) SEC_1;
void take_hit(Short damage, Char *hit_from) SEC_1;

// moria2.c
void change_trap(Short y, Short x) SEC_2;
void search(Short y, Short x, Short chance) SEC_2;
void find_init(Short dir) SEC_2;
void find_run() SEC_2;
void end_find() SEC_2;
//static Boolean see_wall(Short dir, Short y, Short x) SEC_2;
//static Boolean see_nothing(Short dir, Short y, Short x) SEC_2;
void area_affect(Short dir, Short y, Short x) SEC_2;
Boolean minus_ac(ULong typ_dam) SEC_2;
void corrode_gas(Char *kb_str) SEC_2;
void poison_gas(Short dam, Char *kb_str) SEC_2;
void fire_dam(Short dam, Char *kb_str) SEC_2;
void cold_dam(Short dam, Char *kb_str) SEC_2;
void light_dam(Short dam, Char *kb_str) SEC_2;
void acid_dam(Short dam, Char *kb_str) SEC_2;

// moria3.c  (moria3a.c)  There is more yet to come.
void delete_monster(Short j) SEC_3;
Short cast_spell(Char *prompt, Short item_val, Short *sn, Short *sc) SEC_3;//xxx not implemented yet
void fix1_delete_monster(Short j) SEC_3;
void fix2_delete_monster(Short j) SEC_3;
Boolean delete_object(Short y, Short x) SEC_3;
ULong monster_death(Short y, Short x, ULong flags) SEC_3;
Short mon_take_hit(Short monptr, Short dam) SEC_3;
void py_attack(Short y, Short x) SEC_3;
void move_char(Short dir, Boolean do_pickup) SEC_3;// xxx
void chest_trap(Short y, Short x) SEC_3;
void openobject(Short dir) SEC_3; // xxx
void closeobject(Short dir) SEC_3;// xxx
Boolean twall(Short y, Short x, Short t1, Short t2) SEC_3;


// moria4.c
void tunnel(Short dir) SEC_3;
void disarm_trap(Short dir) SEC_3;
void look(Short dir) SEC_3;
void throw_object(Short item_val, Short dir) SEC_3;
void bash(Short dir) SEC_3;



// desc.c
Boolean is_a_vowel(Char ch) SEC_1;
Short object_offset(inven_type *t_ptr) SEC_1;
void known1(inven_type *i_ptr) SEC_1;
Boolean known1_p(inven_type *i_ptr) SEC_1;
void known2(inven_type *i_ptr, Short index) SEC_1;
Boolean known2_p(inven_type *i_ptr) SEC_1;
void clear_known2(inven_type *i_ptr, Short index) SEC_1;
void clear_empty(inven_type *i_ptr, Short index) SEC_1;
void store_bought(inven_type *i_ptr, Short index) SEC_1;
Boolean store_bought_p(inven_type *i_ptr) SEC_1;
void sample (inven_type *i_ptr) SEC_1;
void identify(Short *item) SEC_1;
void unmagic_name(Short iptr) SEC_1;
void objdes(Char *out_val, inven_type *i_ptr, Boolean pref) SEC_1;
void desc_charges(Short item_val) SEC_1;
void desc_remain(Short item_val) SEC_1;


// recall.c
Boolean bool_roff_recall(Short mon_num) SEC_3;
void roff_recall(Short mon_num, Char *buf) SEC_3;

// util.c
void update_field_scrollers(FormPtr frm, FieldPtr fld,
			    Word up_scroller, Word down_scroller);
void page_scroll_field(FormPtr frm, FieldPtr fld, DirectionType dir);
void LeaveForm();
Char * md_malloc(Int n);

// spells.c  (spells-a.c, but not missing any functions)
void monster_name(Char *m_name, monster_type *m_ptr,
		  creature_type *r_ptr) SEC_2;
void lower_monster_name(Char *m_name, monster_type *m_ptr,
			creature_type *r_ptr) SEC_2;
Boolean sleep_monsters1(Short y, Short x) SEC_2;
Boolean detect_treasure() SEC_2;
Boolean detect_object() SEC_2;
Boolean detect_trap() SEC_2;
Boolean detect_sdoor() SEC_2;
Boolean detect_invisible() SEC_2;
Boolean light_area(Short y, Short x) SEC_2;
Boolean unlight_area(Short y, Short x) SEC_2;
void map_area() SEC_2;
Short ident_spell() SEC_2;
Boolean aggravate_monster(Short dis_affect) SEC_2;
Boolean trap_creation() SEC_2;
Boolean door_creation() SEC_2;
Short td_destroy() SEC_2;
Boolean detect_monsters() SEC_2;
void light_line(Short dir, Short y, Short x) SEC_2;
void starlite(Short y, Short x) SEC_2;
Boolean disarm_all(Short dir, Short y, Short x) SEC_2;
//static void get_flags(Short typ, ULong *weapon_type, Short *harm_type, 
//		      Short (**destroy)() ) SEC_2;
void fire_bolt(Short typ, Short dir, Short y, Short x, Short dam,
	       Char *bolt_typ) SEC_2;
void fire_ball(Short typ, Short dir, Short y, Short x, Short dam_hp,
	       Char *descrip) SEC_2;
void breath(Short typ, Short y, Short x, Short dam_hp, Char *ddesc,
	    Short monptr) SEC_2;
Boolean recharge(Short num) SEC_2;
Boolean hp_monster(Short dir, Short y, Short x, Short dam) SEC_2;
Boolean drain_life(Short dir, Short y, Short x) SEC_2;
Boolean speed_monster(Short dir, Short y, Short x, Short spd) SEC_2;
Boolean confuse_monster(Short dir, Short y, Short x) SEC_2;
Boolean sleep_monster(Short dir, Short y, Short x) SEC_2;
Boolean wall_to_mud(Short dir, Short y, Short x) SEC_2;
Boolean td_destroy2(Short dir, Short y, Short x) SEC_2;
Boolean poly_monster(Short dir, Short y, Short x) SEC_2;
Boolean build_wall(Short dir, Short y, Short x) SEC_2;
Boolean clone_monster(Short dir, Short y, Short x) SEC_2;
void teleport_away(Short monptr, Short dis) SEC_2;
void teleport_to(Short ny, Short nx) SEC_2;
Boolean teleport_monster(Short dir, Short y, Short x) SEC_2;
Boolean mass_genocide() SEC_2;
Boolean genocide() SEC_2;
Boolean speed_monsters(Short spd) SEC_2;
Boolean sleep_monsters2() SEC_2;
Boolean mass_poly() SEC_2;
Boolean detect_evil() SEC_2;
Boolean hp_player(Short num) SEC_2;
Boolean cure_confusion() SEC_2;
Boolean cure_blindness() SEC_2;
Boolean cure_poison() SEC_2;
Boolean remove_fear() SEC_2;
void earthquake() SEC_2;
Boolean protect_evil() SEC_2;
void create_food() SEC_2;
Boolean dispel_creature(Short cflag, Short damage) SEC_2;
Boolean turn_undead() SEC_2;
void warding_glyph() SEC_2;
void lose_str() SEC_2;
void lose_int() SEC_2;
void lose_wis() SEC_2;
void lose_dex() SEC_2;
void lose_con() SEC_2;
void lose_chr() SEC_2;
void lose_exp(Long amount) SEC_2;
Boolean slow_poison() SEC_2;
void bless(Short amount) SEC_2;
void detect_inv2(Short amount) SEC_2;
//static void replace_spot(Short y, Short x, Short typ) SEC_2;
void destroy_area(Short y, Short x) SEC_2;
Boolean enchant(Short *plusses, Short limit) SEC_2;
Boolean remove_curse() SEC_2;
Boolean restore_level() SEC_2;

// eat.c
void eat(Short item_val) SEC_2;
Boolean get_item(Short *, Char *, Short, Short, Char *, Char *); // this is a dummy function and should be replaced by form type stuff.

// potions.c
void quaff(Short item_val) SEC_2;

// staffs.c
void use(Short item_val) SEC_3;

// wands.c
void aim(Short item_val, Short dir) SEC_3;

// magic.c
void cast() SEC_3;
void cast_helper(Short result, Short choice, Short chance) SEC_3;

// prayer.c
void pray() SEC_3;
void pray_helper(Short result, Short choice, Short chance) SEC_3;

// misc5.c
void draw_directional() SEC_3;
void undraw_directional() SEC_3;
Short convert_to_dir(Short x, Short y, Short ignore_center) SEC_3;
void dispatch_directional(Short dir) SEC_3;

// store1.c
Long item_value(inven_type *i_ptr) SEC_4;
Long sell_price(Short snum, Long *max_sell, Long *min_sell,
		inven_type *item) SEC_4;
Boolean store_check_num(inven_type *t_ptr, Short store_num) SEC_4;
void store_carry(Short store_num, Short *ipos, inven_type *t_ptr) SEC_4;
void store_destroy(Short store_num, Short item_val, Boolean one_of) SEC_4;
void store_init() SEC_4;
void store_maint() SEC_4;
Boolean noneedtobargain(Short store_num, Long minprice) SEC_4;
void updatebargain(Short store_num, Long price, Long minprice) SEC_4;

// store2.c
Long haggle_get_ask() SEC_4;
Long haggle_get_incr() SEC_4;
void purchase_haggle_init() SEC_4;
Short purchase_haggle_endturn(Long new_offer) SEC_4;
void sell_haggle_init() SEC_4;
Short sell_haggle_endturn(Long new_offer) SEC_4;
Boolean store_purchase_init(Short store_num, Short item_val) SEC_4;
void store_purchase_end(Boolean in_haggle) SEC_4;
Boolean store_sell_init(Short store_num, Short item_val) SEC_4;
void store_sell_end(Boolean in_haggle) SEC_4;
Boolean enter_store(Short store_num) SEC_4;

// creature.c
void update_mon(Short monptr) SEC_4;
Boolean multiply_monster(Short y, Short x, Short cr_index, Short monptr) SEC_4;
void creatures(Short attack) SEC_5;

