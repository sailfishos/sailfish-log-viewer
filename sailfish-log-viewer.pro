TEMPLATE = subdirs
SUBDIRS = logger

logger.target = logger-common

ofono {
    ofono.target = logger-ofono
    ofono.depends = logger-common
    SUBDIRS += ofono
}

nfc {
    nfc.target = logger-nfc
    nfc.depends = logger-common
    SUBDIRS += nfc
}

OTHER_FILES += LICENSE rpm/*.spec
