package com.android.server;

import android.content.Context;
import android.os.IViregService;
import android.util.Slog;

public class ViregService extends IViregService.Stub {

        private static final String TAG = "ViregService";

        private int mPtr = 0;

        ViregService() {

                mPtr = init_native();

                if(mPtr == 0) {
                        Slog.e(TAG, "Failed to initialize vireg service.");
                }
        }

        public void setVal(int val) {
                if(mPtr == 0) {
                        Slog.e(TAG, "Vireg service is not initialized.");
                        return;
                }

                setVal_native(mPtr, val);
        }

        public int getVal() {
                if(mPtr == 0) {
                        Slog.e(TAG, "Vireg service is not initialized.");
                        return 0;
                }

                return getVal_native(mPtr);
        }

        private static native int init_native();
        private static native void setVal_native(int ptr, int val);
        private static native int getVal_native(int ptr);
};
