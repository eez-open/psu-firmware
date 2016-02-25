#pragma once

namespace eez {
namespace psu {
namespace gui {
namespace data {

int count(uint16_t id);
void select(uint16_t id, int index);
char *get(uint16_t id, bool &changed);
void set(uint16_t id, const char *value);

}
}
}
} // namespace eez::psu::ui
