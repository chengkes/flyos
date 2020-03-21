
#include "types.h"

#define FS_TYPE             0x1f2f3f4f
#define MAX_INODE_COUNT     4096

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
    u32 fileMode;
    u32 filetype;
    u32 filesize;
    u32 startSector;
    u32 sectorCnt;
} Inode;

typedef enum  {
    DIR,
    TEXT,
    BINARY
} FileType;

void taskFs() {

}

void mkfs() {
    //1、 准备超级块并写入磁盘

    SuperBlock sb ;
    sb.type = FS_TYPE;

    sb.inodeCnt = MAX_INODE_COUNT;
    sb.inodeSize = sizeof(Inode);
    // sb.inodeMapSectorCnt =  
    // sb.inodeSectorCnt = 
    

    //2、写入inode map

    //3. 写入sector map
    

} 

