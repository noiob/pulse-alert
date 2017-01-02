#pragma once

#include <pebble.h>

#define SETTINGS_KEY 1

static void prv_default_settings();
static void prv_load_settings();
static void prv_save_settings();

// A structure containing our settings
typedef struct ClaySettings {
  int16_t Threshold;
  bool OverrideFreq;
  bool BackgroundWorker;
  uint16_t Frequency;
  time_t SnoozeUntil;
} __attribute__((__packed__)) ClaySettings;

struct HealthMinuteData;

void snooze(time_t until);
static void init();
static void deinit();
static void update_threshold_hr_layer();
static void edit_click_handler();
static void plus_click_handler();
static void minus_click_handler();
static void save_click_handler();
static void click_config_provider();
static void edit_click_config_provider();
static void get_historic_bpm();