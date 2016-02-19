#pragma once

#include "UTouch.h"

namespace eez {
namespace psu {
namespace gui {
namespace touch {

extern void init();
extern void tick();

extern bool is_down;
extern int x;
extern int y;

}
}
}
} // namespace eez::psu::ui::touch
