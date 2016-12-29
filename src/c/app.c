#include <pebble.h>
#include "app.h"

static Window *s_window;
static TextLayer *s_live_hr_layer, *s_threshold_hr_layer, *s_threshold_label_layer;
ActionBarLayer *s_action_bar;
static GBitmap *b_plus_bitmap, *b_minus_bitmap;
ClaySettings settings;

// Initialize the default settings
static void prv_default_settings() {
  settings.Threshold = 130;
  settings.OverrideFreq = false;
  settings.Frequency = 300;
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

static void update_threshold_hr_layer() {
  static char s_threshold_buffer[8];
  snprintf(s_threshold_buffer, sizeof(s_threshold_buffer), "%d BPM", settings.Threshold);
  text_layer_set_text(s_threshold_hr_layer, s_threshold_buffer);
  prv_save_settings();
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Threshold
  Tuple *threshold_t = dict_find(iter, MESSAGE_KEY_Threshold);
  if (threshold_t) {
    settings.Threshold = threshold_t->value->int32;
    update_threshold_hr_layer();
  }

  bool success = true;

  // Override Frequency
  Tuple *override_freq_t = dict_find(iter, MESSAGE_KEY_OverrideFreq);
  if (override_freq_t) {
    settings.OverrideFreq = override_freq_t->value->int32 == 1;
    if (!settings.OverrideFreq) {
      success = health_service_set_heart_rate_sample_period(0);
    }
    else {
      success = health_service_set_heart_rate_sample_period(settings.Frequency);
    }
  }

  // Frequency
  Tuple *frequency_t = dict_find(iter, MESSAGE_KEY_Frequency);
  if (frequency_t) {
    settings.Frequency = frequency_t->value->int32;
    if (settings.OverrideFreq) {
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
    HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
    // Check the heart rate
    APP_LOG(APP_LOG_LEVEL_DEBUG, "current heart rate: %lu", (uint32_t) value);

    static char s_hrm_buffer[8];
    snprintf(s_hrm_buffer, sizeof(s_hrm_buffer), "%lu BPM", (uint32_t) value);
    text_layer_set_text(s_live_hr_layer, s_hrm_buffer);
  }
}

static void plus_click_handler() {
  if (settings.Threshold < 190) {
    settings.Threshold += 10;
  }
  update_threshold_hr_layer();
}

static void minus_click_handler() {
  if (settings.Threshold > 40) {
    settings.Threshold -= 10;
  }
  update_threshold_hr_layer();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) plus_click_handler);
  
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) minus_click_handler);
}

static void init(void) {
  prv_load_settings();

  // Create a window and get information about the window
  s_window = window_create();
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the ActionBar
  s_action_bar = action_bar_layer_create();
  b_plus_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PLUS);
  b_minus_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MINUS);
  action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_UP, b_plus_bitmap, true);
  action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_DOWN, b_minus_bitmap, true);
  
  action_bar_layer_add_to_window(s_action_bar, s_window);
  
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  
  // Create the text layers
  const GEdgeInsets mid_label_insets = {.top = 122, .right = ACTION_BAR_WIDTH};
  s_live_hr_layer = text_layer_create(grect_inset(bounds, mid_label_insets));
  const GEdgeInsets bot_label_insets = {.bottom = 51, .top = 71, .left = 15, .right = ACTION_BAR_WIDTH + 5};
  s_threshold_hr_layer = text_layer_create(grect_inset(bounds, bot_label_insets));
  static char s_threshold_buffer[8];
  snprintf(s_threshold_buffer, sizeof(s_threshold_buffer), "%d BPM", settings.Threshold);
  text_layer_set_text(s_threshold_hr_layer, s_threshold_buffer);
  text_layer_set_background_color(s_threshold_hr_layer,GColorBlack);
  text_layer_set_text_color(s_threshold_hr_layer,GColorWhite);
  const GEdgeInsets mid_label_2_insets = {.bottom = 51, .top = 51, .left = 15, .right = ACTION_BAR_WIDTH + 5};
  s_threshold_label_layer = text_layer_create(grect_inset(bounds, mid_label_2_insets));
  text_layer_set_background_color(s_threshold_label_layer,GColorBlack);
  text_layer_set_text_color(s_threshold_label_layer,GColorWhite);
  text_layer_set_text(s_threshold_label_layer, "Alert at:");

  //custom vibe pattern to really catch the user's attention
  static const uint32_t segments[] = { 100, 100, 100, 100, 100, 100, 800 };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };

  // Launch the background worker
  AppWorkerResult result = app_worker_launch();
  if (result == APP_WORKER_RESULT_NO_WORKER) {
    text_layer_set_text(s_live_hr_layer, "Could not launch a worker.");
  }
  else {
    HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
    // Check the heart rate
    APP_LOG(APP_LOG_LEVEL_DEBUG, "current heart rate: %lu", (uint32_t) value);
    
    prv_load_settings();
    static char s_hrm_buffer[8];
    snprintf(s_hrm_buffer, sizeof(s_hrm_buffer), "%lu BPM", (uint32_t) value);
    text_layer_set_text(s_live_hr_layer, s_hrm_buffer);
    if (value > settings.Threshold) {
      vibes_enqueue_custom_pattern(pat);
    }

    // Subscribe to health event handler
    health_service_events_subscribe(prv_on_health_data, NULL);
  }

  // Set the font and text alignment
  text_layer_set_font(s_live_hr_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_live_hr_layer, GTextAlignmentCenter);
  text_layer_set_font(s_threshold_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_threshold_label_layer, GTextAlignmentCenter);
  text_layer_set_font(s_threshold_hr_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_threshold_hr_layer, GTextAlignmentCenter);
  

  // Add the text layer to the window
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_live_hr_layer));
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_threshold_label_layer));
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_threshold_hr_layer));

  // Enable text flow and paging on the text layer, with a slight inset of 10, for round screens
  text_layer_enable_screen_text_flow_and_paging(s_live_hr_layer, 10);
  text_layer_enable_screen_text_flow_and_paging(s_threshold_label_layer, 10);
  text_layer_enable_screen_text_flow_and_paging(s_threshold_hr_layer, 10);

  // Push the window, setting the window animation to 'true'
  window_stack_push(s_window, true);

  // App Logging!
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");

  // Listen for AppMessages
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);
}

static void deinit(void) {
  // Destroy the text layer
  text_layer_destroy(s_live_hr_layer);
  text_layer_destroy(s_threshold_label_layer);
  text_layer_destroy(s_threshold_hr_layer);
  
  // Destroy the ActionBar layer and the icons
  action_bar_layer_destroy(s_action_bar);
  gbitmap_destroy(b_plus_bitmap);
  gbitmap_destroy(b_minus_bitmap);

  // Destroy the window
  window_destroy(s_window);

  //unsubscribe from healt data
  health_service_events_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
