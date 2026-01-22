CC = gcc
CFLAGS += -g -Og -std=gnu17 -Wall -Wno-unused-result -DLINUX -DTYPECOLOR
LFLAGS = -lcrypt -lm -lsodium -rdynamic
OBJDIR = ../obj
BINDIR = ../bin
TYPE ?= $(shell basename $$(dirname $$PWD))

$(info Making ${TYPE}...)

ifneq ($(TYPE), live)
	CFLAGS += -DTEST_SITE
endif

#
# Generic Rules
#

.PHONY : all
all : ronin roninslave list_vaults

.PHONY : clean
clean :
	-rm $(OBJDIR)/ronin $(OBJS_RONIN)
	-rm $(OBJDIR)/roninslave $(OBJS_SLAVE)
	-rm $(OBJDIR)/list_vaults $(OBJS_LIST_VAULTS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) :
	@mkdir -p $(OBJDIR)

$(BINDIR) :
	@mkdir -p $(BINDIR)

#
# List Vaults
#

OBJS_LIST_VAULTS = $(OBJDIR)/list_vaults.o

$(OBJS_SLAVE) :| $(OBJDIR) $(BINDIR)

.PHONY: list_vaults
list_vaults : $(OBJDIR)/list_vaults

$(OBJDIR)/list_vaults : $(OBJS_LIST_VAULTS)
	$(CC) -o $(OBJDIR)/list_vaults $(CFLAGS) $(OBJS_LIST_VAULTS)

#
# Slave
#

OBJS_SLAVE = $(OBJDIR)/slave.o

$(OBJS_SLAVE) :| $(OBJDIR) $(BINDIR)
$(OBJS_SLAVE) : slave.h

.PHONY: roninslave
roninslave : $(OBJDIR)/roninslave

$(OBJDIR)/roninslave : $(OBJS_SLAVE)
	$(CC) -o $(OBJDIR)/roninslave $(CFLAGS) $(OBJS_SLAVE)

#
# Ronin
#

OBJS_RONIN_BASE = \
	act.comm.o \
	act.informative.o \
	act.movement.o \
	act.obj1.o \
	act.obj2.o \
	act.offensive.o \
	act.other.o \
	act.social.o \
	act.wizard.o \
	aff_ench.o \
	auction.o \
	board.o \
	char_spec.o \
	constants.o \
	comm.o \
	create.o \
	db.o \
	enchant.o \
	fight.o \
	handler.o \
	interpreter.o \
	limits.o \
	magic.o \
	memory.o \
	mob.magic.o \
	mobact.o \
	modify.o \
	reception.o \
	score.o \
	signals.o \
	song_parser.o \
	spec.vote.o \
	spells.o \
	spell_parser.o \
	utility.o \
	weather.o \
	wizlist.o

OBJS_RONIN_SPEC = \
	aquest.o \
	gamble.o \
	hunt.o \
	meta.o \
	paper.o \
	quest.o \
	remortv2.o \
	shop.o \
	spec.abyss.o \
	spec.barovia.o \
	spec.boat.o \
	spec.cafeteria.o \
	spec.cavern.o \
	spec.chaos.o \
	spec.christmasvillage.o \
	spec.clan.o \
	spec.ctower.o \
	spec.daimyo.o \
	spec.darkspire.o \
	spec.dasharr.o \
	spec.deathplay.o \
	spec.deathplayancientgrave.o \
	spec.deathplaycrypt.o \
	spec.desert.o \
	spec.dionysus.o \
	spec.dragonred.o \
	spec.druids.o \
	spec.ecanyon.o \
	spec.elf.o \
	spec.elmuseo.o \
	spec.emith.o \
	spec.enchanted.o \
	spec.ershteep.o \
	spec.eryndlyn.o \
	spec.feach.o \
	spec.goblin.o \
	spec.golden.o \
	spec.grail.o \
	spec.haondor.o \
	spec.hell.o \
	spec.helventia.o \
	spec.house.o \
	spec.invasion.o \
	spec.jungle.o \
	spec.keening.o \
	spec.labyrinth.o \
	spec.lost.o \
	spec.mapmaker.o \
	spec.marikith.o \
	spec.medievalblackmarket.o\
	spec.midgaard.o \
	spec.misc.o \
	spec.monastery.o \
	spec.monkey.o \
	spec.morgoth.o \
	spec.moria.o \
	spec.nergal.o \
	spec.newbie.o \
	spec.newt.o \
	spec.norway.o \
	spec.pagoda.o \
	spec.pirate.o \
	spec.questgearII.o \
	spec.quests.o \
	spec.questyvaderII.o \
	spec.rank.o \
	spec.remorhaz.o \
	spec.sphinx.o \
	spec.stable.o \
	spec.swamp.o \
	spec.swordoftruth.o \
	spec.tarioncity.o \
	spec.theldon.o \
	spec.topknot.o \
	spec.turkeytakeover.o \
	spec.trollcastle-moat.o \
	spec.trollvillage.o \
	spec.tweefmanor.o \
	spec.ubers.o \
	spec.underworld.o \
	spec.vagabond.o \
	spec.vanity.o \
	spec.vermillion.o \
	spec.welmar.o \
	spec.wolf.o \
	spec.WOT.o \
	spec.wsewer.o \
	spec.wyvern.o \
	spec.zankingdom.o \
	spec.zoo.o \
	spec.zyca.o \
	spec_assign.o \
	subclass.o \
	subclass.spells.o \
	subclass.skills.o 

