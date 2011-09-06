
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := writer-jni
LOCAL_SRC_FILES := \
	d2d.c \
	d2d_jni.c \
	div.c \
	g2d.c \
	wsheet.c \
	mempool.c \
	ahash.c \
	main.c

LOCAL_LDLIBS := -lc -llog

include $(BUILD_SHARED_LIBRARY)
