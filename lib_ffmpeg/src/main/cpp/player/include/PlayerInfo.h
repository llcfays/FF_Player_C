//
// Created by Dwayne on 20/11/24.
//


#pragma once
#ifndef FF_PLAYER_PLAYER_INFO
#define FF_PLAYER_PLAYER_INFO

#include <media/NdkMediaCodec.h>
#include "pthread.h"
#include "StateListener.h"
#include "AsyncQueue.hpp"
#include <android/native_window.h>

#ifdef __cplusplus
extern "C" {
#include "android_log.h"
#include"libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#endif

#ifdef __cplusplus
}
#endif


class PlayerInfo {

public:
    int id{};
    AVBSFContext *bsf_ctx = NULL;
    AMediaCodec *videoCodec = NULL;
    AMediaCodec *audioCodec = NULL;
    AVFormatContext *inputContext = NULL;
    AVFormatContext *outContext = NULL;
    AVStream *inputVideoStream = NULL;
    AVStream *inputAudioStream = NULL;
    bool isOnlyRecordMedia = false;
    char *resource = NULL;
    int width{}, height{};

    AsyncQueue<AVPacket> packetQueue;

    pthread_t decode_thread = 0;
    pthread_t deMux_thread = 0;
    pthread_t open_resource_thread = 0;

    ANativeWindow *window = NULL;
    int windowWith{};
    int windowHeight{};

    pthread_mutex_t mutex{};
    pthread_cond_t cond{};

    int8_t lastNALUType = 0;

    bool isOpenAudio = false;
    const char *mine = "video/avc";
    volatile enum PlayState playState = UNINITIALIZED;

private:
    void (*stateListener)(PlayState, int) = NULL;


public:

    PlayerInfo();

    ~PlayerInfo();

    void SetPlayState(PlayState s, bool isNotify) volatile;

    PlayState GetPlayState();

    void SetStateListener(void (*stateListener)(PlayState, int));

};

#endif //AUDIO_PRACTICE_QUEUE_H
