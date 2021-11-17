#!/bin/bash
TYPE=$(basename $PWD)
PREFIX="../backup/$TYPE/$(date +%Y%m%d)-$TYPE"
BASEDIR="."
EXT="tar.xz"
TAROPT="cfJv"

if [ -f $PREFIX-base.$EXT ]
then
  echo "File $PREFIX-base.$EXT already exists.  Skipping base backup..."
else
  tar $TAROPT $PREFIX-base.$EXT \
   --owner=ronin --group=ronin \
   --exclude=.git \
   --exclude='*.o' \
   --exclude="$BASEDIR/syslogs/*" \
   --exclude="$BASEDIR/runlog" \
   --exclude="$BASEDIR/syslog" \
   --exclude="$BASEDIR/syslog.last" \
   --exclude="$BASEDIR/bin/ronin" \
   --exclude="$BASEDIR/bin/roninslave" \
   --exclude="$BASEDIR/lib/*.messages" \
   --exclude="$BASEDIR/lib/auction.data" \
   --exclude="$BASEDIR/lib/clanlist.*" \
   --exclude="$BASEDIR/lib/clans" \
   --exclude="$BASEDIR/lib/death.log" \
   --exclude="$BASEDIR/lib/highest_id*" \
   --exclude="$BASEDIR/lib/lottery.data" \
   --exclude="$BASEDIR/lib/plrlog" \
   --exclude="$BASEDIR/lib/mail/*" \
   --exclude="$BASEDIR/lib/post/*" \
   --exclude="$BASEDIR/lib/rent/?/*.*" \
   --exclude="$BASEDIR/lib/rent/death/*" \
   --exclude="$BASEDIR/lib/vault/*" \
   --exclude="$BASEDIR/lib/names-*" \
   --exclude="$BASEDIR/lib/player_idname*" \
   --exclude="$BASEDIR/lib/rank_file*.dat" \
   --exclude="$BASEDIR/lib/slave.pid" \
   --exclude="$BASEDIR/lib/vote*.dat" \
   --exclude="$BASEDIR/lib/warnings.log" \
   --exclude="$BASEDIR/lib/wizlist*" \
   --exclude="$BASEDIR/obj/*" \
   $BASEDIR/
fi

if [ -f $PREFIX-data.$EXT ]
then
  echo "File $PREFIX-data.$EXT already exists.  Skipping data backup..."
else
  tar $TAROPT $PREFIX-data.$EXT \
   --owner=ronin --group=ronin \
   --exclude=.git \
   --exclude="*.o" \
   --exclude="$BASEDIR/lib/vault/vault.log" \
   $BASEDIR/lib/mail/ \
   $BASEDIR/lib/post/ \
   $BASEDIR/lib/rent/?/ \
   $BASEDIR/lib/rent/death/ \
   $BASEDIR/lib/vault/ \
   $BASEDIR/lib/*.messages \
   $BASEDIR/lib/clanlist.* \
   $BASEDIR/lib/*.messages \
   $BASEDIR/lib/auction.data \
   $BASEDIR/lib/clanlist.* \
   $BASEDIR/lib/clans \
   $BASEDIR/lib/highest_id* \
   $BASEDIR/lib/lottery.data \
   $BASEDIR/lib/names-* \
   $BASEDIR/lib/player_idname* \
   $BASEDIR/lib/rank_file*.dat \
   $BASEDIR/lib/vote*.dat \
   $BASEDIR/lib/wizlist*
fi

if [ -f $PREFIX-log.$EXT ]
then
  echo "File $PREFIX-log.$EXT already exists.  Skipping log backup..."
else
  tar $TAROPT $PREFIX-log.$EXT \
   --owner=ronin --group=ronin \
   --exclude=.git \
   --exclude="*.o" \
   $BASEDIR/syslogs/ \
   $BASEDIR/runlog \
   $BASEDIR/syslog \
   $BASEDIR/syslog.last \
   $BASEDIR/lib/death.log \
   $BASEDIR/lib/plrlog \
   $BASEDIR/lib/warnings.log \
   $BASEDIR/lib/vault/vault.log
fi
