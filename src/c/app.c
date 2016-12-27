#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;

static void init(void) {
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
    if (value > 130) {
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
