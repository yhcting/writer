
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := writer-jni
LOCAL_SRC_FILES := \
	d2d.c \
	g2d.c \
	wsheet.c \
	mempool.c \
	main.c

LOCAL_LDLIBS := -lc -llog

include $(BUILD_SHARED_LIBRARY)
