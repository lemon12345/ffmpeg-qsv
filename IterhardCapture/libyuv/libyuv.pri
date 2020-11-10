
LIBYUV_ROOT_PATH = $$PWD
LIBJPEG_ROOT_PATH = $$PWD/../libjpeg_turbo
LIBYUV_INCLUDE_PATH = $$LIBYUV_ROOT_PATH/include
LIBYUV_LIB_PATH = $$LIBYUV_ROOT_PATH/lib


win32{
    LIBYUV_LIB_PATH = $$LIBYUV_LIB_PATH/win
}

INCLUDEPATH += $$LIBYUV_INCLUDE_PATH
INCLUDEPATH += $$LIBJPEG_ROOT_PATH
win32{
        LIBS += -L$$LIBYUV_LIB_PATH -llibyuv -llibjpeg
}
unset(LIBYUV_ROOT_PATH)
unset(LIBYUV_INCLUDE_PATH)
unset(LIBYUV_LIB_PATH)
