//
// Created by LN on 2021/2/24.
//


#ifdef __cplusplus
extern "C" {
#include "android_log.h"
#endif
#ifdef __cplusplus
}
#endif

#include "include/RecorderInfo.h"

RecorderInfo::~RecorderInfo() {
    LOGD("start to delete RecorderInfo ");
    if (o_fmt_ctx != NULL) {
        packetQueue.clearAVPacket();
        avformat_free_context(o_fmt_ctx);
        o_fmt_ctx = NULL;
    }
    free(storeFile);
    storeFile=NULL;
    LOGD("delete RecorderInfo over!");
}

void RecorderInfo::SetRecordState(RecordState state) {
    LOGI("SetRecordState state=%d", state);
    recordState = state;
    if (listener != NULL) {
        listener(recordState, id);
    }
}

RecordState RecorderInfo::GetRecordState() {
    return recordState;
}

void RecorderInfo::SetStateListener(void (*l)(RecordState, int)) {
    listener = l;
}

RecorderInfo::RecorderInfo() {
    //packetQueue.tag = "recorder";
}
