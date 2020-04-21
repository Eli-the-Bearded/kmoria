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

// SEE ALSO: the old variable.c

/* source/externs.h: declarations for global variables and initialized data */

#include "Pilot.h"

// How big is this?  assuming I get rid of all arrays
// 29 bool  = 29
//  4 char  =  4
// 24 Short = 48
//  6 Long  = 24
//           105 B
// there's only one of it
// but don't forget there's also 'py'
typedef struct everything_s {
  Char died_from[80]; // vtype is just a char[80]
  //extern vtype savefile;			/* The save file. -CJS- */
  Long birth_date; // int 32
  
  /* These are options, set with set_options command -CJS- */
  // Perhaps these could be a bitfield.
  Boolean rogue_like_commands;
  Boolean find_cut;			/* Cut corners on a run */
  Boolean find_examine;		        /* Check corners on a run */
  Boolean find_prself;			/* Print yourself on a run (slower) */
  Boolean find_bound;			/* Stop run when the map shifts */
  Boolean prompt_carry_flag;		/* Prompt to pick something up */
  Boolean show_weight_flag;		/* Display weights in inventory */
  Boolean highlight_seams;		/* Highlight magma and quartz */
  Boolean find_ignore_doors;		/* Run through open doors */
  Boolean sound_beep_flag;		/* Beep for invalid character */
  Boolean display_counts;		/* Display rest/repeat counts */
  /* global flags */
  Boolean new_level_flag;	  /* Next level when true  */
  Boolean teleport_flag;	/* Handle teleport traps  */
  Boolean eof_flag;		/* Used to handle eof/HANGUP */
  Boolean player_light;      /* Player carrying light */
  Boolean find_flag;	/* Used in MORIA	      */
  Boolean free_turn_flag;	/* Used in MORIA	      */
  Boolean weapon_heavy;	/* Flag if the weapon too heavy -CJS- */
  Boolean pack_heavy;		/* Flag if the pack too heavy -CJS- */
  extern char doing_inven;	/* Track inventory commands */
  Boolean screen_change;	/* Screen changes (used in inven_commands) */
  Boolean character_generated;	 /* don't save score until char gen finished*/
  Boolean character_saved;	 /* prevents save on kill after save_char() */
  //extern FILE *highscore_fp;	/* High score file pointer */
  Short command_count;	/* Repetition of commands. -CJS- */
  Boolean default_dir;		/* Use last direction in repeated commands */
  bitfield3 noscore;		/* Don't score this game. -CJS- */
  //extern int32u randes_seed;    /* For encoding colors */
  //extern int32u town_seed;	    /* Seed for town genera*/
  Short dun_level;	/* Cur dungeon level   */
  Short missile_ctr;	/* Counter for missiles */
  Boolean msg_flag;	/* Set with first msg  */
  //extern vtype old_msg[MAX_SAVE_MSG];	/* Last messages -CJS- */
  //extern int16 last_msg;		/* Where in the array is the last */
  Boolean death;	/* True if died	      */
  Long turn;	/* Cur trun of game    */
  Boolean wizard;	/* Wizard flag	      */
  Boolean to_be_wizard;
  //extern int16 panic_save; /* this is true if playing from a panic save */
  Boolean wait_for_more;
  //  Char days[7][29]; // I think this is only used to restrict game play
  Char closing_flag;	/* Used for closing   */
  Short cur_height, cur_width;	/* Cur dungeon size    */
  /*  Following are calculated from max dungeon sizes		*/
  Short max_panel_rows, max_panel_cols;
  Short panel_row, panel_col;
  Short panel_row_min, panel_row_max;
  Short panel_col_min, panel_col_max;
  Short panel_col_prt, panel_row_prt;

  /*  Following are all floor definitions				*/
  // Could this be a record? it is now
  //  cave_type cave[MAX_HEIGHT][MAX_WIDTH];

  /* Following are player variables				*/
  player_type py;
  // Could the remainder of these also be records?
  //  Char *player_title[MAX_CLASS][MAX_PLAYER_LEVEL];
  race_type race[MAX_RACES];
  //  background_type background[MAX_BACKGROUND];
  //  ULong player_exp[MAX_PLAYER_LEVEL];
  //  UShort player_hp[MAX_PLAYER_LEVEL];

  Short char_row;
  Short char_col;
  // Could the remainder of these also be records?
  //  UChar rgold_adj[MAX_RACES][MAX_RACES];
  class_type class[MAX_CLASS];
  Short class_level_adj[MAX_CLASS][MAX_LEV_ADJ];

  /* Warriors don't have spells, so there is no entry for them. */
  // Could the remainder of these also be records?
  spell_type magic_spell[MAX_CLASS-1][31];
  //  Char *spell_names[62];

  ULong spell_learned;	/* Bit field for spells learnt -CJS- */
  ULong spell_worked;	/* Bit field for spells tried -CJS- */
  ULong spell_forgotten;	/* Bit field for spells forgotten -JEW- */
  // Could the remainder of these also be records?
  //  UChar spell_order[32];	/* remember order that spells are learned in */
  UShort player_init[MAX_CLASS][5];
  Short total_winner;
  Long max_score;

  /* Following are store definitions				*/
  // Could the remainder of these also be records?
  //  owner_type owners[MAX_OWNERS];
  store_type store[MAX_STORES];
  UShort store_choice[MAX_STORES][STORE_CHOICES];

  Short (*store_buy[MAX_STORES])(); // whoa Nellie

  /* Following are treasure arrays	and variables			*/
  // Could the remainder of these also be records?
  treasure_type object_list[MAX_OBJECTS];
  UChar object_ident[OBJECT_IDENT_SIZE];
  Short t_level[MAX_OBJ_LEVEL+1];
  inven_type t_list[MAX_TALLOC];
  inven_type inventory[INVEN_ARRAY_SIZE];
  //  Char *special_names[SN_ARRAY_SIZE];
  Short sorted_objects[MAX_DUNGEON_OBJ];

  Short inven_ctr;		/* Total different obj's	*/
  Short inven_weight;	/* Cur carried weight	*/
  Short equip_ctr;	/* Cur equipment ctr	*/
  Short tcptr;	/* Cur treasure heap ptr	*/

  /* Following are creature arrays and variables			*/
  // Could the remainder of these also be records?
  creature_type c_list[MAX_CREATURES];
  monster_type m_list[MAX_MALLOC];
  Short m_level[MAX_MONS_LEVEL+1];
  //  m_attack_type monster_attacks[N_MONS_ATTS];
  recall_type c_recall[MAX_CREATURES];	/* Monster memories. -CJS- */

  monster_type blank_monster;	/* Blank monster values	*/
  Short mfptr;	/* Cur free monster ptr	*/
  Short mon_tot_mult;	/* # of repro's of creature	*/

  /* Following are arrays for descriptive pieces			*/
  // Now records
  //  Char *colors[MAX_COLORS];
  //  Char *mushrooms[MAX_MUSH];
  //  Char *woods[MAX_WOODS];
  //  Char *metals[MAX_METALS];
  //  Char *rocks[MAX_ROCKS];
  //  Char *amulets[MAX_AMULETS];
  //  Char *syllables[MAX_SYLLABLES];

  UChar blows_table[7][6]; // weapon blows

  // Now a record
  //  UShort normal_table[NORMAL_TABLE_SIZE];

  /* Initialized data which had to be moved from some other file */
  /* Since these get modified, macrsrc.c must be able to access them */
  /* Otherwise, game cannot be made restartable */
  /* dungeon.c */
  // This is basically only used for UI things I will be revising.
  //  extern char last_command;  /* Memory of previous command. */
  /* moria1.c */
  /* Track if temporary light about player.  */
  Boolean light_flag;

  // There were also function prototypes in this file.
  // These will have been moved out into prototypes.h or something
} everything;
