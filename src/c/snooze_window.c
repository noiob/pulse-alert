#include <pebble.h>
#include "snooze_window.h"
#include "app.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GBitmap *s_res_plus;
static GBitmap *s_res_accept;
static GBitmap *s_res_minus;
static GFont s_res_gothic_28_bold;
static GFont s_res_gothic_28;
static ActionBarLayer *s_actionbarlayer;
static TextLayer *s_hs;
static TextLayer *s_mins;
static TextLayer *s_text_h;
static TextLayer *s_text_min;
static StatusBarLayer *s_status_bar;
int i_h;
int i_min;

static void update_h_layer () {
  static char s_buffer[8];
  snprintf(s_buffer, sizeof(s_buffer), "%d", i_h);
  text_layer_set_text(s_hs, s_buffer);
}

static void update_min_layer () {
  static char s_buffer[8];
  snprintf(s_buffer, sizeof(s_buffer), "%d", i_min);
  text_layer_set_text(s_mins, s_buffer);
}

static void plus_h_click_handler () {
  if (i_h < 24) {
    i_h++;
  }
  update_h_layer();
}
  
static void minus_h_click_handler () {
  if (i_h > 0) {
    i_h--;
  }
  update_h_layer();
}
  
static void save_h_click_handler () {
  text_layer_set_background_color(s_hs, GColorWhite);
  text_layer_set_text_color(s_hs, GColorBlack);
  text_layer_set_background_color(s_mins, GColorBlack);
  text_layer_set_text_color(s_mins, GColorWhite);
  action_bar_layer_set_click_config_provider(s_actionbarlayer,snooze_window_click_config_provider_min);
}

static void plus_min_click_handler () {
  if (i_min < 59) {
    i_min++;
  }
  else {
    i_min = 0;
  }
  update_min_layer();
}
  
static void minus_min_click_handler () {
  if (i_min > 0) {
    i_min--;
  }
  else {
    i_min = 59;
  }
  update_min_layer();
}
  
static void save_min_click_handler () {
  time_t now = time(NULL);
  snooze(now + i_h * SECONDS_PER_HOUR + i_min * SECONDS_PER_MINUTE);
  hide_snooze_window();
}

static void snooze_window_click_config_provider_h(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) plus_h_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) save_h_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) minus_h_click_handler);
}

static void snooze_window_click_config_provider_min(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) plus_min_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) save_min_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) minus_min_click_handler);
}

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, true);
  #endif
  
  s_res_plus = gbitmap_create_with_resource(RESOURCE_ID_PLUS);
  s_res_accept = gbitmap_create_with_resource(RESOURCE_ID_ACCEPT);
  s_res_minus = gbitmap_create_with_resource(RESOURCE_ID_MINUS);
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  s_res_gothic_28 = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  // s_actionbarlayer
  s_actionbarlayer = action_bar_layer_create();
  action_bar_layer_add_to_window(s_actionbarlayer, s_window);
  action_bar_layer_set_background_color(s_actionbarlayer, GColorBlack);
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_UP, s_res_plus);
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_SELECT, s_res_accept);
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_DOWN, s_res_minus);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_actionbarlayer);
  
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_UP, s_res_plus);
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_SELECT, s_res_accept);
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_DOWN, s_res_minus);
  action_bar_layer_set_click_config_provider(s_actionbarlayer,snooze_window_click_config_provider_h);
  
  i_h = 0;
  i_min = 15;
  
  // s_hs
  s_hs = text_layer_create(GRect(10, 80, 22, 38));
  text_layer_set_background_color(s_hs, GColorBlack);
  text_layer_set_text_color(s_hs, GColorWhite);
  text_layer_set_text(s_hs, "0");
  text_layer_set_text_alignment(s_hs, GTextAlignmentRight);
  text_layer_set_font(s_hs, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_hs);
  
  // s_mins
  s_mins = text_layer_create(GRect(53, 80, 22, 38));
  text_layer_set_text(s_mins, "15");
  text_layer_set_text_alignment(s_mins, GTextAlignmentRight);
  text_layer_set_font(s_mins, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_mins);
  
  // s_text_h
  s_text_h = text_layer_create(GRect(33, 80, 13, 38));
  text_layer_set_text(s_text_h, "h");
  text_layer_set_font(s_text_h, s_res_gothic_28);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_text_h);
  
  // s_text_min
  s_text_min = text_layer_create(GRect(76, 80, 36, 38));
  text_layer_set_text(s_text_min, "min");
  text_layer_set_font(s_text_min, s_res_gothic_28);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_text_min);
  
  // s_status_bar
  s_status_bar = status_bar_layer_create();
  layer_set_frame(status_bar_layer_get_layer(s_status_bar), GRect(0, 0, layer_get_bounds(window_get_root_layer(s_window)).size.w - ACTION_BAR_WIDTH, STATUS_BAR_LAYER_HEIGHT)); 
  status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
  layer_add_child(window_get_root_layer(s_window), status_bar_layer_get_layer(s_status_bar));
}

static void destroy_ui(void) {
  window_destroy(s_window);
  action_bar_layer_destroy(s_actionbarlayer);
  text_layer_destroy(s_hs);
  text_layer_destroy(s_mins);
  text_layer_destroy(s_text_h);
  text_layer_destroy(s_text_min);
  gbitmap_destroy(s_res_plus);
  gbitmap_destroy(s_res_accept);
  gbitmap_destroy(s_res_minus);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_snooze_window(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_snooze_window(void) {
  window_stack_remove(s_window, true);
}
