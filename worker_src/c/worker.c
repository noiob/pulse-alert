#include <pebble_worker.h>
#include "worker.h"

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

static void prv_on_health_data(HealthEventType type, void *context) {
  // If the update was from the Heart Rate Monitor, query it
  if (type == HealthEventHeartRateUpdate) {
    HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
    // Check the heart rate
    APP_LOG(APP_LOG_LEVEL_DEBUG, "current heart rate: %lu", (uint32_t) value);
    prv_load_settings();
    time_t now = time(NULL);
    if ((value > settings.Threshold) && (settings.SnoozeUntil - now >= 0)) {
      worker_launch_app();
    }
  }
}

static void prv_init() {
  prv_load_settings();
  // Initialize the worker here
  // Subscribe to health event handler
  health_service_events_subscribe(prv_on_health_data, NULL);
}

static void prv_deinit() {
  // Deinitialize the worker here
  health_service_events_unsubscribe();
}

int main(void) {
  prv_init();
  worker_event_loop();
  prv_deinit();
}