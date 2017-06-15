#!/bin/bash
TYPE=$(basename $PWD)
PREFIX="../backup/$TYPE/$(date +%Y%m%d)-$TYPE"
BASEDIR="."
EXT="tar.xz"
TAROPT="Jcvf"

if [ -f $PREFIX-base.$EXT ]
then
  echo "File $PREFIX-base.$EXT already exists.  Skipping base backup..."
else
  tar $TAROPT $PREFIX-base.$EXT \
   --owner=ronin --group=ronin \
   --exclude=.svn \
   --exclude=.git \
   --exclude='*.o' \
   --exclude="$BASEDIR/syslogs/*" \
   --exclude="$BASEDIR/runlog" \
   --exclude="$BASEDIR/syslog" \
   --exclude="$BASEDIR/syslog.last" \
   --exclude="$BASEDIR/lib/plrlog*" \
   --exclude="$BASEDIR/lib/death.log" \
   --exclude="$BASEDIR/lib/warnings.log" \
   --exclude="$BASEDIR/lib/mail/*" \
   --exclude="$BASEDIR/lib/post/*" \
   --exclude="$BASEDIR/lib/PURGED/*" \
   --exclude="$BASEDIR/lib/PURGENEWBIE/*" \
   --exclude="$BASEDIR/lib/rent/?/*.*" \
   --exclude="$BASEDIR/lib/rent/*.list" \
   --exclude="$BASEDIR/lib/rent/*.stat" \
   --exclude="$BASEDIR/lib/rent/death/*" \
   --exclude="$BASEDIR/lib/TPURGE/*" \
   --exclude="$BASEDIR/lib/VAULTBACKUP/*" \
   --exclude="$BASEDIR/lib/vault/*" \
   --exclude="$BASEDIR/lib/VPURGED/*" \
   --exclude="$BASEDIR/lib/*.messages" \
   --exclude="$BASEDIR/lib/bugs.old.Z" \
   --exclude="$BASEDIR/lib/clanlist.*" \
   --exclude="$BASEDIR/lib/clans" \
   --exclude="$BASEDIR/lib/highest_id*" \
   --exclude="$BASEDIR/lib/auction.data" \
   --exclude="$BASEDIR/lib/lottery.data" \
   --exclude="$BASEDIR/lib/names-*" \
   --exclude="$BASEDIR/lib/player_idname*" \
   --exclude="$BASEDIR/lib/rank_file*.dat" \
   --exclude="$BASEDIR/lib/wizlist*" \
   --exclude="$BASEDIR/lib/slave.pid" \
   --exclude="$BASEDIR/lib/last_command.txt" \
   --exclude="$BASEDIR/lib/rent/eq-allfiles" \
   --exclude="$BASEDIR/obj/*" \
   --exclude="$BASEDIR/src/nlp" \
   --exclude="$BASEDIR/src/libdmalloc.a" \
   --exclude=bin/ronin \
   --exclude=bin/roninslave \
   $BASEDIR/ bin/
fi

if [ -f $PREFIX-data.$EXT ]
then
  echo "File $PREFIX-data.$EXT already exists.  Skipping data backup..."
else
  tar $TAROPT $PREFIX-data.$EXT \
   --owner=ronin --group=ronin \
   --exclude=.svn \
   --exclude=.git \
   --exclude="*.o" \
   --exclude="$BASEDIR/lib/vault/vault.log" \
   --exclude="$BASEDIR/lib/vault/vault-allfiles" \
   $BASEDIR/lib/mail/ \
   $BASEDIR/lib/post/ \
   $BASEDIR/lib/PURGED/ \
   $BASEDIR/lib/PURGENEWBIE/ \
   $BASEDIR/lib/rent/?/ \
   $BASEDIR/lib/rent/*.list \
   $BASEDIR/lib/rent/*.stat \
   $BASEDIR/lib/rent/death/ \
   $BASEDIR/lib/TPURGE/ \
   $BASEDIR/lib/VAULTBACKUP/ \
   $BASEDIR/lib/vault/ \
   $BASEDIR/lib/VPURGED/ \
   $BASEDIR/lib/*.messages \
   $BASEDIR/lib/bugs.old.Z \
   $BASEDIR/lib/clanlist.* \
   $BASEDIR/lib/clans \
   $BASEDIR/lib/highest_id* \
   $BASEDIR/lib/auction.data \
   $BASEDIR/lib/lottery.data \
   $BASEDIR/lib/names-* \
   $BASEDIR/lib/player_idname* \
   $BASEDIR/lib/rank_file*.dat \
   $BASEDIR/lib/wizlist*
fi

if [ -f $PREFIX-log.$EXT ]
then
  echo "File $PREFIX-log.$EXT already exists.  Skipping log backup..."
else
  tar $TAROPT $PREFIX-log.$EXT \
   --owner=ronin --group=ronin \
   --exclude=.svn \
   --exclude=.git \
   --exclude="*.o" \
   $BASEDIR/syslogs/ \
   $BASEDIR/runlog \
   $BASEDIR/syslog \
   $BASEDIR/syslog.last \
   $BASEDIR/lib/plrlog* \
   $BASEDIR/lib/death.log \
   $BASEDIR/lib/warnings.log \
   $BASEDIR/lib/vault/vault.log
fi
