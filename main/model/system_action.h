#ifndef SYSTEM_ACTION_H
#define SYSTEM_ACTION_H

enum SystemAction {
    TOGGLE_ONBOARD_LED,
    DISPLAY_OFF,
    DISPLAY_ON,
};

enum DisplayAction { SCREEN_ON, SCREEN_OFF, SHOW_DHT_READING };

extern const char *system_action_str[];
extern const char *display_action_str[];

#endif
