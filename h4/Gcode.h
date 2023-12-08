#pragma once

class Gcode {
    private:

        String gcodeCommand = "";
        long gcodeFeedDuPerSec = GCODE_FEED_DEFAULT_DU_SEC;
        bool gcodeInitialized = false;
        bool gcodeAbsolutePositioning = true;
        bool gcodeInBrace = false;
        bool gcodeInSemicolon = false;
}