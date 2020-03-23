
#include "types.h"
#include "hd.h"
#include "lib.h"

#define FS_TYPE 0x1f2f3f4f
#define MAX_INODE_COUNT 4096

typedef struct _SuperBlock
{
    u32 type; // magic word
    u32 sectorCnt;
    u32 inodeCnt;
    u32 inodeSize;

    u32 inodeMapSectorCnt;
    u32 sectorMapSectorCnt;
    u32 inodeArraySectorCnt;

    u32 dataSectorStartNo;
    u32 rootDirIdx;
} SuperBlock;

// 最好保证sizeof（INode）是SectorSize（512）的约数
typedef struct _Inode
{
    u32 fileMode;
    u32 filetype;
    u32 filesize;
    u32 startSector;
} Inode;

typedef enum
{
    DIR,
    TEXT,
    BINARY,
    DEVICE
} FileType;

void mkfs(u8 hdIdx, u8 partIdx);

void taskFs()
{
    mkfs(0, 4);

    while (1)
    {
    }
}

// 对硬盘分区建立化文件系统
void mkfs(u8 hdIdx, u8 partIdx)
{
    u8 device = hdIdx & 1;
    u8 chanel = hdIdx >> 1;
    u32 sectorCnt = allHd[hdIdx].partInfo[partIdx].sectorCnt;
    u32 lba = allHd[hdIdx].partInfo[partIdx].startSector;

    u16 buf[SECTOR_SIZE / 2];
    memSet((u8 *)buf, 0, sizeof(buf));
    //1、 准备超级块并写入磁盘
    SuperBlock sb;
    sb.type = FS_TYPE;
    sb.rootDirIdx = 1;
    sb.inodeCnt = MAX_INODE_COUNT;
    sb.sectorCnt = sectorCnt;
    sb.inodeSize = sizeof(Inode);
    sb.inodeMapSectorCnt = (sb.inodeCnt + SECTOR_SIZE * 8 - 1) / (SECTOR_SIZE * 8);
    sb.sectorMapSectorCnt = (sb.sectorCnt + SECTOR_SIZE * 8 - 1) / (SECTOR_SIZE * 8);
    sb.inodeArraySectorCnt = (sb.inodeCnt * sb.inodeSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
    sb.dataSectorStartNo = lba + 1 + 1 + sb.inodeMapSectorCnt + sb.sectorMapSectorCnt + sb.inodeArraySectorCnt; // boot(1),SuperBlock(1),InodeMap, SectorMap, InodeArray
    memCpy((u8 *)buf, (u8 *)&sb, sizeof(SuperBlock));
    writeHd(device, chanel, lba + 1, 1, buf); // 写入SuperBlock

    //2、写入inode map
    memSet((u8 *)buf, 0, sizeof(buf));
    buf[0] = 0x0001; // inode-0特殊用途， inode-1为rootDir
    writeHd(device, chanel, lba + 1 + 1, 1, buf);

    //3. 写入sector map
    memSet((u8 *)buf, 0, sizeof(buf));
    writeHd(device, chanel, lba + 1 + 1 + sb.inodeMapSectorCnt, 1, buf);

    // 4. 写入inode array
    memSet((u8 *)buf, 0, sizeof(buf)) ;
    // Inode *pnode = (Inode *)buf;
    // pnode[sb.rootDirIdx].filetype = DIR;
    // pnode[sb.rootDirIdx].fileMode = 0;
    // pnode[sb.rootDirIdx].filesize = 0;
    // pnode[sb.rootDirIdx].startSector = 0;
    writeHd(device, chanel, lba + 1 + 1 + sb.inodeMapSectorCnt + sb.sectorMapSectorCnt, 1, buf);
    
    //5.
}
