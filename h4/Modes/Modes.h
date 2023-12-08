#pragma once

class Mode {
    public:

        enum class Type {
              NORMAL = 0,
              TURN,
              FACE,
              CUT,
              CONE,
              THREAD,
              ELIPSE
        };

        virtual Type GetType() const {
            return Type::NORMAL;
        }
};