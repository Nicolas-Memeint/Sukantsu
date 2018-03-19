#include "riichi_engine.h"
#include "../AI/detect.h"
#include "../console_io.h"
#include "../debug.h"

void init_riichi_engine(struct riichi_engine *engine, enum player_type t1,
                        enum player_type t2, enum player_type t3,
                        enum player_type t4) {
	ASSERT_BACKTRACE(engine);

	init_player(&engine->players[0], t1);
	init_player(&engine->players[1], t2);
	init_player(&engine->players[2], t3);
	init_player(&engine->players[3], t4);

	engine->nb_games = 0;
}

// Play a riichi game and return the index of the player who has won
// If noone has won, return -1
int play_riichi_game(struct riichi_engine *engine) {
	ASSERT_BACKTRACE(engine);

	++engine->nb_games;
	engine->phase = PHASE_INIT;

	// Initialize structures
	init_histogram(&engine->wall, 4);
	for (int i = 0; i < NB_PLAYERS; ++i) {
		// player_type has been initialized in init_riichi_engine
		init_hand(&engine->players[i].hand);
	}

	// Give 13 tiles to each player
	for (int i = 0; i < 13; ++i) {
		for (int p = 0; p < NB_PLAYERS; ++p) {
			histo_index_t r = random_pop_histogram(&engine->wall);
			add_tile_hand(&engine->players[p].hand, r);
		}
	}

	// To initialize the waits
	for (int p = 0; p < NB_PLAYERS; ++p) {
		tenpailist(&engine->players[p].hand, &engine->grouplist);
	}

	// Display PHASE_INIT
	display_riichi(engine, 0);

	// Main loop
	for (int p = 0; engine->wall.nb_tiles > 14; p = (p + 1) % NB_PLAYERS) {
		struct player *player = &engine->players[p];

		// Draw Phase
		engine->phase = PHASE_DRAW;

		// Give one tile to the player
		histo_index_t r = random_pop_histogram(&engine->wall);
		add_tile_hand(&player->hand, r);

		// Calculate best discards (hints)
		tilestodiscard(&player->hand, &engine->grouplist);

		if (player->player_type == PLAYER_HUMAN)
			display_riichi(engine, p);

		// GetInput Phase
		engine->phase = PHASE_GETINPUT;
		histo_index_t discard;
		int win = player_turn(player, &engine->grouplist, &discard);

		if (win) {
			// Tsumo Phase (player p win)
			engine->phase = PHASE_TSUMO;
			display_riichi(engine, p);
			return p;
		}

		// Claim Phase (for all other players)
		if (is_valid_index(discard)) {
			engine->phase = PHASE_CLAIM;
			for (int p2 = 0; p2 < NB_PLAYERS; ++p2) {
				if (p == p2)
					continue;

				struct player *other_player = &engine->players[p2];

				if (get_histobit(&other_player->hand.wintiles, discard)) {
					// Claim the tile
					add_tile_hand(&other_player->hand, discard);

					ASSERT_BACKTRACE(
					    is_valid_hand(&other_player->hand, &engine->grouplist));

					// Player p2 win
					engine->phase = PHASE_TSUMO;
					makegroups(&other_player->hand, &engine->grouplist);
					display_riichi(engine, p2);
					return p2;
				}
			}
		}

		// Wait Phase
		engine->phase = PHASE_WAIT;

		// Calculate winning tiles
		tenpailist(&player->hand, &engine->grouplist);

		if (player->player_type == PLAYER_HUMAN)
			display_riichi(engine, p);
	}

	return -1;
}
