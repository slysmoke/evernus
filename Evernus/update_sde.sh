#!/bin/bash

if [ ! -f "$1" ] || [ ! -f "$2" ]; then
    echo "Usage: $0 sde target"
    exit 0
fi

TABLES=(industryActivity industryActivityMaterials industryActivityProducts industryActivitySkills invGroups invMarketGroups invMetaGroups invMetaTypes invTypeMaterials invTypes mapConstellations mapRegions mapSolarSystemJumps mapSolarSystems ramActivities staStations)

for TABLE in ${TABLES[*]}; do
    echo "Migrating $TABLE..."
    
    sqlite3 << EOF
        ATTACH '$1' AS SDE;
        ATTACH '$2' AS EVE;
        
        DELETE FROM EVE.$TABLE;
        INSERT INTO EVE.$TABLE SELECT * FROM SDE.$TABLE;
EOF
done

echo 'Migrating mapDenormalize...'
sqlite3 << EOF
        ATTACH '$1' AS SDE;
        ATTACH '$2' AS EVE;
        
        DELETE FROM EVE.mapDenormalize;
        INSERT INTO EVE.mapDenormalize SELECT * FROM SDE.mapDenormalize WHERE SDE.mapDenormalize.groupID IN (5, 15);
EOF

sqlite3 "$2" <<< "VACUUM;"
echo 'Done.'
