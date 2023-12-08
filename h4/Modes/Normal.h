#include "Modes.h"

class Normal : public Mode {
    public:

    virtual Modes GetMode() const override {
        return Modes::NORMAL;
    }
}