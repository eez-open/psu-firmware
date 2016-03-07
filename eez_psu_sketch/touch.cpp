#include "psu.h"
#include "touch.h"

namespace eez {
namespace psu {
namespace gui {
namespace touch {

UTouch utouch(TOUCH_SCLK, TOUCH_CS, TOUCH_DIN, TOUCH_DOUT, TOUCH_IRQ);

EventType event_type = TOUCH_NONE;
int x;
int y;

#define BUF_SIZE 5
#define TOLERANCE 5

static int buf_x[BUF_SIZE];
static int buf_y[BUF_SIZE];
static int buf_i;

void init() {
	utouch.InitTouch(PORTRAIT);
	utouch.setPrecision(PREC_HI);
}

void tick(unsigned long tick_usec) {
	if (utouch.dataAvailable()) {
		utouch.read();

        x = utouch.getX();
        y = utouch.getY();

        if (x != -1 && y != -1) {
            if (event_type == TOUCH_NONE || event_type == TOUCH_UP) {
                buf_x[0] = touch::x;
                buf_y[0] = touch::y;
                buf_i = 1;
                event_type = TOUCH_TEST;
            } else if (event_type == TOUCH_TEST) {
                buf_x[buf_i] = touch::x;
                buf_y[buf_i] = touch::y;
                if (++buf_i == BUF_SIZE) {
                    int max_count = 0;
                    int found_start = -1;
                    int found_end = -1;
                    for (int i = 0; i < BUF_SIZE - 1 && max_count <= BUF_SIZE / 2; ++i) {
                        int k = i;
                        int count = 0;
                        for (int j = i + 1; j < BUF_SIZE; ++j) {
                            long dx = abs(buf_x[j] - buf_x[k]);
                            long dy = abs(buf_y[j] - buf_y[k]);
                            if (dx <= TOLERANCE && dy <= TOLERANCE) {
                                ++count;
                                k = j;
                            }
                        }
                        if (count > max_count) {
                            max_count = count;
                            found_start = i;
                            found_end = k;
                        }
                    }

                    if (max_count > BUF_SIZE / 2) {
                        x = buf_x[found_start];
                        y = buf_y[found_start];
            
                        event_type = TOUCH_DOWN;
                    } else {
                        for (int i = 1; i < BUF_SIZE; ++i) {
                            buf_x[i-1] = buf_x[i];
                            buf_y[i-1] = buf_y[i];
                        }
                        --buf_i;
                    }
                }
            } else if (event_type == TOUCH_DOWN) {
                event_type = TOUCH_MOVE;
            }
            return;
        }
    }

    if (event_type == TOUCH_DOWN || event_type == TOUCH_MOVE) {
        event_type = TOUCH_UP;
    } else if (event_type == TOUCH_UP || TOUCH_TEST) {
        event_type = TOUCH_NONE;
    }
}

}
}
}
} // namespace eez::psu::ui::touch
