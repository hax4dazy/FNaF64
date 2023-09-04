#include "engine/object.h"
#include "engine/util.h"

#include "game/camera.h"
#include "game/buttons.h"
#include "game/game.h"
#include "game/settings.h"
#include "game/ui.h"

int power_usage;
int power_left;
float power_timer;

object_t am;
object_t hour_atlas;
object_t power_left_text;
object_t usage_text;
object_t usage_atlas;

static object_t night_text;
static object_t night_atlas;

void ui_load(void)
{
	power_usage = 1;
	power_left = 999;
	power_timer = 0.0f;

	object_load(&am, "rom:/am.ia4.sprite");
	object_load(&night_text, "rom:/night.ia4.sprite");
	object_load(&hour_atlas, "rom:/hour_atlas.ia4.sprite");
	object_load(&night_atlas, "rom:/night_atlas.ia4.sprite");
	object_load(&power_left_text, "rom:/power_left.ia4.sprite");
	object_load(&usage_text, "rom:/usage_text.ia4.sprite");
	object_load(&usage_atlas, "rom:/usage_atlas.ci8.sprite");
}

void ui_unload(void)
{
	object_unload(&am);
	object_unload(&night_text);
	object_unload(&hour_atlas);
	object_unload(&night_atlas);
	object_unload(&power_left_text);
	object_unload(&usage_text);
	object_unload(&usage_atlas);
}

void ui_draw(void)
{
	rdpq_set_mode_standard();
	rdpq_mode_alphacompare(true);
	object_draw(am, 879, 30, 0, 0);
	object_draw(night_text, 828, 71, 0, 0);
	int hour = (int)clampf(night_timer / HOUR_LEN_SECONDS, 0, 6);

	if(hour > 0)
		object_draw_index_y(hour_atlas, 843, 30, 9, hour);
	else {
		object_draw_index_y(hour_atlas, 843 - 26, 30, 9, 1);
		object_draw_index_y(hour_atlas, 843, 30, 9, 2);
	}

	object_draw_index_y(night_atlas, 903, 71, 6, night_num);
	object_draw(power_left_text, 106, 638, 68, 7);
	object_draw(usage_text, 74, 674, 36, 7);

	for(int i = 0; i < power_usage; i++) {
		int ind = clampf(i - 1, 0, 69);
		object_draw_index_x(usage_atlas, 120 + i * 21, 657, 6, ind);
	}

	object_draw_index_y(night_atlas, 200, 632, 6, (power_left % 100) / 10);
	
	if(power_left > 100)
		object_draw_index_y(night_atlas, 185, 632, 6, power_left / 100);
}

void ui_update(double dt)
{
	power_timer += dt;
	bool power_tick;
	power_timer = wrapf(power_timer, 1, &power_tick);
	if(power_tick)
		power_left -= power_usage;
	power_left = clampf(power_left, 0, 999);

	power_usage = 1;
	power_usage += camera_is_visible;
	power_usage += (button_state & BUTTON_LEFT_DOOR) > 0;
	power_usage += (button_state & BUTTON_RIGHT_DOOR) > 0;
	power_usage += (button_state & BUTTON_LEFT_LIGHT) > 0;
	power_usage += (button_state & BUTTON_RIGHT_LIGHT) > 0;
}
