#include <pebble.h>
#include "app.h"

static Window *s_window;
static TextLayer *s_text_layer;
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

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Threshold
  Tuple *threshold_t = dict_find(iter, MESSAGE_KEY_Threshold);
  if (threshold_t) {
    settings.Threshold = threshold_t->value->int32;
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

static void init(void) {
  prv_load_settings();
  
  // Create a window and get information about the window
  s_window = window_create();
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create a text layer
  s_text_layer = text_layer_create(bounds);
  
  //custom vibe pattern to really catch the user's attention
  static const uint32_t const segments[] = { 100, 100, 100, 100, 100, 100, 800 };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };
  
  // Launch the background worker
  AppWorkerResult result = app_worker_launch();
  if (result == APP_WORKER_RESULT_NO_WORKER) {
    text_layer_set_text(s_text_layer, "Could not launch a worker.");
  }
  else {
    HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
    // Check the heart rate
    APP_LOG(APP_LOG_LEVEL_DEBUG, "current heart rate: %lu", (uint32_t) value);
    
    static char s_hrm_buffer[8];
    snprintf(s_hrm_buffer, sizeof(s_hrm_buffer), "%lu BPM", (uint32_t) value);
    text_layer_set_text(s_text_layer, s_hrm_buffer);
    if (value > settings.Threshold) {
      vibes_enqueue_custom_pattern(pat);
    }
  }

  // Set the font and text alignment
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

  // Add the text layer to the window
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_text_layer));

  // Enable text flow and paging on the text layer, with a slight inset of 10, for round screens
  text_layer_enable_screen_text_flow_and_paging(s_text_layer, 10);

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
  text_layer_destroy(s_text_layer);

  // Destroy the window
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
