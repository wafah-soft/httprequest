QT -= gui
QT += network testlib concurrent

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11 c++14


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS qtnetwork

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR         += D:\TFM\TFMLib\thttprequest
SRCMOC          = D:\TFM\TFMLib\thttprequest\httprequest\srcmoc
MOC_DIR         = D:\TFM\TFMLib\thttprequest\httprequest\moc
RCC_DIR         = D:\TFM\TFMLib\thttprequest\httprequest\rcc
UI_DIR          = D:\TFM\TFMLib\thttprequest\httprequest\ui
OBJECTS_DIR     = D:\TFM\TFMLib\thttprequest\httprequest\obj

SOURCES += \
    curlcalllibrary.cpp \
    curlrequest.cpp \
    filesizegetter.cpp \
    httprequest.cpp \
    qtrequest.cpp


HEADERS += \
    curlcalllibrary.h \
    curlrequest.h \
    filesizegetter.h \
    httpclientdef.h \
    httprequest.h \
    qtrequest.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

#win32: LIBS += -L$$PWD/network_lib/openssl/ -llibcrypto \
#                            -lssh   \
#                            -llibssl

#win32: LIBS += -LD:/TFM/TFMHttpReqeustLibrary -llibcurl


DEFINES += CURL_STATICLIB

LIBS += -lcurl -lcrypto -lssl -lWldap32 -lcrypt32

#win32: LIBS += -LD:/TFM/TFMHttpReqeustLibrary -lHttpRequest \
#                                                -llibssl \
#                                                -llibcrypto \
#                                                -llibeay32 \
#                                                -llibssl32
#                                                -llibcurl

#win32: LIBS += -LD:/TFM/TFMHttpReqeustLibrary -llibcurl
#win32: LIBS += -L$$PWD/network_lib/curl/ -llibcurl

INCLUDEPATH += $$PWD/network_lib/openssl
INCLUDEPATH += $$PWD/network_lib/curl
INCLUDEPATH += $$PWD/network_lib


include(qtpromise/qtpromise.pri)
