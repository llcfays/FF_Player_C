//
// Created by LN on 2021/1/4.
//

#include <jni.h>
#include <android_log.h>
#include "FFPlayerBridge.h"
#include <android/native_window_jni.h>
#include <StateListener.h>
#include "PlayerResult.h"
#include "Player.h"
#include <map>
#include <utility>

#ifdef __cplusplus
extern "C" {
#include "Muxer.h"
#endif
#ifdef __cplusplus
}
#endif

#include <pthread.h>
#include <algorithm>

std::map<int, Player *> playerCache;

static jmethodID jMid_onPlayStateChangeId = NULL;
static jmethodID jMid_onRecordStateChangeId = NULL;
static jmethodID jMid_onMuxProgress = NULL;

static jclass clazz = NULL;

static JavaVM *vm = NULL;


Player *findPlayer(int id) {
    auto iterator = playerCache.find(id);
    if (iterator != playerCache.end()) {
        return iterator->second;
    } else {
        return NULL;
    }
}

bool removePlayer(int id) {
    auto i = playerCache.find(id);
    if (i != playerCache.end()) {
        playerCache.erase(i);
        return true;
    } else {
        return false;
    }
}


jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *env = NULL;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    vm = jvm;
    return JNI_VERSION_1_6;
}

void *ChangeRecordState(void *p) {
    int *params = (int *) p;
    int id = params[0];
    Player *player = findPlayer(id);
    if (player == NULL) {
        LOGE("onStateChange() not found player with id=%d", id);
        return nullptr;
    }
    JNIEnv *env = NULL;
    int ret = vm->AttachCurrentThread(&env, NULL);
    if (ret == 0 && env) {
        int state = params[1];
        env->CallVoidMethod(player->jPlayerObject, jMid_onRecordStateChangeId, state);
    } else {
        LOGE("onStateChange() get jEnv error");
    }
    vm->DetachCurrentThread();
    free(p);
    return nullptr;
}


void *ChangePlayState(void *p) {
    int *params = (int *) p;
    int id = params[0];
    Player *player = findPlayer(id);
    if (player == NULL) {
        LOGE("onStateChange() not found player with id=%d", id);
        return nullptr;
    }
    JNIEnv *env = NULL;
    int ret = vm->AttachCurrentThread(&env, NULL);
    if (ret == 0 && env) {
        int state = params[1];
        env->CallVoidMethod(player->jPlayerObject, jMid_onPlayStateChangeId, state);
    } else {
        LOGE("onStateChange() get jEnv error");
    }
    vm->DetachCurrentThread();
    free(p);
    return nullptr;
}


const void *onPlayStateChange(PlayState state, int id) {
    if (vm) {
        pthread_t thread = 0;
        int *params = (int *) malloc(sizeof(int));
        params[0] = id;
        params[1] = state;
        pthread_create(&thread, NULL, ChangePlayState, params);
        pthread_detach(pthread_self());
    }
    return NULL;
}


const void *onRecordStateChange(RecordState state, int id) {
    if (vm) {
        pthread_t thread = 0;
        int *params = (int *) malloc(sizeof(int));
        params[0] = id;
        params[1] = state;
        pthread_create(&thread, NULL, ChangeRecordState, params);
        pthread_detach(pthread_self());
    }
    return NULL;
}


