#include <pebble.h>
#include "app.h"
#include "snooze_window.h"

static Window *s_window;
static GBitmap *s_res_zzz;
static GBitmap *s_res_gear;
static GBitmap *s_res_pencil;
static GBitmap *s_res_plus;
static GBitmap *s_res_minus;
static GBitmap *s_res_accept;
static GFont s_res_leco_42_numbers;
static GFont s_res_gothic_14;
static GBitmap *s_res_heart;
static GFont s_res_gothic_28;
static ActionBarLayer *s_actionbarlayer_1;
static TextLayer *s_hr_live_label;
static TextLayer *s_hr_live;
static TextLayer *s_alert_label;
static BitmapLayer *s_heart_icon;
static TextLayer *s_alert_threshold;
static StatusBarLayer *s_status_bar;
static Layer *s_graph;
static ActionMenu *s_action_menu;
static ActionMenuLevel *s_action_menu_level;
static ClaySettings settings;
static HealthMinuteData *data;
static int min, max;
uint32_t max_records;

// Initialize the default settings
static void prv_default_settings() {
  settings.Threshold = 130;
  settings.OverrideFreq = false;
  settings.BackgroundWorker = false;
  settings.Frequency = 300;
  settings.SnoozeUntil = 0;
  settings.Backoff = 1;
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

void snooze(time_t until) {
  prv_load_settings();
  settings.SnoozeUntil = until;
  prv_save_settings();
}

static void start_app_worker() {
  if (app_worker_launch() != APP_WORKER_RESULT_NO_WORKER) {
    settings.BackgroundWorker = true;
  }
}

static void stop_app_worker() {
  if (app_worker_kill() == APP_WORKER_RESULT_SUCCESS) {
    settings.BackgroundWorker = false;
  }
}

static void update_threshold_hr_layer() {
  static char s_threshold_buffer[8];
  snprintf(s_threshold_buffer, sizeof(s_threshold_buffer), "%d BPM", settings.Threshold);
  text_layer_set_text(s_alert_threshold, s_threshold_buffer);
  prv_save_settings();
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  prv_load_settings();
  
  // Threshold
  Tuple *threshold_t = dict_find(iter, MESSAGE_KEY_Threshold);
  if (threshold_t) {
    settings.Threshold = threshold_t->value->int32;
    update_threshold_hr_layer();
  }

  // Backoff
  Tuple *backoff_t = dict_find(iter, MESSAGE_KEY_Backoff);
  if (backoff_t) {
    settings.Backoff = backoff_t->value->int32;
  }

  bool success = true;

  // Override Frequency
  Tuple *override_freq_t = dict_find(iter, MESSAGE_KEY_OverrideFreq);
  if (override_freq_t) {
    settings.OverrideFreq = override_freq_t->value->int32 == 1;
    if (settings.OverrideFreq && app_worker_is_running()) {
      success = health_service_set_heart_rate_sample_period(settings.Frequency);
    }
    else {
      success = health_service_set_heart_rate_sample_period(0);
    }
  }

  // Frequency
  Tuple *frequency_t = dict_find(iter, MESSAGE_KEY_Frequency);
  if (frequency_t) {
    settings.Frequency = frequency_t->value->int32;
    if (settings.OverrideFreq && app_worker_is_running()) {
      success = health_service_set_heart_rate_sample_period(settings.Frequency);
    }
  }

  if (!success) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Could not set sampling period");
  }

  //don't forget to save!
  prv_save_settings();
}

