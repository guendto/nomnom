
BUILT_SOURCES=\
 ui_LogView.h \
 ui_Reminder.h \
 ui_MainWindow.h \
 ui_YoutubeFeed.h \
 ui_Preferences.h
 
noinst_HEADERS=\
 rc/LogView.ui \
 rc/Reminder.ui \
 rc/MainWindow.ui \
 rc/YoutubeFeed.ui \
 rc/Preferences.ui \
 rc/img/nomnom.png \
 rc/img/README \
 rc/text/COPYING \
 rc/nomnom.qrc

nodist_nomnom_SOURCES=\
 moc_naboutdialog.cpp \
 moc_Preferences.cpp \
 moc_YoutubeFeed.cpp \
 moc_MainWindow.cpp \
 moc_Reminder.cpp \
 moc_DownloadDiag.cpp \
 moc_ProcProgDiag.cpp \
 moc_LogView.cpp \
 moc_Video.cpp \
 qrc_nomnom.cpp

CLEANFILES=	$(nodist_nomnom_SOURCES) $(BUILT_SOURCES)

ui_Preferences.h:
	$(UIC) -o $@ $(top_srcdir)/src/rc/Preferences.ui

ui_YoutubeFeed.h:
	$(UIC) -o $@ $(top_srcdir)/src/rc/YoutubeFeed.ui

ui_MainWindow.h:
	$(UIC) -o $@ $(top_srcdir)/src/rc/MainWindow.ui

ui_Reminder.h:
	$(UIC) -o $@ $(top_srcdir)/src/rc/Reminder.ui

ui_LogView.h:
	$(UIC) -o $@ $(top_srcdir)/src/rc/LogView.ui

moc_Preferences.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/i/Preferences.h

moc_YoutubeFeed.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/i/YoutubeFeed.h

moc_MainWindow.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/i/MainWindow.h

moc_Reminder.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/i/Reminder.h

moc_LogView.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/i/LogView.h

moc_DownloadDiag.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/DownloadDiag.h

moc_ProcProgDiag.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/ProcProgDiag.h

moc_Video.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/Video.h

moc_naboutdialog.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/about/naboutdialog.h

qrc_nomnom.cpp:
	$(RCC) -o $@ $(top_srcdir)/src/rc/nomnom.qrc

src=\
 about/naboutdialog.cpp \
 i/MainWindow.cpp \
 i/LogView.cpp \
 i/Reminder.cpp \
 i/YoutubeFeed.cpp \
 i/Preferences.cpp \
 ProcProgDiag.cpp \
 DownloadDiag.cpp \
 Log.cpp \
 main.cpp \
 util.cpp \
 tips.cpp \
 Recent.cpp \
 Video.cpp

hdr=\
 NAboutDialog \
 about/naboutdialog.h \
 i/MainWindow.h \
 i/LogView.h \
 i/Reminder.h \
 i/YoutubeFeed.h \
 i/Preferences.h \
 ProcProgDiag.h \
 DownloadDiag.h \
 Log.h \
 util.h \
 tips.h \
 Recent.h \
 Video.h

bin_PROGRAMS=	nomnom
nomnom_SOURCES=	$(src) $(hdr)

AM_CPPFLAGS=	-I$(top_srcdir)/src -I$(top_srcdir)/src/i
AM_CPPFLAGS+=	-DDATADIR='"$(datadir)"'
AM_CPPFLAGS+=	$(QtScript_CFLAGS)
AM_CPPFLAGS+=	$(QtCore_CFLAGS)
AM_CPPFLAGS+=	$(QtGui_CFLAGS)

nomnom_LDADD=	$(QtScript_LIBS)
nomnom_LDADD+=	$(QtCore_LIBS)
nomnom_LDADD+=	$(QtGui_LIBS)
