#pragma once
#include "Modes.h"

class Cone : public Mode {
    public:

    private:
        
    float coneRatio = 1; // In cone mode, how much X moves for 1 step of Z
    float savedConeRatio = 0; // value of coneRatio saved in Preferences
    float nextConeRatio = 0; // coneRatio that should be applied asap
    bool nextConeRatioFlag = false; // whether nextConeRatio requires attention
};