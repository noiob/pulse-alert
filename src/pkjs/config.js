module.exports = [
  {
    "type": "heading",
    "defaultValue": "Pulse Alert Configuration",
    "size": "1"
  },
  {
  "type": "section",
  "items": [
    {
    "type": "heading",
    "defaultValue":"Alert settings"
    },
    {
      "type": "slider",
      "messageKey": "Threshold",
      "defaultValue": 130,
      "label": "Alert at (bpm)",
      "description": "When your HR monitor readings exceed this value, your Pebble will vibrate and display your current heart rate.",
      "min": 50,
      "max": 200,
      "step": 1
    },
    {
      "type": "toggle",
      "messageKey": "OverrideFreq",
      "label": "Use custom frequency",
      "defaultValue": false
    },
    {
      "type": "slider",
      "messageKey": "Frequency",
      "defaultValue": 300,
      "label": "heart rate monitor frequency (every x seconds)",
      "description": "This will tell the Health servie how often you want your heart rate to be measured. The system will use this value as a suggestion, but does not guarantee that value will be used. The actual sampling period may be greater or less due to other apps that require input from the sensor, or data quality issues. Will definitely affect battery life, as default handling is 'measure every 15 minutes or more often when activity is detected'",
      "min": 1,
      "max": 600,
      "step": 1
    }
  ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  },
  {
  "type": "section",
  "items": [
      {
        "type": "text",
        "defaultValue": "If you'd like to buy me a coffee, here's a PayPal button, but you really don't have to :)"
      },
      {
        "type": "text",
        "defaultValue": "<form action='https://www.paypal.com/cgi-bin/webscr' method='post' target='_top'><input type='hidden' name='cmd' value='_s-xclick'><input type='hidden' name='hosted_button_id' value='Y7BXBVW4H6FAY'><input type='image' src='https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif' border='0' name='submit' alt='PayPal - The safer, easier way to pay online!'><img alt='' border='0' src='https://www.paypalobjects.com/de_DE/i/scr/pixel.gif' width='1' height='1'></form>"
      }
    ]
  },
  {
    "type": "heading",
    "defaultValue": "Please remember this disclaimer from Pebble"
  },
  {
    "type": "heading",
    "defaultValue": "Pebble devices with a heart rate (HR) monitor are intended to be a valuable tool that can provide an accurate estimation of a user’s heart rate. The HR monitor is designed to attempt to monitor a user’s heart rate on a periodic basis. The frequency at which heart rate is measured varies and depends on the level and activity of the user. While the HR monitor technology is state-of-the-art, there are inherent limitations of the technology that may cause some heart rate readings to be inaccurate under certain circumstances. These circumstances include the user’s physical characteristics, the fit of the device, and the type and intensity of the activity. The HR monitor data is not intended to be used for medical purposes, nor is it intended to diagnose, treat, cure or prevent any disease or condition.",
    "size": "6"
  }
];