static void prv_on_health_data(HealthEventType type, void *context) {
  // If the update was from the Heart Rate Monitor, query it
  if (type == HealthEventHeartRateUpdate) {
    get_historic_bpm();

    HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
    // Check the heart rate
    APP_LOG(APP_LOG_LEVEL_DEBUG, "current heart rate: %lu", (uint32_t) value);

    static char s_hrm_buffer[8] = "-";
    if (value != 0) {
      snprintf(s_hrm_buffer, sizeof(s_hrm_buffer), "%lu", (uint32_t) value);
    }
    text_layer_set_text(s_hr_live, s_hrm_buffer);
    if ((value > settings.Threshold) && (time(NULL) - settings.SnoozeUntil >= 0)) {
      snooze(time(NULL) + SECONDS_PER_MINUTE);
      
      //custom vibe pattern to really catch the user's attention
      static const uint32_t segments[] = { 100, 100, 100, 100, 100, 100, 800 };
      VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
      };
      vibes_enqueue_custom_pattern(pat);
    }
  }
}

static void snooze_click_handler() {
  show_snooze_window();
}

static void menu_click_handler() {
  s_action_menu_level = action_menu_level_create(1);
  if (app_worker_is_running()) {
    action_menu_level_add_action(s_action_menu_level, "Stop worker task", stop_app_worker, NULL);
  }
  else {
    action_menu_level_add_action(s_action_menu_level, "Start worker task", start_app_worker, NULL);
  }
  ActionMenuConfig config = (ActionMenuConfig) {
    .root_level = s_action_menu_level,
    .colors = {
      .background = GColorWhite,
      .foreground = GColorBlack,
    },
      .align = ActionMenuAlignCenter
  };
  
  s_action_menu = action_menu_open(&config);
}

static void edit_click_handler() {
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_UP, s_res_plus);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_SELECT, s_res_accept);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_DOWN, s_res_minus);
  action_bar_layer_set_click_config_provider(s_actionbarlayer_1,edit_click_config_provider);
}

static void plus_click_handler() {
  if (settings.Threshold <= 190) {
    settings.Threshold += 10;
  }
  update_threshold_hr_layer();
}

static void minus_click_handler() {
  if (settings.Threshold >= 60) {
    settings.Threshold -= 10;
  }
  update_threshold_hr_layer();
}

static void save_click_handler() {
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_UP, s_res_zzz);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_SELECT, s_res_gear);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_DOWN, s_res_pencil);
  action_bar_layer_set_click_config_provider(s_actionbarlayer_1,click_config_provider);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) snooze_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) menu_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) edit_click_handler);
}

static void edit_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) plus_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) save_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) minus_click_handler);
}

static int scale (int val, int height, int loc_min, double my_scale) {
  return (-1 * ((val - loc_min) * my_scale)) + height + 0.5;
}

static uint32_t get_available_records(HealthMinuteData *array, time_t query_start, time_t query_end, uint32_t max_records) {
  time_t next_start = query_start;
  time_t next_end = query_end;
  uint32_t num_records_found = 0;

  // Find more records until no more are returned
  while (num_records_found < max_records) {
    int ask_num_records = max_records - num_records_found;
    uint32_t ret_val = health_service_get_minute_history(&array[num_records_found], ask_num_records, &next_start, &next_end);
    if (ret_val == 0) {
      // a 0 return value means no more data is available
      return num_records_found;
    }
    num_records_found += ret_val;
    next_start = next_end;
    next_end = query_end;
  } 

  return num_records_found;
}

static void get_historic_bpm() {
  // Query for the last 15 minutes
  const time_t query_end = time(NULL);
  const time_t query_start = query_end - (15 * SECONDS_PER_MINUTE);
  max_records = (query_end - query_start) / SECONDS_PER_MINUTE;
  data = malloc(max_records * sizeof(HealthMinuteData));

  // Populate the array
  max_records = get_available_records(data, query_start, query_end, max_records);
  
  max = 0;
  min = 300;

  // Calculate highest and lowest value
  for(uint32_t i = 0; i < max_records; i++) {
    if(!(data[i].heart_rate_bpm == 0)) {
      if (data[i].heart_rate_bpm > max) {
        max = data[i].heart_rate_bpm;
      }
      if (data[i].heart_rate_bpm < min) {
        min = data[i].heart_rate_bpm;
      }
    }
  }
}

