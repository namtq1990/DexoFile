#ifndef NDT_CONFIG_H
#define NDT_CONFIG_H

#define APP_NAME "NDT"

#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 1
#define APP_VERSION_PATCH 0

// Helper macros to stringify version components
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define APP_VERSION_STRING TOSTRING(APP_VERSION_MAJOR) "." \
                           TOSTRING(APP_VERSION_MINOR) "." \
                           TOSTRING(APP_VERSION_PATCH)

// You could also add build date/time if needed, e.g., using __DATE__ and __TIME__
// #define APP_BUILD_TIMESTAMP __DATE__ " " __TIME__

namespace tag {

constexpr const char* WARNING_DLG = "Warning_Dlg";
constexpr const char* SUCCESS_DLG = "SuccessDlg";
constexpr const char* WINDOW_TAG  = "MainWindow";
constexpr const char* HOME_TAG    = "HomeScreen";
constexpr const char* SETTING_TAG = "SettingScreen";
constexpr const char* CHOICE_TAG  = "ChoiceDlg";
}  // namespace tag

#endif // NDT_CONFIG_H
