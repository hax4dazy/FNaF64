#include <stdlib.h>

#include "engine/object.h"
#include "engine/util.h"
#include "engine/sfx.h"

#include "game/game.h"
#include "game/buttons.h"
#include "game/camera.h"
#include "game/bonnie.h"
#include "game/chica.h"
#include "game/foxy.h"
#include "game/freddy.h"
#include "game/settings.h"
#include "game/texture_index.h"
#include "game/office.h"

#define ROOM_VIEWS 5

float office_turn;
static float office_turn_lerp;
float flicker_rand_timer;
int flicker_rand;

static object_t room_views[ROOM_VIEWS];
static const char *room_view_paths[ROOM_VIEWS] = {
	TX_ROOM_NORMAL,	     TX_ROOM_LEFT_EMPTY,  TX_ROOM_RIGHT_EMPTY,
	TX_ROOM_LEFT_BONNIE, TX_ROOM_RIGHT_CHICA,
};

object_t foxy_scare[FOXY_SCARE_FRAMES];
const char *foxy_scare_paths[FOXY_SCARE_FRAMES] = {
	TX_FOXY_SCARE0, TX_FOXY_SCARE1, TX_FOXY_SCARE2, TX_FOXY_SCARE3,
	TX_FOXY_SCARE4, TX_FOXY_SCARE5, TX_FOXY_SCARE6, TX_FOXY_SCARE7,
};

object_t freddy_scare[FREDDY_SCARE_FRAMES];
const char *freddy_scare_paths[FREDDY_SCARE_FRAMES] = {
	TX_FREDDY_SCARE_ROOM00, TX_FREDDY_SCARE_ROOM01, TX_FREDDY_SCARE_ROOM02,
	TX_FREDDY_SCARE_ROOM03, TX_FREDDY_SCARE_ROOM04, TX_FREDDY_SCARE_ROOM05,
	TX_FREDDY_SCARE_ROOM06, TX_FREDDY_SCARE_ROOM07, TX_FREDDY_SCARE_ROOM08,
	TX_FREDDY_SCARE_ROOM09, TX_FREDDY_SCARE_ROOM10, TX_FREDDY_SCARE_ROOM11,
	TX_FREDDY_SCARE_ROOM12, TX_FREDDY_SCARE_ROOM13,
};

static int room_get_state(void)
{
	if (flicker_rand <= 1)
		return 0;

	if (button_state & BUTTON_LEFT_LIGHT) {
		if (bonnie_cam == AT_DOOR)
			return 3;

		return 1;
	}

	if (button_state & BUTTON_RIGHT_LIGHT) {
		if (chica_cam == AT_DOOR)
			return 4;

		return 2;
	}

	return 0;
}

void office_load(void)
{
	office_turn = ROOM_TURN_MIN >> 1;
	office_turn_lerp = office_turn;
	flicker_rand_timer = 0.0f;
	flicker_rand = 0;

	objects_load(room_views, ROOM_VIEWS, room_view_paths);
	objects_load(foxy_scare, FOXY_SCARE_FRAMES, foxy_scare_paths);
	wav64_play(&light_sfx, SFXC_LIGHT);
}

void office_unload(void)
{
	objects_unload(room_views, ROOM_VIEWS);
	objects_unload(foxy_scare, FOXY_SCARE_FRAMES);
}

void office_draw(void)
{
	rdpq_set_mode_copy(false);

	if (foxy_is_scaring) {
		object_draw(foxy_scare[(int)foxy_scare_timer], office_turn, 0,
			    0, 0);
		return;
	}

	for (int i = 0; i < FREDDY_SCARE_FRAMES; i++) {
		if ((int)freddy_scare_timer == i)
			continue;

		object_unload(freddy_scare + i);
	}

	if (freddy_is_jumpscaring) {
		int frame = (int)freddy_scare_timer;
		object_load(freddy_scare + frame, freddy_scare_paths[frame]);
		object_draw(freddy_scare[(int)freddy_scare_timer], office_turn,
			    0, 0, 0);
		return;
	}

	object_draw(room_views[room_get_state()], office_turn, 0, 0, 0);
}

static void _office_update_turn_normal(update_parms_t uparms)
{
	const int stick_clamped = icutoff(uparms.sticks.stick_x, 10);
	const float turn_amount = stick_clamped * ROOM_TURN_SPEED;
	office_turn -= turn_amount * uparms.dt;
	office_turn = clampf(office_turn, ROOM_TURN_MIN, 0);
}

static void _office_update_turn_smooth(update_parms_t uparms)
{
	const int stick_clamped = icutoff(uparms.sticks.stick_x, 10);
	const float turn_amount = stick_clamped * ROOM_TURN_SPEED;
	office_turn_lerp -= turn_amount * uparms.dt;
	office_turn_lerp = clampf(office_turn_lerp, ROOM_TURN_MIN, 0);

	if (fabsf(office_turn_lerp - office_turn) < 0.001f) {
		office_turn = office_turn_lerp;
		return;
	}

	office_turn = lerpf(office_turn, office_turn_lerp, uparms.dt * 16);
}

void office_update(update_parms_t uparms)
{
	if (freddy_is_jumpscaring || foxy_is_scaring || bonnie_is_jumpscaring ||
	    chica_is_jumpscaring)
		camera_is_using = false;

	if (!camera_is_visible) {
		if (settings_flags & SET_SMOOTH_TURN_BIT)
			_office_update_turn_smooth(uparms);
		else
			_office_update_turn_normal(uparms);
	}

	if (button_state & (BUTTON_LEFT_LIGHT | BUTTON_RIGHT_LIGHT) &&
	    flicker_rand > 1)
		mixer_ch_set_vol(SFXC_LIGHT, 1, 1);
	else
		mixer_ch_set_vol(SFXC_LIGHT, 0, 0);

	static bool bonnie_scared_last = false;
	if ((button_state & BUTTON_LEFT_LIGHT) && bonnie_cam == AT_DOOR)
		bonnie_scared = true;
	if (bonnie_scared && !bonnie_scared_last) {
		mixer_ch_set_vol(SFXC_AMBIENCE, 0.8f, 0.8f);
		wav64_play(&window_scare, SFXC_AMBIENCE);
	}
	bonnie_scared_last = bonnie_scared;

	static bool chica_scared_last = false;
	if ((button_state & BUTTON_RIGHT_LIGHT) && chica_cam == AT_DOOR)
		chica_scared = true;
	if (chica_scared && !chica_scared_last)
		wav64_play(&window_scare, SFXC_AMBIENCE);
	chica_scared_last = chica_scared;

	bool do_rand;
	flicker_rand_timer =
		wrapf(flicker_rand_timer + uparms.dt * 60, 1.0 / 60, &do_rand);
	if (do_rand) {
		flicker_rand = rand() % 10;
	}

	if (camera_is_visible)
		return;

	if (fabsf(office_turn + 193) < 32 &&
	    (uparms.pressed.a || uparms.pressed.b))
		wav64_play(&boop_sfx, SFXC_BLIP);
}