static void draw_graph(Layer *layer, GContext *ctx) {
  // Start drawing the graph
  GRect bounds = layer_get_bounds(s_graph);
  int step = bounds.size.w / max_records;
  int height = bounds.size.h - 1;
  GPoint last_point = GPoint(0,0);
  double my_scale = (double) (height) / (double) (max - min);

  if (data[0].heart_rate_bpm != 0) {
    last_point = GPoint(0,scale(data[0].heart_rate_bpm,height,min,my_scale));
  }
  // Print the results
  for(uint32_t i = 0; i < max_records; i++) {
    if(!(data[i].heart_rate_bpm == 0)) {
      if ((last_point.x == 0) && (last_point.y == 0)) {
        last_point = GPoint(0,scale(data[i].heart_rate_bpm,height,min,my_scale));
      }
      graphics_draw_line(ctx, last_point, GPoint(i*step, scale(data[i].heart_rate_bpm,height,min,my_scale)));
      last_point = GPoint(i*step, scale(data[i].heart_rate_bpm,height,min,my_scale));
    }
  }
  graphics_draw_line(ctx, last_point, GPoint(bounds.size.w, last_point.y));
}

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, false);
  #endif
  
  s_res_zzz = gbitmap_create_with_resource(RESOURCE_ID_ZZZ);
  s_res_gear = gbitmap_create_with_resource(RESOURCE_ID_GEAR);
  s_res_pencil = gbitmap_create_with_resource(RESOURCE_ID_PENCIL);
  s_res_plus = gbitmap_create_with_resource(RESOURCE_ID_PLUS);
  s_res_minus = gbitmap_create_with_resource(RESOURCE_ID_MINUS);
  s_res_accept = gbitmap_create_with_resource(RESOURCE_ID_ACCEPT);
  s_res_leco_42_numbers = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_res_heart = gbitmap_create_with_resource(RESOURCE_ID_HEART);
  s_res_gothic_28 = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  
  // s_hr_live_label
  s_hr_live_label = text_layer_create(GRect(85, 103, 28, 20));
  text_layer_set_background_color(s_hr_live_label, GColorClear);
  text_layer_set_text(s_hr_live_label, "BPM");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_hr_live_label);
  
  // s_hr_live
  s_hr_live = text_layer_create(GRect(0, 74, 85, 45));
  text_layer_set_background_color(s_hr_live, GColorClear);
  text_layer_set_text_alignment(s_hr_live, GTextAlignmentRight);
  text_layer_set_font(s_hr_live, s_res_leco_42_numbers);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_hr_live);
  
  // s_alert_label
  s_alert_label = text_layer_create(GRect(0, 123, 110, 18));
  text_layer_set_background_color(s_alert_label, GColorClear);
  text_layer_set_text(s_alert_label, "ALERT AT");
  text_layer_set_text_alignment(s_alert_label, GTextAlignmentCenter);
  text_layer_set_font(s_alert_label, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_alert_label);
  
  // s_heart_icon
  s_heart_icon = bitmap_layer_create(GRect(85, 81, 24, 25));
  bitmap_layer_set_bitmap(s_heart_icon, s_res_heart);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_heart_icon);
  
  // s_alert_threshold
  s_alert_threshold = text_layer_create(GRect(0, 133, 110, 28));
  text_layer_set_background_color(s_alert_threshold, GColorClear);
  static char s_threshold_buffer[8];
  snprintf(s_threshold_buffer, sizeof(s_threshold_buffer), "%d BPM", settings.Threshold);
  text_layer_set_text(s_alert_threshold, s_threshold_buffer);
  text_layer_set_text_alignment(s_alert_threshold, GTextAlignmentCenter);
  text_layer_set_font(s_alert_threshold, s_res_gothic_28);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_alert_threshold);
  
  // s_graph
  s_graph = layer_create(GRect(5, 5 + STATUS_BAR_LAYER_HEIGHT, 105, 55));
  layer_set_update_proc(s_graph,draw_graph);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_graph);
  
  // s_status_bar
  s_status_bar = status_bar_layer_create();
  layer_set_frame(status_bar_layer_get_layer(s_status_bar), GRect(0, 0, layer_get_bounds(window_get_root_layer(s_window)).size.w - ACTION_BAR_WIDTH, STATUS_BAR_LAYER_HEIGHT)); 
  status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
  layer_add_child(window_get_root_layer(s_window), status_bar_layer_get_layer(s_status_bar));
  
  // s_actionbarlayer_1
  s_actionbarlayer_1 = action_bar_layer_create();
  action_bar_layer_add_to_window(s_actionbarlayer_1, s_window);
  action_bar_layer_set_background_color(s_actionbarlayer_1, GColorBlack);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_UP, s_res_zzz);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_SELECT, s_res_gear);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_DOWN, s_res_pencil);
  action_bar_layer_set_click_config_provider(s_actionbarlayer_1,click_config_provider);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_actionbarlayer_1);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  action_bar_layer_destroy(s_actionbarlayer_1);
  text_layer_destroy(s_hr_live_label);
  text_layer_destroy(s_hr_live);
  text_layer_destroy(s_alert_label);
  bitmap_layer_destroy(s_heart_icon);
  text_layer_destroy(s_alert_threshold);
  layer_destroy(s_graph);
  gbitmap_destroy(s_res_zzz);
  gbitmap_destroy(s_res_gear);
  gbitmap_destroy(s_res_pencil);
  gbitmap_destroy(s_res_heart);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_main_window(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_main_window(void) {
  window_stack_remove(s_window, true);
}

static void init(void) {
  prv_load_settings();

  get_historic_bpm();

  show_main_window();
  
  //custom vibe pattern to really catch the user's attention
  static const uint32_t segments[] = { 100, 100, 100, 100, 100, 100, 800 };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };

  // Launch the background worker
  if (settings.BackgroundWorker) {
    start_app_worker();
  }
  
  HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
  // Check the heart rate
  APP_LOG(APP_LOG_LEVEL_DEBUG, "current heart rate: %lu", (uint32_t) value);

  prv_load_settings();
  static char s_hrm_buffer[8] = "-";
  if (value != 0) {
    snprintf(s_hrm_buffer, sizeof(s_hrm_buffer), "%lu", (uint32_t) value);
  }
  text_layer_set_text(s_hr_live, s_hrm_buffer);
  if ((value > settings.Threshold) && (time(NULL) - settings.SnoozeUntil >= 0)) {
    snooze(time(NULL) + settings.Backoff * 6);
    vibes_enqueue_custom_pattern(pat);
  }

  // Subscribe to health event handler
  health_service_events_subscribe(prv_on_health_data, NULL);

  // App Logging!
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");

  // Listen for AppMessages
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);

  //Set either 60s period or the overridden one, which one is shorter
  bool success = true;
  if (settings.OverrideFreq && app_worker_is_running()) {
    if (settings.Frequency < 60) {
      success = health_service_set_heart_rate_sample_period(settings.Frequency);
    }
    else {
      success = health_service_set_heart_rate_sample_period(60);
    }
  }
  else {
    success = health_service_set_heart_rate_sample_period(60);
  }
  if (!success) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Could not set sampling period");
  }
}

static void deinit(void) {
  bool success = true;
  if (settings.OverrideFreq && app_worker_is_running()) {
    success = health_service_set_heart_rate_sample_period(settings.Frequency);
  }
  else {
    success = health_service_set_heart_rate_sample_period(0);
  }
  if (!success) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Could not set sampling period");
  }

  hide_main_window();
  action_menu_hierarchy_destroy(s_action_menu_level, NULL, NULL);
  free(data);
  
  //unsubscribe from healt data
  health_service_events_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}