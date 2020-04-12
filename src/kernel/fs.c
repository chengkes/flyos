
#include "types.h"
#include "hd.h"
#include "lib.h"
#include "pcb.h"
#include "tty.h"
#include "main.h"

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

    u32 startSectorNo;
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
void mkfs(HdInfo* hd , u8 partIdx, SuperBlock* sb);
int fexist(HdInfo* hd, SuperBlock* s, char* filename, Inode* p) ;

void taskFs()
{
    // test writeHd and readHd, use shell command xxd to verify
    // u16 buf[SECTOR_SIZE];
    // memSet((u8 *)buf, 0, sizeof(buf));
    // writeHd(0, 0, 1, 1,  buf);
    // readHd(0, 0, 1, 2, buf);
    // for(int i=0;i<sizeof(buf)/2; i++) {
    //     printf("%x", buf[i]);
    //     if (i %16==0) printf("\n");
    // }

    HdInfo allHd[10];
    int hdCount = identifyHd (allHd);
    assert(hdCount>0);
    assert(allHd[0].partCnt >0);

    u8 partIdx = 0;
    SuperBlock sb;
    readSuperBlock(allHd, partIdx, &sb); 
    if (sb.type != FS_TYPE) {
        mkfs(allHd, partIdx, &sb);         
    }  
    assert(sb.type == FS_TYPE);    

    int i = fexist(allHd, &sb, "not exist", NULL);

    printf("start fs task. %d ..\n", i);
    while (1)
    {    }
}

void readSuperBlock(HdInfo* hd, u8 partIdx, OUT SuperBlock* sb) {
    u32 lba = hd->partInfo[partIdx].startSector;
    u8 buf[SECTOR_SIZE];
    readHd(hd->device, hd->chanel, lba+1, 1,  (u16*)buf);
    memCpy((u8*)sb, buf, sizeof(SuperBlock));
}

// 对硬盘分区建立化文件系统
// 扇区功能分配依次为： BootScector(1) | SuperBlock(1) | InodeMaps(1) | SectorMaps | InodeArrays | DataSectors
// @param hd 硬盘信息
// @param partIdx，分区索引
// @note inode[0] 保留为特殊用途，表示空
// 文件系统不支持目录，所有文件最大可用空间固定为512kb,且连续分配
void mkfs(HdInfo* hd , u8 partIdx, OUT SuperBlock* sb)
{     
    //1、 准备SuperBlock
    sb->fileSectorCnt = 32; // 每个文件固定分配s多少个Sector
    sb->type = FS_TYPE;  
    sb->inodeCnt = MAX_INODE_COUNT; 
    sb->sectorCnt = hd->partInfo[partIdx].sectorCnt;
    sb->inodeSize = sizeof(Inode);
    sb->startSectorNo = hd->partInfo[partIdx].startSector;
    sb->inodeMapSectorCnt = (sb->inodeCnt + SECTOR_SIZE * 8 - 1) / (SECTOR_SIZE * 8);
    sb->sectorMapSectorCnt = (sb->sectorCnt + SECTOR_SIZE * 8 - 1) / (SECTOR_SIZE * 8);
    sb->inodeArraySectorCnt = (sb->inodeCnt * sb->inodeSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
    sb->dataSectorStartNo = sb->startSectorNo  + 1 + 1 + sb->inodeMapSectorCnt + sb->sectorMapSectorCnt + sb->inodeArraySectorCnt;

    u8 buf[SECTOR_SIZE*BUF_SECTOR_CNT];
    //2 将InodeMap 和 SectorMap 都置0
    memSet(buf, 0, BUF_SECTOR_CNT*SECTOR_SIZE);
    u32 startLba = sb->startSectorNo + 1 + 1;
    u32 leftSectorCnt = sb->inodeMapSectorCnt + sb->sectorMapSectorCnt; 
    while (leftSectorCnt > 0)
    {
        int sectorCnt2Write = BUF_SECTOR_CNT > leftSectorCnt ? leftSectorCnt : BUF_SECTOR_CNT;
        writeHd(hd->device, hd->chanel, startLba, sectorCnt2Write, buf);
        startLba += sectorCnt2Write;
        leftSectorCnt -= sectorCnt2Write;
    }

    // 3 写入SuperBlock
    memCpy(buf, (u8*)sb, sizeof(SuperBlock));
    writeHd(hd->device, hd->chanel, sb->startSectorNo + 1, 1, buf); // 写入SuperBlock
}


// 根据inodeIdx获取Inode信息
static void getInode(HdInfo* hd, SuperBlock* s,int inodeIdx, Inode* p){
    u8 buf[SECTOR_SIZE];
    u32 inodeArrayStartNo = s->startSectorNo + 1+1 + s->inodeMapSectorCnt + s->sectorMapSectorCnt;
    u32 inodeStartNo = inodeArrayStartNo + inodeIdx*s->inodeSize / SECTOR_SIZE;
    u32 inodeStartByte = (inodeIdx*s->inodeSize) % SECTOR_SIZE;

    readHd(hd->device, hd->chanel, inodeStartNo, 1, buf);
    memCpy((u8*)p, buf+inodeStartByte, sizeof(Inode));
}

// 判断文件是否存在,存在返回fidg并设置Inode信息，否则为-1
int fexist(HdInfo* hd, SuperBlock* s, char* filename, Inode* p) {
    u8 inodeMapBuf[SECTOR_SIZE*BUF_SECTOR_CNT];
    int inodeMapSectorIdx = s->startSectorNo + 1 +1 ;
    int inodeIdx = 0;
    Inode node;
    while(inodeIdx <= s->inodeCnt) {
        int sectorCntRead = s->inodeMapSectorCnt > BUF_SECTOR_CNT ? BUF_SECTOR_CNT : s->inodeMapSectorCnt;
        readHd(hd->device, hd->chanel, inodeMapSectorIdx, sectorCntRead, inodeMapBuf);

        for (int i =0; i<sectorCntRead*SECTOR_SIZE; i++) {
            for(int bit=0; bit<8; ++bit)  {
                if ((inodeMapBuf[i]>>bit) & 1) {
                    getInode(hd, s, inodeIdx + i*8 + bit, &node);
                    printf( "...check file: %s \n", node.filename );
                    if (0==strcmp(node.filename, filename)) {
                        if (p != NULL) memCpy((u8*)p, (u8*)&node, sizeof(Inode));
                        return inodeIdx + i*8 + bit;
                    }
                }
            }
        }

        inodeIdx += sectorCntRead * SECTOR_SIZE * 8; 
        inodeMapSectorIdx += sectorCntRead;
    }
 
    return -1;
}

int fcreate(HdInfo* hd, SuperBlock* sb, char* filename) {
    assert(fexist(hd, sb, filename, NULL)==-1);


    return -1;
}

// u32 fopen(char* filename) ;
// int fread(u32 fid, u8* buf, u32 size) ;
// int fclose(u32 fid);
// int fwrite(u32 fid, u8* data, u32 size) ;
