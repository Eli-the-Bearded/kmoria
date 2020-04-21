TARGET = kMoriaBuild
APPNAME = "kMoriaBuild"
APPID = "Xyzz"

#OBJS = main.o object_list.o
#OBJS = main.o creature_list.o
#OBJS = main.o tlevel_sortobj.o
OBJS = main.o haggle_text.o

CC = m68k-palmos-gcc
CFLAGS = -Wall -g -O2 -I. -I.. -palmos2 -DI_AM_OS_2
#CFLAGS = -Wall -g -O2 -I. -I.. -palmos3.1 -DNON_INTERNATIONAL

PILRC = pilrc
BUILDPRC = build-prc

all: $(TARGET).prc

.S.o:
	$(CC) $(TARGETFLAGS) -c $<

.c.s:
	$(CC) $(CSFLAGS) $<

$(TARGET).prc: $(TARGET) bin.stamp
	$(BUILDPRC) $(TARGET).def $(TARGET) *.bin

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

bin.stamp: $(TARGET).rcp $(TARGET).pbm
	$(PILRC) $(TARGET).rcp .
	touch bin.stamp


send: $(TARGET).prc
	$(PILOTXFER) -i $(TARGET).prc

depend:
	makedepend -Y -I.. *.c

dingy:
	-rm -f *.[oa] $(TARGET)

clean:
	-rm -f *.[oa] $(TARGET) *.bin bin.stamp *.grc Makefile.bak

veryclean: clean
	-rm -f $(TARGET).prc pilot.ram pilot.scratch

# DO NOT DELETE

fix_db.o: iLarnFixDBRsc.h fix_db.h ../defines.h
rebuild.o: fix_db.h ../defines.h
