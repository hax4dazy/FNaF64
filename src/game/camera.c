#include <stdlib.h>

#include "engine/object.h"
#include "engine/util.h"
#include "engine/sfx.h"
#include "engine/perspective.h"

#include "game/blip.h"
#include "game/buttons.h"
#include "game/game.h"
#include "game/freddy.h"
#include "game/bonnie.h"
#include "game/chica.h"
#include "game/foxy.h"
#include "game/golden_freddy.h"
#include "game/settings.h"
#include "game/camera.h"

#define FLIP_FRAMES 11
#define FLICKER_BIT 8

const int cam_button_pos[CAM_COUNT][2] = {
	{ 983, 353}, // 1A
	{ 963, 409}, // 1B
	{ 931, 487}, // 1C
	{ 983, 603}, // 2A
	{ 983, 643}, // 2B
	{ 899, 585}, // 3
	{1089, 604}, // 4A
	{1089, 644}, // 4B
	{ 857, 436}, // 5
	{1186, 568}, // 6
	{1195, 437}, // 7
};

static const char *fox_paths[4] = {
	"rom:/cam_1c_foxy0.ci8.sprite",
	"rom:/cam_1c_foxy1.ci8.sprite",
	"rom:/cam_1c_foxy2.ci8.sprite",
	"rom:/cam_1c_foxy3.ci8.sprite",
};

static const int which_cam_lut[CAM_COUNT][4] = {
	{    -1,     -1,     -1, CAM_1B}, // 1A
	{ CAM_5,  CAM_7, CAM_1A, CAM_1C}, // 1B
	{ CAM_5,  CAM_7, CAM_1B,  CAM_3}, // 1C
	{ CAM_3, CAM_4A, CAM_1C, CAM_2B}, // 2A
	{ CAM_3, CAM_4B, CAM_2A,     -1}, // 2B
	{    -1, CAM_2A, CAM_1C,     -1}, // 3
	{CAM_2A,  CAM_6,     -1, CAM_4B}, // 4A
	{CAM_2B,     -1, CAM_4A,     -1}, // 4B
	{    -1, CAM_1B,     -1, CAM_1C}, // 5
	{CAM_4A,     -1,  CAM_7,     -1}, // 6
	{CAM_1B,     -1,     -1,  CAM_6}, // 7
};

typedef struct {
	const char *path;
	int state;
} camera_state_t;

camera_state_t cam_1a_states[] = {
	{"rom:/cam_1a_all.ci8.sprite", FREDDY_BIT | BONNIE_BIT | CHICA_BIT},
	{"rom:/cam_1a_fred_chic.ci8.sprite", FREDDY_BIT | CHICA_BIT},
	{"rom:/cam_1a_bon_fred.ci8.sprite", FREDDY_BIT | BONNIE_BIT},
	{"rom:/cam_1a_fred.ci8.sprite", FREDDY_BIT},
	{"rom:/cam_1a_empty.ci8.sprite", 0},
};

camera_state_t cam_1b_states[] = {
	{"rom:/cam_1b_empty.ci8.sprite", 0},
	{"rom:/cam_1b_empty.ci8.sprite", FREDDY_BIT},
	{"rom:/cam_1b_chica0.ci8.sprite", FREDDY_BIT | CHICA_BIT},
	{"rom:/cam_1b_chica1.ci8.sprite", FREDDY_BIT | CHICA_BIT | ROOM_SPOT_BIT},
	{"rom:/cam_1b_bonnie0.ci8.sprite", FREDDY_BIT | BONNIE_BIT},
	{"rom:/cam_1b_bonnie1.ci8.sprite", FREDDY_BIT | BONNIE_BIT | ROOM_SPOT_BIT},
	{"rom:/cam_1b_chica0.ci8.sprite", FREDDY_BIT | CHICA_BIT | BONNIE_BIT},
	{"rom:/cam_1b_chica1.ci8.sprite", FREDDY_BIT | CHICA_BIT | BONNIE_BIT | ROOM_SPOT_BIT},
	{"rom:/cam_1b_bonnie0.ci8.sprite", BONNIE_BIT},
	{"rom:/cam_1b_bonnie1.ci8.sprite", BONNIE_BIT | ROOM_SPOT_BIT},
	{"rom:/cam_1b_chica0.ci8.sprite", CHICA_BIT},
	{"rom:/cam_1b_chica1.ci8.sprite", CHICA_BIT | ROOM_SPOT_BIT},
	{"rom:/cam_1b_chica0.ci8.sprite", CHICA_BIT | BONNIE_BIT},
	{"rom:/cam_1b_chica1.ci8.sprite", CHICA_BIT | BONNIE_BIT | ROOM_SPOT_BIT},
};

