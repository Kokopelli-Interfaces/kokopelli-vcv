#pragma once

#include "rack.hpp"


// backgroundColor = nvgRGB(0xee, 0xe8, 0xd5); // solarized base2
// backgroundColor = nvgRGB(0x93, 0xa1, 0xa1); // solarized base1
// backgroundColor = nvgRGB(0x78, 0x78, 0x78); // blendish default
// backgroundColor = nvgRGB(0xff, 0xff, 0xff); // custom
// backgroundColor = nvgRGB(0xd7, 0xda, 0xec); // blend

namespace colors {

// static const NVGcolor SOFT_SUNLIGHT = nvgRGB(0xd6, 0x91, 0x72); // v != 1
static const NVGcolor SOFT_SUNLIGHT = nvgRGB(0xff, 0xad, 0x88);

// static const NVGcolor SOFT_SUNLIGHT_BRIGHT = nvgRGB(0xff, 0x6a, 0x26); //hmm
static const NVGcolor SOFT_SUNLIGHT_BRIGHT = nvgRGB(0xff, 0xad, 0x88);

static const NVGcolor STRONG_SUNLIGHT = nvgRGB(0xff, 0x99, 0x55);
static const NVGcolor DARK_BROWN = nvgRGB(0x19, 0x0d, 0x05);
static const NVGcolor CYAN = nvgRGB(0x71, 0x99, 0x74);
static const NVGcolor CYAN_BRIGHT = nvgRGB(0x2c, 0xd7, 0x37);

static const NVGcolor YELLOWY = nvgRGB(0xe6, 0xa6, 0x0e);
// static const NVGcolor WOMB_RED = nvgRGB(0xbd, 0x37, 0x37); // v != 1
static const NVGcolor WOMB_RED = nvgRGB(0xfe, 0x4a, 0x4a);

// static const NVGcolor WOMB_RED_BRIGHT = nvgRGB(0xff, 0x2a, 0x2a); // pinky
static const NVGcolor WOMB_RED_BRIGHT = nvgRGB(0xff, 0x18, 0x18);



// default theme colors

static const NVGcolor ESTABLISHED = SOFT_SUNLIGHT;
static const NVGcolor SUN = STRONG_SUNLIGHT;
static const NVGcolor EMERGENCE = CYAN;
static const NVGcolor WOMB = WOMB_RED;
static const NVGcolor BOX_BG = DARK_BROWN;
static const NVGcolor LOOK_BACK_LAYER = YELLOWY;

static const NVGcolor ESTABLISHED_LIGHT = SOFT_SUNLIGHT_BRIGHT;
static const NVGcolor EMERGENCE_LIGHT = CYAN_BRIGHT;
static const NVGcolor WOMB_LIGHT = WOMB_RED_BRIGHT;

} // namespace colors