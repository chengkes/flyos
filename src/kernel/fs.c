
#include "types.h"

typedef struct _SuperBlock {
    u32 type; // magic word

    u32 sectorCnt;

    u32 inodeMapSectorCnt;
    u32 sectorMapSectorCnt;
    u32 inodeSectorCnt;
    u32 inodeCnt;
    u32 inodeSize;
    
    u32 dataSectorStartSector;
    u32 rootDirIdx;

} SuperBlock;

typedef struct _Inode {
    u32 fid;
    u32 filesize;
    u32 filetype;
    u32 startSector;
} Inode;

void taskFs() {


}

void mkfs() {

    
} 

