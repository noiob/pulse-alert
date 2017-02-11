var Clay = require('pebble-clay');
var messageKeys = require('message_keys');
var clayConfig = require('./config');
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

// Get AppMessage events
Pebble.addEventListener('appmessage', function(e) {
	console.log('Received the appMessage from watch!');
	
	// Get the clay settings object from localStorage
	var settings = JSON.parse(localStorage.getItem('clay-settings'));
	
  // Get the dictionary from the message
  var dict = e.payload;
	
	// Set JSON values to match those in the dictionary
	for (var key in dict) {
		if (key == 'SportsMode' || key == 'OverrideFreq') {
			var booleanValue = dict[key] == 1; // Convert integer to boolean
			settings[key] = booleanValue;
		} else {
			settings[key] = dict[key];
		}
	}
	
	// Convert JSON back to object, then save the settings object back into localStorage
	localStorage.setItem('clay-settings', JSON.stringify(settings));
});

// As soon as PebbleKit JS has been initialized, send request to get settings from watch
Pebble.addEventListener('ready', function() {
    var dict = {};
    dict[messageKeys.RequestSettings] = 'true';

    Pebble.sendAppMessage(dict, function() {
        console.log('Settings Request sent successfully: ' + JSON.stringify(dict));
    }, function(e) {
        console.log('Settings Request failed: ' + JSON.stringify(e));
    });
});

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
	if (e && !e.response) {
		return;
	}

	// Get the keys and values from each config item
	var dict = clay.getSettings(e.response);
	dict[messageKeys.VibeTypeBelow] = parseInt(dict[messageKeys.VibeTypeBelow]);
	dict[messageKeys.VibeTypeAbove] = parseInt(dict[messageKeys.VibeTypeAbove]);
	dict[messageKeys.Threshold] = parseInt(dict[messageKeys.Threshold]);
	dict[messageKeys.Backoff] = parseInt(dict[messageKeys.Backoff]);
	dict[messageKeys.SportsMode] = parseInt(dict[messageKeys.SportsMode]);
	dict[messageKeys.OverrideFreq] = parseInt(dict[messageKeys.OverrideFreq]);
	dict[messageKeys.Frequency] = parseInt(dict[messageKeys.Frequency]);
	
	// Send settings values to watch side
	Pebble.sendAppMessage(dict, function(e) {
		console.log('Sent config data to Pebble');
	}, function(e) {
		console.log('Failed to send config data!');
		console.log(JSON.stringify(e));
	});
});