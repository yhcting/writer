
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := writer-jni
LOCAL_SRC_FILES := \
	ahash.c \
	cstack.c \
	curve.c \
	d2d.c \
	d2d_jni.c \
	div.c \
	file.c \
	g2d.c \
	history.c \
	main.c \
	mempool.c \
	obj.c \
	ucmd.c \
	wsheet.c \
	wsheet_jni.c

LOCAL_LDLIBS := -lc -llog

LOCAL_CFLAGS := \
	-O2 \
	-Wall \
	-DCONFIG_ANDROID \
	-D_GNU_SOURCE

include $(BUILD_SHARED_LIBRARY)
