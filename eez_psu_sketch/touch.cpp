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
int last_x;
int last_y;

#ifdef EEZ_PSU_SIMULATOR
#define ELIMINATE_NOISE 0
#define CONF_TOUCH_INERTIA 1
#else
#define ELIMINATE_NOISE 1
#define CONF_TOUCH_INERTIA 0.2
#endif

#if ELIMINATE_NOISE
#define TEST_BUF_SIZE 5
#define TEST_TOLERANCE 5

static int test_buf_x[TEST_BUF_SIZE];
static int test_buf_y[TEST_BUF_SIZE];
static int test_buf_i;
#endif

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
#if ELIMINATE_NOISE
                test_buf_x[0] = x;
                test_buf_y[0] = y;
                test_buf_i = 1;
                event_type = TOUCH_DOWN_TEST;
#else
                event_type = TOUCH_DOWN;
                last_x = x;
                last_y = y;
#endif
#if ELIMINATE_NOISE
            } else if (event_type == TOUCH_DOWN_TEST) {
                test_buf_x[test_buf_i] = x;
                test_buf_y[test_buf_i] = y;
                if (++test_buf_i == TEST_BUF_SIZE) {
                    int max_count = 0;
                    int found_start = -1;
                    int found_end = -1;
                    for (int i = 0; i < TEST_BUF_SIZE - 1 && max_count <= TEST_BUF_SIZE / 2; ++i) {
                        int k = i;
                        int count = 0;
                        for (int j = i + 1; j < TEST_BUF_SIZE; ++j) {
                            long dx = abs(test_buf_x[j] - test_buf_x[k]);
                            long dy = abs(test_buf_y[j] - test_buf_y[k]);
                            if (dx <= TEST_TOLERANCE && dy <= TEST_TOLERANCE) {
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

                    if (max_count > TEST_BUF_SIZE / 2) {
                        x = test_buf_x[found_start];
                        y = test_buf_y[found_start];
            
                        event_type = TOUCH_DOWN;

                        last_x = x;
                        last_y = y;
                    } else {
                        for (int i = 1; i < TEST_BUF_SIZE; ++i) {
                            test_buf_x[i-1] = test_buf_x[i];
                            test_buf_y[i-1] = test_buf_y[i];
                        }
                        --test_buf_i;
                    }
                }
#endif
            } else {
                if (event_type == TOUCH_DOWN) {
                    event_type = TOUCH_MOVE;
                }

                int dx = x - last_x;
                int dy = y - last_y;

                x = (int)round(last_x + CONF_TOUCH_INERTIA * dx);
                y = (int)round(last_y + CONF_TOUCH_INERTIA * dy);

                last_x = x;
                last_y = y;
            }
            return;
        }
    }

    if (event_type == TOUCH_DOWN || event_type == TOUCH_MOVE) {
        event_type = TOUCH_UP;
    } else if (event_type == TOUCH_UP || event_type == TOUCH_DOWN_TEST) {
        event_type = TOUCH_NONE;
    }
}

}
}
}
} // namespace eez::psu::ui::touch
