#include <pebble_worker.h>
#include "worker.h"

static void prv_on_health_data(HealthEventType type, void *context) {
  // If the update was from the Heart Rate Monitor, query it
  if (type == HealthEventHeartRateUpdate) {
    HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
    // Check the heart rate
    APP_LOG(APP_LOG_LEVEL_DEBUG, "current heart rate: %lu", (uint32_t) value);
    if (value > 130) {
      worker_launch_app();
    }
  }
}

static void prv_init() {
  // Initialize the worker here
  // Subscribe to health event handler
  health_service_events_subscribe(prv_on_health_data, NULL);
}

static void prv_deinit() {
  // Deinitialize the worker here
}

int main(void) {
  prv_init();
  worker_event_loop();
  prv_deinit();
}