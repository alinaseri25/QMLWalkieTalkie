package org.verya.QMLWalkieTalkie

import android.util.Log
import android.content.Context

object TestBridge {
    @JvmStatic
    external fun onMessageFromKotlin(msg: String)

    @JvmStatic
    external fun nativeOnNotificationAction(msg: String)

    @JvmStatic
    fun notifyCPlusPlus(msg: String) {
        //Log.d("123654", "123654notifyCPlusPlus: $msg")
        onMessageFromKotlin(msg)
    }

    @JvmStatic
    fun postNotification(ctx: Context, title: String, message: String,notifyId: Int) {
        val helper = NotificationHelper(ctx)
        helper.show(title, message, notifyId)
    }
}
