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

#define POLY1(x, a0) (a0)
#define POLY2(x, a1, a0) (a1 * (x) + a0)
#define POLY3(x, a2, a1, a0) (a2 * (x) * (x) + a1 * (x) + a0)
#define POLY4(x, a3, a2, a1, a0) (a3 * (x) * (x) * (x) + a2 * (x) * (x) + a1 * (x) + a0)
#define GET_POLY_MACRO(_1,_2,_3,_4,NAME,...) NAME
#define POLY(x, ...) GET_POLY_MACRO(__VA_ARGS__, POLY4, POLY3, POLY2, POLY1)(x, __VA_ARGS__)



namespace nucare {
constexpr int CHSIZE = 1024;
constexpr int HW_CHSIZE = 2048;


constexpr int BINSIZE = 850;

constexpr double CS137_PEAK1 = 32.0;
constexpr double CS137_PEAK2 = 661.66;
constexpr double K40_PEAK = 1461;
constexpr double Co60_PEAK = 1332;
constexpr double Co60_WND = 0.3;
}  // namespace nucare

namespace tag {

constexpr const char* WARNING_DLG = "Warning_Dlg";
constexpr const char* SUCCESS_DLG = "SuccessDlg";
constexpr const char* WINDOW_TAG  = "MainWindow";
constexpr const char* HOME_TAG    = "HomeScreen";
constexpr const char* SETTING_TAG = "SettingScreen";
constexpr const char* BACKGROUND_TAG = "BackgroundScreen";
constexpr const char* SPECTRUMVIEW_TAG = "SpectrumScreen";
constexpr const char* CALIBRATION_TAG = "CalibrationScreen";
constexpr const char* CHOICE_TAG  = "ChoiceDlg";
constexpr const char* ACQ_TIME_TAG = "AcqTimeDlg";
constexpr const char* EVENTS_TAG = "EventList";
constexpr const char* EVENT_DETAIL_TAG = "EventDetailScreen";
constexpr const char* SHUTDOWN_DLG = "Shutdown_dlg";
constexpr const char* SW_VERSION = "SwVersionDlg";
}  // namespace tag

namespace ui {
constexpr const int PADDING_1 = 4;
}

#endif // NDT_CONFIG_H
