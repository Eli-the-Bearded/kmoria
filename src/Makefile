TARGET = kMoria
APPNAME = "kMoria"
APPID = "Mria"
# NOTE: I have not requested an app id yet.

# spells.c, magic.c, prayer.c .. have a few lines commented out..
UIOBJS = form_about.o form_inv.o form_equ.o form_recall.o form_chargen.o \
       form_store.o form_spell.o form_messagelog.o form_prefs.o \
       form_map.o

#OBJS = main.o display.o lock.o generate.o sets.o desc.o recall.o util.o 
#       misc1.o misc2.o misc3.o misc4.o moria1.o moria2.o create.o 
#       eat.o potions.o staffs.o wands.o   store1.o store2.o 
#       magic.o prayer.o spells.o moria3.o moria4.o misc5.o 
#       creature.o 
# monsters, player, tables, treasure, variable: these are just data
# io: this will be partly implemented elsewhere partly ignored.
# rnd, wizard, signals: these will be ignored
# death dungeon files help save: these need to be munged.

OBJS = create.o creature.o desc.o \
       dungeon.o \
       eat.o generate.o magic.o main.o \
       misc1.o misc2.o misc3.o misc4.o moria1.o moria2.o moria3.o moria4.o \
       potions.o prayer.o recall.o sets.o scrolls.o spells.o staffs.o \
       store1.o store2.o wands.o \
       misc5.o display.o lock.o util.o \
       $(UIOBJS) $(TARGET)-sections.o

INCS = kMoriaRsc.h kMoria.h

CC = m68k-palmos-gcc

# Moria will probably be OS 3.*  sorry all you palm pilot professionals.
# But I will have to do palmos2 if I want to test on my old xcopilot.  rats.
CFLAGS = -Wall -g -O2 -I. -palmos2 -DI_AM_OS_2
#CFLAGS = -Wall -g -O2 -I. -palmos3.1 -DNON_INTERNATIONAL

PILRC = pilrc
BUILDPRC = build-prc

all: kMoria.prc

install: kMoria.prc
	/usr/local/pilot/bin/pilot-xfer /dev/ttyqe -i kMoria.prc

.S.o:
	$(CC) $(TARGETFLAGS) -c $<

.c.s:
	$(CC) $(CSFLAGS) $<


$(TARGET).prc: $(TARGET) bin.stamp
	$(BUILDPRC) $(TARGET).def $(TARGET) *.bin

$(TARGET): $(OBJS) $(TARGET)-sections.ld
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(TARGET)-sections.ld

bin.stamp: $(TARGET).rcp $(TARGET).pbm Font56.txt
	$(PILRC) $(TARGET).rcp .
	touch bin.stamp

$(TARGET)-sections.o: $(TARGET)-sections.s
	$(CC) -c $(TARGET)-sections.s

$(TARGET)-sections.s $(TARGET)-sections.ld: $(TARGET).def
	multigen $(TARGET).def

depend:
	makedepend -Y  *.c

dingy:
	-rm -f *.[oa] $(TARGET)

clean:
	-rm -f *.[oa] $(TARGET) *.bin bin.stamp *.grc Makefile.bak $(TARGET)-sections.s $(TARGET)-sections.ld

veryclean: clean
	-rm -f $(TARGET).prc pilot.ram pilot.scratch *.ld *.s

# DO NOT DELETE

create.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
create.o: lock-externs.h misc-externs.h
create_db.o: dbrecords.h
creature.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
creature.o: lock-externs.h misc-externs.h
dbrecords.o: dbrecords.h
desc.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
desc.o: lock-externs.h misc-externs.h
display.o: kMoriaRsc.h kMoria.h sections.h constant.h types.h prototypes.h
display.o: records.h lock-externs.h misc-externs.h
dungeon.o: kMoriaRsc.h kMoria.h sections.h constant.h types.h prototypes.h
dungeon.o: records.h lock-externs.h misc-externs.h
eat.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
eat.o: lock-externs.h misc-externs.h
form_about.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
form_about.o: lock-externs.h misc-externs.h kMoriaRsc.h
form_chargen.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
form_chargen.o: lock-externs.h misc-externs.h kMoriaRsc.h
form_equ.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
form_equ.o: lock-externs.h misc-externs.h kMoriaRsc.h
form_inv.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
form_inv.o: lock-externs.h misc-externs.h kMoriaRsc.h
form_messagelog.o: kMoria.h sections.h constant.h types.h prototypes.h
form_messagelog.o: records.h lock-externs.h misc-externs.h kMoriaRsc.h
form_recall.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
form_recall.o: lock-externs.h misc-externs.h kMoriaRsc.h
form_spell.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
form_spell.o: lock-externs.h misc-externs.h kMoriaRsc.h
form_store.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
form_store.o: lock-externs.h misc-externs.h kMoriaRsc.h
generate.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
generate.o: lock-externs.h misc-externs.h
io.o: constant.h types.h
lock.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
magic.o: kMoriaRsc.h kMoria.h sections.h constant.h types.h prototypes.h
magic.o: records.h lock-externs.h misc-externs.h
main.o: kMoriaRsc.h kMoria.h sections.h constant.h types.h prototypes.h
main.o: records.h lock-externs.h misc-externs.h
misc1.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
misc1.o: lock-externs.h misc-externs.h
misc2.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
misc2.o: lock-externs.h misc-externs.h
misc3.o: kMoriaRsc.h kMoria.h sections.h constant.h types.h prototypes.h
misc3.o: records.h lock-externs.h misc-externs.h
misc4.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
misc4.o: lock-externs.h misc-externs.h
misc5.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
misc5.o: lock-externs.h misc-externs.h
moria1.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
moria1.o: lock-externs.h misc-externs.h
moria2.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
moria2.o: lock-externs.h misc-externs.h
moria3.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
moria3.o: lock-externs.h misc-externs.h
moria3a.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
moria3a.o: lock-externs.h misc-externs.h
moria4.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
moria4.o: lock-externs.h misc-externs.h
potions.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
potions.o: lock-externs.h misc-externs.h
prayer.o: kMoriaRsc.h kMoria.h sections.h constant.h types.h prototypes.h
prayer.o: records.h lock-externs.h misc-externs.h
recall.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
recall.o: lock-externs.h misc-externs.h
scrolls.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
scrolls.o: lock-externs.h misc-externs.h
sets.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
spells-b.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
spells-b.o: lock-externs.h misc-externs.h
spells.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
spells.o: lock-externs.h misc-externs.h
staffs.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
staffs.o: lock-externs.h misc-externs.h
store1.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
store1.o: lock-externs.h misc-externs.h
store2.o: kMoriaRsc.h kMoria.h sections.h constant.h types.h prototypes.h
store2.o: records.h lock-externs.h misc-externs.h
tmp.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
tmp.o: lock-externs.h misc-externs.h
wands.o: kMoria.h sections.h constant.h types.h prototypes.h records.h
wands.o: lock-externs.h misc-externs.h
