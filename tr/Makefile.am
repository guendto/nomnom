qmfiles=	nomnom_fi.qm nomnom_de.qm
EXTRA_DIST=	nomnom_fi.ts nomnom_de.ts
CLEANFILES=	$(qmfiles)

nomnom_fi.qm:
	$(LRELEASE) -compress -qm $@ $(top_srcdir)/tr/nomnom_fi.ts

nomnom_de.qm:
	$(LRELEASE) -compress -qm $@ $(top_srcdir)/tr/nomnom_de.ts

qmdir=	$(datarootdir)/nomnom/tr
qm_DATA=$(qmfiles)