OBJS_RONIN_SPEC_TEST = \
	spec.chess.o \
	spec.crazylab.o \
	spec.digsite.o \
	spec.elementalmines.o \
	spec.luthienIV.o \
	spec.happybob.o \
	spec.workbench.o \
	spec.olympus.o \

ifneq ($(TYPE), live)
	ifneq ($(TYPE), chaos)
		OBJS_RONIN_SPEC += $(OBJS_RONIN_SPEC_TEST)
	endif
endif

OBJS_RONIN_BASE := $(patsubst %, $(OBJDIR)/%, $(OBJS_RONIN_BASE))
OBJS_RONIN_SPEC := $(patsubst %, $(OBJDIR)/%, $(OBJS_RONIN_SPEC))

$(OBJS_RONIN_BASE) : structs.h utils.h comm.h interpreter.h handler.h db.h spells.h limits.h cmd.h enchant.h act.h spec.clan.h constants.h quest.h aquest.h
$(OBJS_RONIN_SPEC) : structs.h utils.h comm.h interpreter.h handler.h db.h spells.h limits.h cmd.h enchant.h act.h spec_assign.h constants.h aff_ench.h

$(OBJDIR)/act.movement.o : subclass.h
$(OBJDIR)/act.offensive.o : subclass.h
$(OBJDIR)/act.other.o : subclass.h
$(OBJDIR)/act.wizard.o : remortv2.h
$(OBJDIR)/aquest.o : subclass.h
$(OBJDIR)/fight.o : fight.h subclass.h
$(OBJDIR)/handler.o : subclass.h
$(OBJDIR)/hunt.o : subclass.h
$(OBJDIR)/limits.o : subclass.h
$(OBJDIR)/magic.o : subclass.h
$(OBJDIR)/meta.o : meta.h
$(OBJDIR)/mobact.o : subclass.h
$(OBJDIR)/remortv2.o : fight.h meta.h remortv2.h
$(OBJDIR)/score.o : score.h
$(OBJDIR)/spec.elmuseo.o : subclass.h
$(OBJDIR)/spec.midgaard.o : subclass.h
$(OBJDIR)/spec.mapmaker.o : quest.h
$(OBJDIR)/spec.vote.o : spec.vote.h
$(OBJDIR)/spec.zyca.o : subclass.h
$(OBJDIR)/spell_parser.o : subclass.h
$(OBJDIR)/subclass.o : subclass.h
$(OBJDIR)/subclass.spells.o : subclass.h
$(OBJDIR)/subclass.skills.o : subclass.h

OBJS_RONIN := $(OBJS_RONIN_BASE) $(OBJS_RONIN_SPEC)

$(OBJS_RONIN) :| $(OBJDIR) $(BINDIR)

.PHONY : ronin
ronin : $(OBJDIR)/ronin

$(OBJDIR)/ronin : $(OBJS_RONIN)
	$(CC) -o $(OBJDIR)/ronin $(CFLAGS) $(OBJS_RONIN) $(LFLAGS)
