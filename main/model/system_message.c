
struct IRRemoteMessage {
    enum RemoteButton pressed_button;
} ir_remote_message;

struct DisplayMessage {
    enum DisplayAction requested_action;
    float temperature;
    float humidity;
} display_message;

struct SystemMessage {
    enum SystemAction system_action;
} system_message;
