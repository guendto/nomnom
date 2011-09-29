
BUILT_SOURCES=\
 ui_Reminder.h \
 ui_MainWindow.h \
 ui_Preferences.h
 
noinst_HEADERS=\
 rc/Reminder.ui \
 rc/MainWindow.ui \
 rc/Preferences.ui \
 rc/img/nomnom.png \
 rc/img/README \
 rc/text/COPYING \
 rc/nomnom.qrc

nodist_nomnom_SOURCES=\
 moc_naboutdialog.cpp \
 moc_nfeedprogressdialog.cpp \
 moc_nfeeddialog.cpp \
 moc_Preferences.cpp \
 moc_MainWindow.cpp \
 moc_Reminder.cpp \
 moc_DownloadDiag.cpp \
 moc_ProcProgDiag.cpp \
 moc_Video.cpp \
 qrc_nomnom.cpp

CLEANFILES=	$(nodist_nomnom_SOURCES) $(BUILT_SOURCES)

ui_Preferences.h:
	$(UIC) -o $@ $(top_srcdir)/src/rc/Preferences.ui

ui_MainWindow.h:
	$(UIC) -o $@ $(top_srcdir)/src/rc/MainWindow.ui

ui_Reminder.h:
	$(UIC) -o $@ $(top_srcdir)/src/rc/Reminder.ui

moc_Preferences.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/i/Preferences.h

moc_MainWindow.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/i/MainWindow.h

moc_Reminder.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/i/Reminder.h

moc_DownloadDiag.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/DownloadDiag.h

moc_ProcProgDiag.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/ProcProgDiag.h

moc_Video.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/Video.h

moc_naboutdialog.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/about/naboutdialog.h

moc_nfeedprogressdialog.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/feed/nfeedprogressdialog.h

moc_nfeeddialog.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/feed/nfeeddialog.h

qrc_nomnom.cpp:
	$(RCC) -o $@ $(top_srcdir)/src/rc/nomnom.qrc

src=\
 about/naboutdialog.cpp \
 feed/nfeeddialog_info.cpp \
 feed/nfeeddialog_items.cpp \
 feed/nfeeddialog.cpp \
 feed/nfeedprogressdialog.cpp \
 feed/nfeed.cpp \
 i/MainWindow.cpp \
 i/Reminder.cpp \
 i/Preferences.cpp \
 ProcProgDiag.cpp \
 DownloadDiag.cpp \
 main.cpp \
 util.cpp \
 tips.cpp \
 Recent.cpp \
 Video.cpp

hdr=\
 NAboutDialog \
 about/naboutdialog.h \
 NFeedProgressDialog \
 NFeedDialog \
 NFeed \
 feed/nfeedprogressdialog.h \
 feed/nfeed.h \
 feed/nfeeddialog.h \
 i/MainWindow.h \
 i/Reminder.h \
 i/Preferences.h \
 ProcProgDiag.h \
 DownloadDiag.h \
 util.h \
 tips.h \
 Recent.h \
 Video.h

bin_PROGRAMS=	nomnom
nomnom_SOURCES=	$(src) $(hdr)

AM_CPPFLAGS=	-I$(top_srcdir)/src
AM_CPPFLAGS+=	-I$(top_srcdir)/src/i
AM_CPPFLAGS+=	-DDATADIR='"$(datadir)"'
AM_CPPFLAGS+=	$(QtScript_CFLAGS)
AM_CPPFLAGS+=	$(QtCore_CFLAGS)
AM_CPPFLAGS+=	$(QtGui_CFLAGS)

nomnom_LDADD=	$(QtScript_LIBS)
nomnom_LDADD+=	$(QtCore_LIBS)
nomnom_LDADD+=	$(QtGui_LIBS)
