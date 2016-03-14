/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "psu.h"
#include "touch_filter.h"

//
// Thanks to: http://dlbeer.co.nz/articles/tsf.html
//

#ifdef EEZ_PSU_ARDUINO
#define CONF_TOUCH_FILTER_N 7
#define CONF_TOUCH_FILTER_D 10
#ifdef EEZ_PSU_ARDUINO_DUE
#define CONF_TOUCH_FILTER_P 20
#else
#define CONF_TOUCH_FILTER_P 10
#endif
#else
#define CONF_TOUCH_FILTER_N 1
#define CONF_TOUCH_FILTER_D 2
#define CONF_TOUCH_FILTER_P 5
#endif


namespace eez {
namespace psu {
namespace gui {
namespace touch {

struct Point {
    int x, y;

    Point() { }
    Point(int _x, int _y) : x(_x), y(_y) { }
};

struct Sample {
    Point       l;
    bool        p;

    Sample() { }
    Sample(const Point& _l, bool _p) : l(_l), p(_p) { }
};

class MedianFilter {
public:
    MedianFilter() {
        for (int i = 0; i < 5; ++i)
            s[i] = 0;
    }

    int operator()(int x) {
        int t[5];

        s[0] = s[1]; t[0] = s[0];
        s[1] = s[2]; t[1] = s[1];
        s[2] = s[3]; t[2] = s[2];
        s[3] = s[4]; t[3] = s[3];
        s[4] = x;    t[4] = s[4];

        cmp_swap(t[0], t[1]);
        cmp_swap(t[2], t[3]);
        cmp_swap(t[0], t[2]);
        cmp_swap(t[1], t[4]);
        cmp_swap(t[0], t[1]);
        cmp_swap(t[2], t[3]);
        cmp_swap(t[1], t[2]);
        cmp_swap(t[3], t[4]);
        cmp_swap(t[2], t[3]);

        return t[2];
    }

private:
    static inline void cmp_swap(int& a, int& b) {
        if (a > b)
            util_swap(int, a, b);
    }

    int s[5];
};

template<int N, int D>
class IIRFilter {
public:
    IIRFilter() : s(0) { }

    int operator()(int x, bool reset = false) {
        if (reset) {
            s = x;
        } else {
            s = (N * s + (D - N) * x + D / 2) / D;
        }
        return (int)s;
    }

private:
    long s;
};

template<int N, int D>
class ChannelFilter {
public:
    int operator()(int x, bool reset = false) {
        return i(m(x), reset);
    }

private:
    MedianFilter m;
    IIRFilter<N, D> i;
};

template<int P>
class DebounceFilter {
public:
    DebounceFilter() : s(0) { }

    bool on() const {
        return s >= P;
    }

    bool operator()(bool x) {
        if (!x) {
            s = 0;
            return false;
        }

        if (s < P) {
            s++;
        }

        return on();
    }

private:
    int s;
};

template<int N, int D, int P>
class SampleFilter {
public:
    Sample operator()(const Sample& s) {
        //bool was_on = p.on();
        //bool on = p(s.p);
        //bool rst = !was_on && on;
        //return Sample(Point(x(s.l.x, rst), y(s.l.y, rst)), on);
        const bool rst = !p.on();

        return Sample(Point(x(s.l.x, rst), y(s.l.y, rst)), p(s.p));
    }

private:
    DebounceFilter<P> p;
    ChannelFilter<N, D> x;
    ChannelFilter<N, D> y;
};

typedef SampleFilter<CONF_TOUCH_FILTER_N, CONF_TOUCH_FILTER_D, CONF_TOUCH_FILTER_P> DefaultSampleFilter;

////////////////////////////////////////////////////////////////////////////////

class ScreenTransform {
public:
    ScreenTransform() { 
        reset(); 
    }

    bool calibrate(const Point& tl, const Point& br, const Point& tr, int m, const Point& d) {
        ScreenTransform next;

        next.adc_min = tl;
        next.adc_range = Point(br.x - tl.x, br.y - tl.y);
        next.dim = d;
        next.margin = m;
        next.swap_coords = false;

        Point t = next(tr);

        if ((t.x >= d.x - m * 2) && (t.y < m * 2)) {
            *this = next;
            return true;
        }

        next.swap_coords = true;
        t = next(tr);

        if ((t.x >= d.x - m * 2) && (t.y < m * 2)) {
            *this = next;
            return true;
        }

        return false;
    }

    void reset() {
        adc_min = Point(0, 0);
        adc_range = Point(0, 0);
        dim = Point(0, 0);
        margin = 0;
        swap_coords = false;
    }

    Point operator()(const Point& p) const {
        if (swap_coords) {
            return Point(ch_scale(adc_min.y, adc_range.y, dim.x, p.y),
                ch_scale(adc_min.x, adc_range.x, dim.y, p.x));
        }

        return Point(ch_scale(adc_min.x, adc_range.x, dim.x, p.x),
            ch_scale(adc_min.y, adc_range.y, dim.y, p.y));
    }

private:
    int ch_scale(long adc_min, long adc_range, long dim, long x) const {
        if (adc_range) {
            x = ((x - adc_min) * (dim - margin * 2) +
                        (adc_range / 2)) / adc_range + margin;

            if (x < 0)
                return 0;

            if (x >= dim)
                return (int)(dim - 1);
        }

        return (int)x;
    }

    Point       adc_min;
    Point       adc_range;
    Point       dim;
    int         margin;
    bool        swap_coords;
};

////////////////////////////////////////////////////////////////////////////////

static DefaultSampleFilter g_filter;
static ScreenTransform g_transform;

bool calibrate_transform(int tl_x, int tl_y, int br_x, int br_y, int tr_x, int tr_y, int margin) {
    DebugTraceF("Touch screen calibration points: %d, %d, %d, %d, %d, %d", tl_x, tl_y, br_x, br_y, tr_x, tr_y);
    return g_transform.calibrate(Point(tl_x, tl_y), Point(br_x, br_y), Point(tr_x, tr_y), margin, Point(240, 320));
}

void reset_transform_calibration() {
    g_transform.reset();
}

bool filter(bool is_pressed, int& x, int& y) {
    Sample s = g_filter(Sample(Point(x, y), is_pressed));
    x = s.l.x;
    y = s.l.y;
    return s.p;
}

void transform(int& x, int& y) {
    Point p = g_transform(Point(x, y));
    x = p.x;
    y = p.y;
}

}
}
}
} // namespace eez::psu::ui::touch
