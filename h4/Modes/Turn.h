#include "Modes.h"

class Turn : public Mode {
    public:

    virtual Modes GetMode() const override {
        return Modes::TURN;
    }

    private:
        int turnPasses = 3; // In turn mode, how many turn passes to make
        int savedTurnPasses = 0; // value of turnPasses saved in Preferences
        bool auxForward = true; // True for external, false for external thread //TODO rename to internal/external
};