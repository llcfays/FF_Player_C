//
// Created by LN on 2021/1/8.
//

#include "include/StateListener.h"
#include "android_log.h"


void StateListener::onStateChange(PlayState state) {
    LOGI("onStateChange=%d", state);
}

std::string StateListener::PlayerStateToString(int state) {
    switch (state) {
        case UNINITIALIZED:
            return "UNINITIALIZED";
        case INITIALIZED:
            return "INITIALIZED";
        case PREPARED:
            return "PREPARED";
        case STOPPED:
            return "STOPPED";
        case ERROR:
            return "ERROR";
        case STARTED:
            return "STARTED";
        case PAUSE:
            return "PAUSE";
        case RELEASE:
            return "RELEASE";
        case EXECUTING:
            return "EXECUTING";
    }

    return "UN KNOW";
}
