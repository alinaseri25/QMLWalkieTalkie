package org.verya.QMLWalkieTalkie

import android.content.Intent
import android.util.Log
import android.os.Bundle
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import org.qtproject.qt.android.bindings.QtActivity
import android.content.Context
import android.media.AudioManager

class MainActivity : QtActivity() {

    companion object {
        private var audioManager: AudioManager? = null
        private var previousMode: Int = AudioManager.MODE_NORMAL

        @JvmStatic
        fun enableEchoCancellation(context: Context) {
            audioManager = context.getSystemService(Context.AUDIO_SERVICE) as AudioManager
            previousMode = audioManager?.mode ?: AudioManager.MODE_NORMAL
            audioManager?.mode = AudioManager.MODE_IN_COMMUNICATION
        }

        @JvmStatic
        fun disableEchoCancellation() {
            audioManager?.mode = previousMode
            audioManager = null
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        hideSystemUI()
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            // دوباره مخفی کن اگه کاربر از بالا سوایپ کرد
            hideSystemUI()
        }
    }

    private fun hideSystemUI() {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
            // Android 11+ (API 30+)
            window.setDecorFitsSystemWindows(false)
            window.insetsController?.let { controller ->
                controller.hide(WindowInsets.Type.systemBars())
                controller.systemBarsBehavior =
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            // Android 10 و پایین‌تر
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            )
        }

        // Keep screen on (اختیاری - صفحه خاموش نشه)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
    }
}
