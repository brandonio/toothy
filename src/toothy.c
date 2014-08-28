#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "resource_ids.auto.h"


#define UUID { 0x5E, 0x4C, 0x37, 0x2C, 0xC8, 0xEC, 0x45, 0x6E, 0x8B, 0xC2, 0x1E, 0xA1, 0xB0, 0xA7, 0x91, 0x8B }
#define MAJOR_VERSION 1
#define MINOR_VERSION 1
#define UNUSED(variable) (void)variable

PBL_APP_INFO(UUID, "Toothy", "hisa.me", MAJOR_VERSION, MINOR_VERSION, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_STANDARD_APP);

static AppContextRef app_ctx; // FIXME, better way??
static Window window;

static const uint32_t const thirty_pattern_segments[] = { 100, 50, 200, 200, 100, 50, 200 };
static VibePattern thirty_pattern = {
  .durations = thirty_pattern_segments,
  .num_segments = ARRAY_LENGTH(thirty_pattern_segments)
};

static const uint32_t const twomin_pattern_segments[] = { 400, 200, 400, 200, 1200 };
static VibePattern twomin_pattern = {
  .durations = twomin_pattern_segments,
  .num_segments = ARRAY_LENGTH(twomin_pattern_segments)
};

#define BRUSHING_STATE_NONE 0
#define BRUSHING_STATE_COUNTDOWN 1
#define BRUSHING_STATE_BRUSHING 2

static struct {
  uint32_t state;
  uint32_t data;
} brushing;

void select_single_click_handler(ClickRecognizerRef recogniser, Window *window) {
  UNUSED(recogniser);
  UNUSED(window);

  if(brushing.state == BRUSHING_STATE_NONE)
  {
    brushing.state = BRUSHING_STATE_COUNTDOWN;
    brushing.data = 3;
    app_timer_send_event(app_ctx, 1000, 0);
  }
}

void click_config_provider(ClickConfig **config, Window *window) {
  UNUSED(window);

  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler)select_single_click_handler;
}

void handle_init(AppContextRef ctx)
{
  app_ctx = ctx;

  brushing.state = BRUSHING_STATE_NONE;

  window_init(&window, "Toothy");
  window_set_fullscreen(&window, true);
  window_set_background_color(&window, GColorBlack);
  window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);

  window_stack_push(&window, true /* Animated */);
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie)
{
  UNUSED(ctx);
  UNUSED(handle);

  switch(brushing.state)
  {
    case BRUSHING_STATE_COUNTDOWN:
      if(--brushing.data == 0)
      {
        vibes_long_pulse();
        brushing.state = BRUSHING_STATE_BRUSHING;
        brushing.data = 4;
        app_timer_send_event(app_ctx, 30 * 1000, 0);
      }
      else
      {
        vibes_short_pulse();
        app_timer_send_event(app_ctx, 1000, 0);
      }
      break;

    case BRUSHING_STATE_BRUSHING:
      if(--brushing.data == 0)
      {
        vibes_enqueue_custom_pattern(twomin_pattern);
        brushing.state = BRUSHING_STATE_NONE;
      }
      else
      {
        vibes_enqueue_custom_pattern(thirty_pattern);
        app_timer_send_event(app_ctx, 30 * 1000, 0);
      }
      break;
  }
}

void pbl_main(void *params)
{
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .timer_handler = &handle_timer
  };

  app_event_loop(params, &handlers);
}