void *onMuxProgress(float p) {
    JNIEnv *env = NULL;
    int ret = vm->AttachCurrentThread(&env, NULL);
    if (ret == 0 && env) {
        env->CallStaticVoidMethod(clazz, jMid_onMuxProgress, p);
    } else {
        LOGE("onStateChange() get jEnv error");
    }
    return nullptr;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_setResource(JNIEnv *env, jobject thiz, jstring url, int id) {
    Player *player = findPlayer(id);
    if (player == NULL) {
        LOGE("not found player by id=%d", id);
        return PLAYER_RESULT_ERROR;
    }
    char *inputUrl = (char *) env->GetStringUTFChars(url, 0);
    if (!inputUrl) {
        LOGE("fuck! input url is NULL");
        return PLAYER_RESULT_ERROR;
    }
    return player->SetResource(inputUrl);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_play(JNIEnv *env, jobject thiz, int id) {
    LOGI("play() called with:id=%d", id);
    Player *player = findPlayer(id);
    if (player == NULL) {
        LOGE("not found player by id=%d", id);
        return PLAYER_RESULT_ERROR;
    }
    return player->Play();
}



extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_start(JNIEnv *env, jobject thiz, int id) {
    Player *player = findPlayer(id);
    if (player == NULL) {
        return PLAYER_RESULT_ERROR;
    }
    return player->Start();
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_configPlayer(JNIEnv *env, jobject thiz,
                                                  jobject surface, int w, int h,
                                                  int isOnyRecorderMod, int id) {
    Player *player = findPlayer(id);
    if (player == NULL) {
        return PLAYER_RESULT_ERROR;
    }

    player->SetPlayStateChangeListener(reinterpret_cast<void (*)(PlayState, int) >
                                       (onPlayStateChange));

    player->SetRecordStateChangeListener(reinterpret_cast<void (*)(RecordState, int) >
                                         (onRecordStateChange));

    if (isOnyRecorderMod > 0) {
        return player->Configure(NULL, 0, 0, true);
    }
    ANativeWindow *native_window = NULL;
    native_window = ANativeWindow_fromSurface(env, surface);
    if (!native_window) {
        LOGE("create native window failQ");
        return PLAYER_RESULT_ERROR;
    }

    return player->Configure(native_window, w, h, false);
}



extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_onSurfaceChange(JNIEnv *env, jobject thiz, jobject surface,
                                                     jint w, jint h, int id) {

    Player *player = findPlayer(id);
    if (player == NULL) {
        return PLAYER_RESULT_ERROR;
    }
    ANativeWindow *native_window = NULL;
    native_window = ANativeWindow_fromSurface(env, surface);
    if (!native_window) {
        LOGE("create native window failQ");
        return PLAYER_RESULT_ERROR;
    }
    return player->OnWindowChange(native_window, w, h);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_init(JNIEnv *env, jobject thiz, int isDebug, int id) {
    LOGI("init() called with: isDebug=%d,id=%d", isDebug, id);
    if (findPlayer(id) == NULL) {
        auto *player = new Player(id);
        player->jPlayerObject = env->NewGlobalRef(thiz);
        Player::SetDebug(isDebug);
        playerCache.insert(std::map<int, Player *>::value_type(id, player));
        clazz = env->GetObjectClass(thiz);

        jMid_onPlayStateChangeId = env->GetMethodID(clazz, "onPlayerStateChange", "(I)V");
        jMid_onRecordStateChangeId = env->GetMethodID(clazz, "onRecorderStateChange", "(I)V");
        jMid_onMuxProgress = env->GetStaticMethodID(clazz, "onMuxProgress", "(F)V");
        return PLAYER_RESULT_OK;
    } else {
        return PLAYER_RESULT_ERROR;
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_stop(JNIEnv *env, jobject thiz, int id) {
    Player *player = findPlayer(id);
    if (player == NULL) {
        return PLAYER_RESULT_ERROR;
    }
    return player->Stop();
}




extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_pause(JNIEnv *env, jobject thiz, int id) {
    Player *player = findPlayer(id);
    if (player == NULL) {
        return PLAYER_RESULT_ERROR;
    }
    return player->Pause(0);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_setRecordState(JNIEnv *env, jobject thiz, jint state, int id) {
    Player *player = findPlayer(id);
    if (player == NULL) {
        return PLAYER_RESULT_ERROR;
    }
    switch (state) {
        case RECORD_START:
            return player->StartRecord();
        case RECORD_PAUSE:
            return player->PauseRecord();
        case RECORD_RESUME:
            return player->ResumeRecord();
        case RECORD_STOP:
            return player->StopRecord();
        default: {
            return -1;
        }
    }
}
/**
 * RUN ON WORK THREAD
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_resume(JNIEnv *env, jobject thiz, int id) {
    Player *player = findPlayer(id);
    if (player == NULL) {
        return PLAYER_RESULT_ERROR;
    }
    return player->Resume();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_prepareRecorder(JNIEnv *env, jobject thiz,
                                                     jstring out_file_path, int id) {
    Player *player = findPlayer(id);
    if (player == NULL) {
        return PLAYER_RESULT_ERROR;
    }
    char *outFile = (char *) env->GetStringUTFChars(out_file_path, 0);
    return player->PrepareRecorder(outFile);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_surfaceDestroyed(JNIEnv *env, jobject thiz, jobject holder,
                                                      int id) {
    Player *player = findPlayer(id);
    if (player == NULL) {
        return PLAYER_RESULT_ERROR;
    }
    return player->Pause(0);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_release(JNIEnv *env, jobject thiz, jint id) {
    Player *player = findPlayer(id);
    if (player == NULL) {
        return;
    }
    env->DeleteGlobalRef(player->jPlayerObject);
    removePlayer(id);
    player->Release();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_pcyfox_lib_1ffmpeg_FFPlayer_muxAV(JNIEnv *env, jclass clazz, jstring audio_file,
                                           jstring video_file, jstring out_file) {

    char *audioFile = (char *) env->GetStringUTFChars(audio_file, 0);
    char *videoFile = (char *) env->GetStringUTFChars(video_file, 0);
    char *outFile = (char *) env->GetStringUTFChars(out_file, 0);
    if (audioFile && videoFile && outFile) {
        return static_cast<jint>(MuxAVFile(audioFile, videoFile, outFile,
                                           reinterpret_cast<void (*)(float)>(onMuxProgress)));
    } else {
        return PLAYER_RESULT_ERROR;
    }
}

