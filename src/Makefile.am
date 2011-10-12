
BUILT_SOURCES=\
 ui_MainWindow.h \
 ui_Preferences.h
 
noinst_HEADERS=\
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
 moc_DownloadDiag.cpp \
 moc_ProcProgDiag.cpp \
 moc_Media.cpp \
 qrc_nomnom.cpp

CLEANFILES=	$(nodist_nomnom_SOURCES) $(BUILT_SOURCES)

ui_Preferences.h:
	$(UIC) -o $@ $(top_srcdir)/src/rc/Preferences.ui

ui_MainWindow.h:
	$(UIC) -o $@ $(top_srcdir)/src/rc/MainWindow.ui

moc_Preferences.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/i/Preferences.h

moc_MainWindow.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/i/MainWindow.h

moc_DownloadDiag.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/DownloadDiag.h

moc_ProcProgDiag.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/ProcProgDiag.h

moc_Media.cpp:
	$(MOC) -o $@ $(top_srcdir)/src/Media.h

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
 feed/nfeeddialog_properties.cpp \
 feed/nfeeddialog_items.cpp \
 feed/nfeeddialog.cpp \
 feed/nfeedprogressdialog.cpp \
 feed/nfeed.cpp \
 lang/nlang.cpp \
 i/MainWindow.cpp \
 i/Preferences.cpp \
 ProcProgDiag.cpp \
 DownloadDiag.cpp \
 main.cpp \
 util.cpp \
 Recent.cpp \
 Media.cpp

hdr=\
 NAboutDialog \
  about/naboutdialog.h \
 NFeedProgressDialog \
 NFeedDialog \
 NFeed \
  feed/nfeedprogressdialog.h \
  feed/nfeeddialog.h \
  feed/nfeed.h \
 NLang \
  lang/nlang.h \
 i/MainWindow.h \
 i/Preferences.h \
 ProcProgDiag.h \
 DownloadDiag.h \
 util.h \
 Recent.h \
 Media.h

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
