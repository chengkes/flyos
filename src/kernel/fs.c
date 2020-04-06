
#include "types.h"
#include "hd.h"
#include "lib.h"
#include "pcb.h"
#include "tty.h"

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
    u32 fileSectorCnt;   // 每个文件分配扇区数
} SuperBlock;

// 最好保证sizeof（INode）是SectorSize（512）的约数
typedef struct _Inode
{
    u32 fileMode;
    u32 filetype;
    u32 filesize;
    u32 startSector;
    char filename[16];
} Inode;

typedef enum
{
    DIR,
    TEXT,
    BINARY,
    DEVICE
} FileType;

void readSuperBlock(HdInfo* hd, u8 partIdx, SuperBlock* sb);
void mkfs(HdInfo* hd , u8 partIdx);

void taskFs()
{
    HdInfo allHd[10];
    identifyHd (allHd);  

    u8 partIdx = 4;
    SuperBlock sb;
    readSuperBlock(allHd, partIdx, &sb);
    printf("befor mkfs: %x \n", sb.type);
    mkfs(allHd, partIdx);     
    readSuperBlock(allHd, partIdx, &sb);
    printf("after mkfs: %x \n", sb.type);

    while (1)
    {

    }
}

void readSuperBlock(HdInfo* hd, u8 partIdx, SuperBlock* sb) {
    u32 lba = hd->partInfo[partIdx].startSector;
    u8 buf[SECTOR_SIZE];
    readHd(hd, (u16*)buf, lba, 1);
    memCpy((u8*)sb, buf, sizeof(SuperBlock));
}



// 对硬盘分区建立化文件系统
// @param hd 硬盘信息
// @param partIdx，分区索引
// @note inode[0] 保留为特殊用途，表示空
// 文件系统不支持目录，所有文件最大可用空间固定为512kb,且连续分配
void mkfs(HdInfo* hd , u8 partIdx)
{ 
    // u8 device = hd->device;
    // u8 chanel = hd->chanel;
    u32 sectorCnt = hd->partInfo[partIdx].sectorCnt;
    u32 lba = hd->partInfo[partIdx].startSector;

    //1、 准备超级块并写入磁盘
    SuperBlock sb;
    sb.fileSectorCnt = 1024; // 每个文件固定分配1024个Sector
    sb.type = FS_TYPE;
    sb.inodeCnt = MAX_INODE_COUNT;
    sb.sectorCnt = sectorCnt;
    sb.inodeSize = sizeof(Inode);
    sb.inodeMapSectorCnt = (sb.inodeCnt + SECTOR_SIZE * 8 - 1) / (SECTOR_SIZE * 8);
    sb.sectorMapSectorCnt = (sb.sectorCnt + SECTOR_SIZE * 8 - 1) / (SECTOR_SIZE * 8);
    sb.inodeArraySectorCnt = (sb.inodeCnt * sb.inodeSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
    sb.dataSectorStartNo = lba + 1 + 1 + sb.inodeMapSectorCnt + sb.sectorMapSectorCnt + sb.inodeArraySectorCnt; // boot(1),SuperBlock(1),InodeMap, SectorMap, InodeArray

    // writeHd(device, chanel, lba + 1, 1, buf); // 写入SuperBlock

    // //2、写入inode map
    // memSet((u8 *)buf, 0, sizeof(buf));
    // buf[0] = 0x0001; // inode-0特殊用途， inode-1为rootDir
    // writeHd(device, chanel, lba + 1 + 1, 1, buf);

    // //3. 写入sector map
    // memSet((u8 *)buf, 0, sizeof(buf));
    // writeHd(device, chanel, lba + 1 + 1 + sb.inodeMapSectorCnt, 1, buf);

    // // 4. 写入inode array
    // writeHd(device, chanel, lba + 1 + 1 + sb.inodeMapSectorCnt + sb.sectorMapSectorCnt, 1, buf);
}

int fcreate(char* filename) {
    // 
    return 0;
}

u32 fopen(char* filename) {
    // 根据文件名查找inode array
    

    return 0;
}

int fread(u32 fid, u8* buf, u32 size) {
    return 0;
}

int fclose(u32 fid) {
    return 0;
}

int fwrite(u32 fid, u8* data, u32 size) {
    return 0;
}

