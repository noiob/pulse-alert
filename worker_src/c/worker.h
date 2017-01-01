#pragma once

#define SETTINGS_KEY 1

// A structure containing our settings
typedef struct ClaySettings {
  int16_t Threshold;
  bool OverrideFreq;
  bool BackgroundWorker;
  uint16_t Frequency;
  time_t SnoozeUntil;
} __attribute__((__packed__)) ClaySettings;

static void prv_default_settings();
static void prv_load_settings();