camera_state_t cam_2a_states[] = {
	{"rom:/cam_2a_empty.ci8.sprite", 0},
	{"rom:/cam_2a_bonnie.ci8.sprite", BONNIE_BIT},
	{"rom:/cam_2a_bonnie.ci8.sprite", BONNIE_BIT | ROOM_SPOT_BIT},
};

camera_state_t cam_2b_states[] = {
	{"rom:/cam_2b_empty.ci8.sprite", 0},
	{"rom:/cam_2b_bonnie0.ci8.sprite", BONNIE_BIT},
	{"rom:/cam_2b_bonnie0.ci8.sprite", BONNIE_BIT | ROOM_SPOT_BIT},
};

camera_state_t cam_3_states[] = {
	{"rom:/cam_3_empty.ci8.sprite", 0},
	{"rom:/cam_3_bonnie.ci8.sprite", BONNIE_BIT},
	{"rom:/cam_3_bonnie.ci8.sprite", BONNIE_BIT | ROOM_SPOT_BIT},
};

camera_state_t cam_4a_states[] = {
	{"rom:/cam_4a_empty.ci8.sprite", 0},
	{"rom:/cam_4a_chica0.ci8.sprite", CHICA_BIT},
	{"rom:/cam_4a_chica1.ci8.sprite", CHICA_BIT | ROOM_SPOT_BIT},
	{"rom:/cam_4a_chica0.ci8.sprite", FREDDY_BIT | CHICA_BIT},
	{"rom:/cam_4a_chica1.ci8.sprite", FREDDY_BIT | CHICA_BIT | ROOM_SPOT_BIT},
	{"rom:/cam_4a_freddy.ci8.sprite", FREDDY_BIT},
};

camera_state_t cam_4b_states[] = {
	{"rom:/cam_4b_empty.ci8.sprite", 0},
	{"rom:/cam_4b_chica0.ci8.sprite", CHICA_BIT},
	{"rom:/cam_4b_chica0.ci8.sprite", CHICA_BIT | ROOM_SPOT_BIT},
	{"rom:/cam_4b_chica0.ci8.sprite", FREDDY_BIT | CHICA_BIT},
	{"rom:/cam_4b_chica0.ci8.sprite", FREDDY_BIT | CHICA_BIT | ROOM_SPOT_BIT},
	{"rom:/cam_4b_freddy.ci8.sprite", FREDDY_BIT},
};

camera_state_t cam_5_states[] = {
	{"rom:/cam_5_empty.ci8.sprite", 0},
	{"rom:/cam_5_bonnie0.ci8.sprite", BONNIE_BIT},
	{"rom:/cam_5_bonnie1.ci8.sprite", BONNIE_BIT | ROOM_SPOT_BIT},
};

camera_state_t cam_7_states[] = {
	{"rom:/cam_7_empty.ci8.sprite", 0},
	{"rom:/cam_7_chica0.ci8.sprite", CHICA_BIT},
	{"rom:/cam_7_chica1.ci8.sprite", CHICA_BIT | ROOM_SPOT_BIT},
	{"rom:/cam_7_freddy.ci8.sprite", FREDDY_BIT},
	{"rom:/cam_7_freddy.ci8.sprite", FREDDY_BIT | CHICA_BIT},
	{"rom:/cam_7_freddy.ci8.sprite", FREDDY_BIT | CHICA_BIT | ROOM_SPOT_BIT},
};

int cam_state_counts[CAM_COUNT] = {
	 5, // 1A
	14, // 1B
	 0, // 1C
	 3, // 2A
	 3, // 2B
	 3, // 3
	 6, // 4A
	 6, // 4B
	 3, // 5
	 0, // 6
	 6, // 7
};

camera_state_t *cam_states[CAM_COUNT] = {
	cam_1a_states, cam_1b_states, NULL,
	cam_2a_states, cam_2b_states, cam_3_states,
	cam_4a_states, cam_4b_states, cam_5_states,
	NULL, cam_7_states,
};

