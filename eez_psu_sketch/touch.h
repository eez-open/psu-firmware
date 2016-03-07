#pragma once

#include "UTouch.h"

namespace eez {
namespace psu {
namespace gui {
namespace touch {

extern void init();
extern void tick(unsigned long tick_usec);

enum EventType {
    TOUCH_NONE,
    TOUCH_TEST,
    TOUCH_DOWN,
    TOUCH_MOVE,
    TOUCH_UP
};

extern EventType event_type;
extern int x;
extern int y;

}
}
}
} // namespace eez::psu::ui::touch
