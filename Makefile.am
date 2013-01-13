# $Id: $

AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip

bin_PROGRAMS = trimdump trimdiag

trimdump_SOURCES  = trimdump.c packet.l utility.c

trimdiag_SOURCES  = trimdiag.c packet.l utility.c

dist_man_MANS = trimdump.1 trimdiag.1

EXTRA_DIST  = 

.1.pdf:
	groff -man -Tps $< | pstopdf -o $@ -i

PDFMAN = trimdump.pdf trimdiag.pdf

pdfman: $(PDFMAN)


CLEANFILES = *~ test.out $(PDFMAN)