int camera_states_last[CAM_COUNT] = {0};
int camera_states[CAM_COUNT] = {
	FREDDY_BIT | BONNIE_BIT | CHICA_BIT,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

float flip_timer = 0.0f;
bool camera_was_using = false;
bool camera_is_using = false;
bool camera_was_visible = false;
bool camera_is_visible = false;
float camera_glitch_timer = 0.0f;
float button_blink_timer = 0.0f;
bool button_blink = 1;
float view_turn = 0.0f;
float flicker_timer = 0.0f;
int flicker_val = 0;
int cam_selected = CAM_1A;

rspq_block_t *border_block;

object_t map;

object_t buttons[2];
const char *button_paths[2] = {
	"rom:/cam_button0.ci8.sprite",
	"rom:/cam_button1.ci8.sprite",
};

object_t name_atlas;
object_t missing_footage;

object_t flip_anim[FLIP_FRAMES];
const char *flip_anim_paths[FLIP_FRAMES] = {
	"rom:/camflip0.ci8.sprite",
	"rom:/camflip1.ci8.sprite",
	"rom:/camflip2.ci8.sprite",
	"rom:/camflip3.ci8.sprite",
	"rom:/camflip4.ci8.sprite",
	"rom:/camflip5.ci8.sprite",
	"rom:/camflip6.ci8.sprite",
	"rom:/camflip7.ci8.sprite",
	"rom:/camflip8.ci8.sprite",
	"rom:/camflip9.ci8.sprite",
	"rom:/camflip10.ci8.sprite",
};

object_t views[CAM_COUNT];

/* Shitty fix for lagging in cam 2A */
#define VIEWS_EXTRA 6
object_t views_extra[VIEWS_EXTRA];

static const char *camera_get_view_path(void)
{
	int bitmask = FREDDY_BIT | CHICA_BIT | BONNIE_BIT | ROOM_SPOT_BIT;
	for(int i = 0; i < cam_state_counts[cam_selected]; i++) {
		if((cam_states[cam_selected][i].state & bitmask) ==
				(camera_states[cam_selected] & bitmask)) {

			// debugf("%s\n", cam_states[cam_selected][i].path);
			return cam_states[cam_selected][i].path;
		}
	}

	char buf[64] = {0};
	buf[0] = '0';
	buf[1] = 'b';
	for(int i = 2; i < 2 + 16; i++) {
		int j = i - 2;
		buf[i] = ((camera_states[cam_selected] & (1 << j)) >> j) + 48;
	}

	assertf(0, "Invalid state: %s on cam %d\n", buf, cam_selected);
}

void camera_load(void)
{
	camera_states[0] = FREDDY_BIT | BONNIE_BIT | CHICA_BIT;
	for(int i = 1; i < CAM_COUNT; i++)
		camera_states[i] = 0;

	flip_timer = 0.0f;
	camera_was_using = false;
	camera_is_using = false;
	camera_was_visible = false;
	camera_is_visible = false;
	camera_glitch_timer = 0.0f;
	button_blink_timer = 0.0f;
	button_blink = 1;
	view_turn = 0.0f;
	flicker_timer = 0.0f;
	flicker_val = 0;
	cam_selected = CAM_1A;

	rspq_block_begin();
	rdpq_set_mode_fill(RGBA32(0xFF, 0xFF, 0xFF, 0xFF));
	rdpq_fill_rectangle(10, 5, 311, 6);    // top
	rdpq_fill_rectangle(10, 6, 11, 231);   // left
	rdpq_fill_rectangle(310, 6, 311, 231); // right
	rdpq_fill_rectangle(10, 230, 311, 231); // bottom
	border_block = rspq_block_end();

	object_load(&map, "rom:/map.ia4.sprite");
	objects_load(buttons, 2, button_paths);
	object_load(&name_atlas, "rom:/cam_name_atlas.ia4.sprite");
	object_load(&missing_footage, "rom:/missing_footage.i4.sprite");
	objects_load(flip_anim, FLIP_FRAMES, flip_anim_paths);

	wav64_play(&robotvoice_sfx, SFXC_ROBOTVOICE);
}

static void _camera_views_unload(bool exclude_current)
{
	int cam_last = camera_states_last[cam_selected];
	int cam_cur = camera_states[cam_selected];
	cam_last &= ~(FACE_GLITCH_MASK | FLICKER_BIT);
	cam_cur &= ~(FACE_GLITCH_MASK | FLICKER_BIT);

	for(int i = 0; i < CAM_COUNT; i++) {
		if(!exclude_current) {
			object_unload(views + i);
			continue;
		}

		if(i == cam_selected && cam_last == cam_cur)
			continue;

		object_unload(views + i);
		
	}
}

void camera_unload(void)
{
	objects_unload(flip_anim, FLIP_FRAMES);
	cam_selected = CAM_1A;
	flip_timer = 0.0f;
	camera_is_using = false;
	camera_is_visible = false;
	rspq_block_free(border_block);
	object_unload(&map);
	objects_unload(buttons, 2);
	object_unload(&name_atlas);
	object_unload(&missing_footage);

	_camera_views_unload(false);
}

void camera_flip_draw(void)
{
	int frame = (int)flip_timer;
	if(camera_is_visible || frame == 0)
		return;

	rdpq_set_mode_copy(true);
	object_draw(flip_anim[frame], 0, 0, 0, 0);
}

void camera_view_draw(void)
{
	if(camera_glitch_timer) {
		rdpq_set_mode_fill(RGBA32(0, 0, 0, 0xFF));
		rdpq_fill_rectangle(0, 0, 320, 240);
		return;
	}

	perspective_begin();
	rdpq_set_mode_copy(false);
	
	_camera_views_unload(true);
	switch(cam_selected) {
	case CAM_1C:
		object_load(views + cam_selected, fox_paths[foxy_progress]);
		object_draw(views[cam_selected], view_turn, 0, 0, 0);
		perspective_end();
		return;

	case CAM_2A:
		if(foxy_run_timer > 0 && foxy_run_timer < 35) {
			int frame = (foxy_run_timer / 35.0f) * FOXY_RUN_FRAMES;
			frame = clampf(frame, 0, FOXY_RUN_FRAMES - 1);
			object_draw(foxy_run[frame], view_turn, 0, 0, 0);
			perspective_end();
			return;
		}

		if(camera_states[CAM_2A] & FLICKER_BIT) {
			object_load(views + cam_selected,
					camera_get_view_path());
			object_draw(views[cam_selected],
					view_turn, 0, 0, 0);
			perspective_end();
			return;
		}

		object_load(views_extra, "rom:/cam_2a_dark.ci8.sprite");
		object_draw(views_extra[0], view_turn, 0, 0, 0);
		perspective_end();
		return;

	case CAM_2B:
		if((golden_freddy_progress != 1 && 
				golden_freddy_progress != 2) ||
				bonnie_cam == CAM_2B) {
			break;
		}

		object_load(views_extra + 1, "rom:/cam_2b_golden.ci8.sprite");
		object_draw(views_extra[1], view_turn, 0, 0, 0);
		perspective_end();
		return;

	case CAM_6:
		rdpq_set_mode_fill(RGBA32(0, 0, 0, 0xFF));
		rdpq_fill_rectangle(0, 0, 320, 240);
		perspective_end();

		rdpq_set_mode_standard();
		object_draw(missing_footage, 294, 90, 0, 0);
		return;

	default:
		break;
	}

	int bot_cams[2] = {bonnie_cam, chica_cam};
	int bot_req_cams[2] = {CAM_2B, CAM_4B};
	const char *side_paths[2][2] = {
		{"rom:/cam_2b_bonnie1.ci8.sprite",
			"rom:/cam_2b_bonnie2.ci8.sprite"},
		{"rom:/cam_4b_chica1.ci8.sprite",
			"rom:/cam_4b_chica2.ci8.sprite"}};

	/* Handle face glitching for Bonnie and Chica */
	int glitch_val = camera_states[cam_selected] >> 5;
	for(int i = 0; i < 2; i++) {
		if(cam_selected != bot_req_cams[i] ||
				bot_cams[i] != cam_selected)
			continue;

		if(glitch_val < 25) {
			object_load(views + cam_selected,
					camera_get_view_path());
			object_draw(views[cam_selected], view_turn, 0, 0, 0);
			perspective_end();
			return;
		}

		if(glitch_val >= 25 && glitch_val < 29) {
			int o = !i ? 1 : 3;
			object_load(views_extra + o, side_paths[i][0]);
			object_draw(views_extra[o], view_turn, 0, 0, 0);
			perspective_end();
			return;
		}

		if(glitch_val >= 29) {
			int o = !i ? 2 : 4;
			object_load(views_extra + o, side_paths[i][1]);
			object_draw(views_extra[o], view_turn, 0, 0, 0);
			perspective_end();
			return;
		}
	}

	objects_unload(views_extra, VIEWS_EXTRA);
	object_load(views + cam_selected, camera_get_view_path());
	object_draw(views[cam_selected], view_turn, 0, 0, 0);
	perspective_end();
}

void camera_ui_draw(void)
{
	rdpq_set_mode_fill(RGBA32(0xFF, 0xFF, 0xFF, 0xFF));
	rspq_block_run(border_block);
	rdpq_set_mode_standard();
	rdpq_mode_alphacompare(true);
	object_draw(map, 531, 313, 0, 0);

	for(int i = 0; i < CAM_COUNT; i++) {
		bool blink = button_blink && i == cam_selected;
		object_draw(buttons[blink], cam_button_pos[i][0] - 317,
				cam_button_pos[i][1], 29, 19);
		object_draw_index_x(name_atlas, cam_button_pos[i][0] - 317 - 24,
				cam_button_pos[i][1] - 12, 11, i);
	}

	if(camera_glitch_timer)
		object_draw(missing_footage, 294, 90, 0, 0);

}

static void camera_flip_update(double dt, struct controller_data down)
{
	int frame = (int)flip_timer;
	camera_was_using = camera_is_using;
	const bool button_down = (down.c->R || down.c->L || down.c->Z);
	const bool flip_anim_stopped = (frame == 0 || frame == FLIP_FRAMES);
	camera_is_using ^= button_down && flip_anim_stopped;

	flip_timer += (camera_is_using * 2 - 1) * dt * speed_fps(50);
	flip_timer = clampf(flip_timer, 0, FLIP_FRAMES);
	frame = (int)flip_timer;
}

static void camera_handle_sfx(void)
{
	if(camera_is_visible)
		mixer_ch_set_vol(SFXC_FAN, 0.1f, 0.1f);
	else
		mixer_ch_set_vol(SFXC_FAN, 0.25f, 0.25f);

	if(!camera_was_visible && camera_is_visible) {
		blip_trigger(true);
		button_state &= ~(BUTTON_LEFT_LIGHT | BUTTON_RIGHT_LIGHT);
	}
	
	if(camera_is_using == camera_was_using)
		return;

	if(camera_is_using) {
		wav64_play(&cam_up_sfx, SFXC_BLIP);
		wav64_play(&cam_scan_sfx, SFXC_CAMERA);
		return;
	}

	wav64_play(&cam_down_sfx, SFXC_BLIP);
	mixer_ch_stop(SFXC_CAMERA);

	return;
}

static void camera_update_turn_manual(double dt, struct controller_data held)
{
	if(!camera_is_visible)
		return;

	view_turn -= held.c->x * dt * 6;
	view_turn = clampf(view_turn, -640, 0);
}

static void camera_update_turn(double dt, struct controller_data held)
{
	if(settings_flags & SET_MANUAL_CAM_TURN_BIT) {
		camera_update_turn_manual(dt, held);
		return;
	}

	static float view_turn_timer = 0.0f;
	static float view_stop_timer = 0.0f;
	static int view_turn_state = 0;
	view_turn = lerpf(0, -640, view_turn_timer);
	switch(view_turn_state) {
	case 0:
		if(view_turn_timer < 1.0f)
			view_turn_timer += dt / 5.0f;
		else {
			view_turn_timer = 1.0f;
			view_turn_state = 1;
			view_stop_timer = 2;
		}
		return;

	case 1:
		if(view_stop_timer > 0)
			view_stop_timer -= dt;
		else {
			view_stop_timer = 0;
			view_turn_state = 2;
		}
		return;

	case 2:
		if(view_turn_timer > 0.0f)
			view_turn_timer -= dt / 5.0f;
		else {
			view_turn_timer = 0.0f;
			view_turn_state = 3;
			view_stop_timer = 2;
		}
		return;

	case 3:
		if(view_stop_timer > 0)
			view_stop_timer -= dt;
		else {
			view_stop_timer = 0;
			view_turn_state = 0;
		}
		return;

	}
}

static void camera_update_glitch_timer(double dt)
{
	camera_glitch_timer -= dt * 60;
	camera_glitch_timer = clampf(camera_glitch_timer, 0, 300);

	static bool has_blipped = false;
	bool bonnie_glitch = (bonnie_cam == cam_selected ||
			bonnie_cam_last == cam_selected) &&
			bonnie_blackout_timer > 0;
	bool chica_glitch = (chica_cam == cam_selected ||
			chica_cam_last == cam_selected) &&
			chica_blackout_timer > 0;
	if(bonnie_glitch || chica_glitch) {
		if(camera_is_visible) {
			switch(rand() % 4) {
			case 0:
				wav64_play(&camglitch1, SFXC_CAMERA);
				break;

			case 1:
				wav64_play(&camglitch2, SFXC_CAMERA);
				break;

			case 2:
				wav64_play(&camglitch3, SFXC_CAMERA);
				break;

			case 3:
				wav64_play(&camglitch4, SFXC_CAMERA);
				break;
			}
			camera_glitch_timer = 300;

			if(!has_blipped) {
				blip_trigger(true);
				has_blipped = true;
			}
		}
		object_unload(views + cam_selected);
		return;
	}

	if(camera_glitch_timer > 0)
		return;

	has_blipped = false;
}

static void camera_update_flicker(double dt)
{
	flicker_timer += dt * 60;
	bool flick;
	flicker_timer = wrapf(flicker_timer, 1.0f, &flick);
	if(flick)
		flicker_val = 1 + (rand() % 10);

	camera_states_last[cam_selected] = camera_states[cam_selected];
	if(flicker_val <= 3 && cam_selected == CAM_2A)
		camera_states[cam_selected] |= FLICKER_BIT;
	else
		camera_states[cam_selected] &= ~FLICKER_BIT;
}

static void camera_update_button_blink(double dt)
{
	button_blink_timer += speed_fps(3) * dt;
	bool blink;
	button_blink_timer = wrapf(button_blink_timer, 1.0f, &blink);
	button_blink ^= blink;
}

static void camera_check_switching(struct controller_data down)
{
	if(cam_selected == CAM_2B &&
			(golden_freddy_progress == 1 ||
			 golden_freddy_progress == 2))
		return;

	int dirs[4] = {
		down.c->C_left || down.c->left,
		down.c->C_right || down.c->right,
		down.c->C_up || down.c->up,
		down.c->C_down || down.c->down,
	};

	for(int i = 0; i < 4; i++) {
		if(!dirs[i])
			continue;

		int new_cam = which_cam_lut[cam_selected][i];
		if(new_cam == -1)
			continue;

		blip_trigger(true);
		button_blink_timer = 0;
		button_blink = 1;
		cam_selected = new_cam;
	}
}

static void camera_update_robot_voice(double dt)
{
 	static float robot_voice_timer = 0.0f;
	bool robot_voice_tick;
	robot_voice_timer += dt;
	robot_voice_timer = wrapf(robot_voice_timer, 0.1f, &robot_voice_tick);
	
	if(!camera_is_visible) {
		mixer_ch_set_vol(SFXC_ROBOTVOICE, 0, 0);
		return;
	}

	if(!robot_voice_tick)
		return;

	bool bonnie_in_cam_and_looking =
		(cam_selected == bonnie_cam && bonnie_cam == CAM_2B);
	bool chica_in_cam_and_looking =
		(cam_selected == chica_cam && chica_cam == CAM_4B);

	if(!camera_is_visible || night_num < 4) {
		mixer_ch_set_vol(SFXC_ROBOTVOICE, 0, 0);
		return;
	}

	if(bonnie_in_cam_and_looking || chica_in_cam_and_looking) {
		float vol = (float)(1 + (rand() % 5) * 5) / 100.0f;
		mixer_ch_set_vol(SFXC_ROBOTVOICE, vol, vol);
		return;
	}

	mixer_ch_set_vol(SFXC_ROBOTVOICE, 0, 0);
}

static void camera_update_face_glitch(double dt)
{
 	static float face_glitch_timer = 0.0f;
 	face_glitch_timer += dt;
	bool face_glitch_tick;
	face_glitch_timer = wrapf(face_glitch_timer, 0.05f, &face_glitch_tick);
	if(!face_glitch_tick)
		return;

	camera_states[cam_selected] &= ~(FACE_GLITCH_MASK);

	if(night_num < 4)
		return;

	camera_states[cam_selected] |= ((rand() % 30) + 1) << FACE_GLITCH_SHIFT;
}

void camera_update(update_parms_t uparms)
{
	camera_flip_update(uparms.dt, uparms.down);
	camera_was_visible = camera_is_visible;
	camera_is_visible = ((int)flip_timer == FLIP_FRAMES);

	if(!camera_is_visible)
		_camera_views_unload(false);

	camera_handle_sfx();
	camera_update_glitch_timer(uparms.dt);
	camera_update_turn(uparms.dt, uparms.held);

	if(!camera_is_visible)
		return;

	camera_update_flicker(uparms.dt);
	camera_update_button_blink(uparms.dt);
	camera_check_switching(uparms.down);

	/* if night is greater than 4 */
	camera_update_face_glitch(uparms.dt);
	camera_update_robot_voice(uparms.dt);
}
