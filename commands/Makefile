#compile source commands to the ras/bin/
CC := g++
DESTDIR = ~/Bitbucket/np_project1_0556091/ras/bin/

source :  
	cp /bin/ls /bin/cat $(DESTDIR)
	g++ noop.cpp -o $(DESTDIR)noop
	g++ number.cpp -o $(DESTDIR)number
	g++ removetag.cpp -o $(DESTDIR)removetag
	g++ removetag0.cpp -o $(DESTDIR)removetag0

clean : 
	rm $(DESTDIR)/ls
	rm $(DESTDIR)/cat
	rm $(DESTDIR)/noop
	rm $(DESTDIR)/number
	rm $(DESTDIR)/removetag
	rm $(DESTDIR)/removetag0
