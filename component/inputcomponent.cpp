#include "inputcomponent.h"
// component.h is included via inputcomponent.h
#include <fcntl.h>
#include <unistd.h>
#include <linux/input-event-codes.h> // For BTN_TRIGGER_HAPPY*
#include <cerrno>  // For errno
#include <cstring> // For strerror

namespace nucare {

InputComponent::InputComponent(QObject *parent, const QString& tag)
    : QObject(parent) // Initialize QObject base
    , Component(tag)  // Initialize Component base
{
    // Tag is set in Component's constructor
}

InputComponent::~InputComponent()
{
    if (notifier_) {
        notifier_->setEnabled(false);
        // QObject parent should handle deletion of notifier_ if it's a child
        // delete notifier_; // Let's rely on QObject parenting for now
    }
    if (fd_ != -1) {
        ::close(fd_);
    }
}

bool InputComponent::initialize()
{
    const char *device = "/dev/input/event0";
    fd_ = ::open(device, O_RDONLY | O_NONBLOCK);
    if (fd_ == -1) {
        logE() << "Cannot open input device" << device << ":" << strerror(errno);
        return false;
    }

    // Pass 'this' as parent to QSocketNotifier for automatic cleanup
    notifier_ = new QSocketNotifier(fd_, QSocketNotifier::Read, this);
    connect(notifier_, &QSocketNotifier::activated, this, &InputComponent::readInput);
    notifier_->setEnabled(true);
    logI() << "InputComponent initialized, listening on" << device;
    return true;
}

void InputComponent::readInput()
{
    struct input_event ev;
    ssize_t n;

    // notifier_ can be null if initialization failed or in destructor path
    if (!notifier_ || !notifier_->isEnabled()) return;


    while ((n = ::read(fd_, &ev, sizeof(ev))) > 0) {
        if (ev.type == EV_KEY) {
            // logD() << "Key event: type" << ev.type << "code" << ev.code << "value" << ev.value;
            if (ev.code == BTN_TRIGGER_HAPPY1) {
                if (ev.value == 1) { // Press
                    emit btnLeftPress();
                } else if (ev.value == 0) { // Release
                    emit btnLeftRelease();
                }
            } else if (ev.code == BTN_TRIGGER_HAPPY2) {
                if (ev.value == 1) { // Press
                    emit btnCenterPress();
                } else if (ev.value == 0) { // Release
                    emit btnCenterRelease();
                }
            } else if (ev.code == BTN_TRIGGER_HAPPY3) {
                if (ev.value == 1) { // Press
                    emit btnRightPress();
                } else if (ev.value == 0) { // Release
                    emit btnRightRelease();
                }
            }
        }
    }
    if (n == -1 && errno != EAGAIN) {
        logE() << "Error reading from input device:" << strerror(errno);
        // Consider disabling notifier or attempting to re-open
        if(notifier_) notifier_->setEnabled(false); // Disable notifier on critical error
    }
}

} // namespace nucare
