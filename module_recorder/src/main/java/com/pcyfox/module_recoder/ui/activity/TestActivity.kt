package com.pcyfox.module_recoder.ui.activity

import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.blankj.utilcode.constant.PermissionConstants
import com.blankj.utilcode.util.PermissionUtils
import com.blankj.utilcode.util.TimeUtils
import com.blankj.utilcode.util.ToastUtils
import com.pcyfox.lib_ffmpeg.PlayState
import com.pcyfox.module_recoder.R
import kotlinx.android.synthetic.main.module_recorder_activity_test.*
import java.io.File

class TestActivity : AppCompatActivity() {
    private val TAG = "TestActivity"
    private val url = "rtsp://admin:taike@2020@192.168.28.12:554/h264/ch01/main/av_stream"
  //  private val url = "/storage/emulated/0/test/20210406_16_26_12/1/out.mp4"
    private val storeDir = Environment.getExternalStorageDirectory().absolutePath + "/test/"

    private var recordCount = 0
    private var startTime = ""
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.module_recorder_activity_test)
        PermissionUtils.permission(PermissionConstants.MICROPHONE, PermissionConstants.STORAGE)
                .request()
        initView()
        val store = File(storeDir)
        if (!store.exists()) {
            store.mkdirs()
        }
        startTime = TimeUtils.getNowString(TimeUtils.getSafeDateFormat("yyyyMMdd_HH_mm_ss"))
    }

    private fun initView() {
        rv_record.run {
            setOnStateChangeListener { state ->
                Log.d(TAG, "initView() called with: state = $state")
                when (state) {
                    PlayState.PREPARED -> {
                        play()
                    }
                    else -> {
                    }
                }
            }
        }

    }

    private fun getRootPath(): File {
        val rp = "$storeDir$startTime/$recordCount/"
        val rf = File(rp)
        if (!rf.exists()) {
            rf.mkdirs()
        }
        return rf
    }

    private fun getFile(fileName: String): File {
        val f = File(getRootPath().absolutePath + "/$fileName")
        if (f.exists()) {
            f.delete()
        } else {
            f.createNewFile()
        }
        return f
    }

    private fun getVideoFile(): File {
        return getFile("video.h264")
    }

    private fun getAudioFile(): File {
        return getFile("audio.aac")
    }

    private fun getOutFile(): File {
        return getFile("out.mp4")
    }


    fun onClick(v: View) {
        rv_record.run {
            when (v.id) {
                R.id.btn_play -> {
                    recordCount++
                    prepareRecorder(getVideoFile().absolutePath, getAudioFile().absolutePath)
                    setResource(url)
                }
                R.id.btn_stop -> {
                    stop()
                }
                R.id.btn_pause -> {
                    pause()
                }
                R.id.btn_resume -> {
                    resume()
                }

                R.id.btn_start_record -> {
                    startRecord()
                }
                R.id.btn_pause_record -> {
                    pauseRecord()
                }
                R.id.btn_resume_record -> {
                    resumeRecord()
                }
                R.id.btn_stop_record -> {
                    stopRecord()
                }
                R.id.btn_mux -> {
                    mux()
                }
                else -> {
                }
            }
        }


    }

    private fun mux() {
        Thread {
            Log.d(TAG, "mux() called start")
            val ret = rv_record.mux(getOutFile().absolutePath)
            if (ret) {
                Log.d(TAG, "mux() called over!")
                ToastUtils.showShort("MUX Over!")
            } else {
                ToastUtils.showShort("MUX Fail!")
                Log.e(TAG, "mux() called fail!")
            }
        }.start()
    }

}