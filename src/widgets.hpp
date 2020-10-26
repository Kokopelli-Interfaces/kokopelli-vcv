#pragma once

#include "rack.hpp"

struct Rogan4PSGray : Rogan {
  Rogan4PSGray() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/Rogan4PSGray.svg")));
  }
};

struct RoganHalfPSRed : Rogan {
  RoganHalfPSRed() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/RoganHalfPSRed.svg")));
  }
};

struct RoganHalfPSLightPurple : Rogan {
  RoganHalfPSLightPurple() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/RoganHalfPSLightPurple.svg")));
  }
};

struct RoganHalfPGray : Rogan {
  RoganHalfPGray() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/RoganHalfPGray.svg")));
  }
};

struct RoganHalfPRed : Rogan {
  RoganHalfPRed() {
    setSvg(APP->window->loadSvg(
        asset::system("./plugins/Myrisa/res/components/RoganHalfPRed.svg")));
  }
};
