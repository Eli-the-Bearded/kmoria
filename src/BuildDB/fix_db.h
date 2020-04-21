//extern DmOpenRef       iLarnDB;
#define iLarnDBName "iLarnDB"
#define iLarnDBType  'Data'
#define iLarnAppID   'Larn'

#include "../defines.h"

Boolean RebuildDatabase();
void make_dndstore_rec(DmOpenRef iLarnDB);
void make_canned_levels(DmOpenRef iLarnDB);
