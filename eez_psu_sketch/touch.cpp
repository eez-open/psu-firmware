#include "psu.h"
#include "touch.h"

namespace eez {
namespace psu {
namespace gui {
namespace touch {

// Initialize touchscreen
// ----------------------
// Set the pins to the correct ones for your development board
// -----------------------------------------------------------
// Standard Arduino Uno/2009 Shield            : 15,10,14, 9, 8
// Standard Arduino Mega/Due shield            :  6, 5, 4, 3, 2
// CTE TFT LCD/SD Shield for Arduino Due       :  6, 5, 4, 3, 2
// Teensy 3.x TFT Test Board                   : 26,31,27,28,29
// ElecHouse TFT LCD/SD Shield for Arduino Due : 25,26,27,29,30
//
UTouch utouch(TOUCH_SCLK, TOUCH_CS, TOUCH_DIN, TOUCH_DOUT, TOUCH_IRQ);

bool is_down = false;
int x;
int y;

void init() {
	utouch.InitTouch(PORTRAIT);
	utouch.setPrecision(PREC_MEDIUM);
}

void tick() {
	if (utouch.dataAvailable()) {
		utouch.read();

        x = utouch.getX();
        y = utouch.getY();
        if (x != -1 && y != -1) {
            is_down = true;
        }
    } else {
        is_down = false;
    }
}

}
}
}
} // namespace eez::psu::ui::touch
