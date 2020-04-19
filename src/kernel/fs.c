
// TODO: fwrite, fread, fcreate-dev-bin

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

    u32 startSectorNo;
    u32 fileSectorCnt; // 每个文件分配扇区数

    u32 inodeMapSectorCnt;
    u32 sectorMapSectorCnt;
    u32 inodeArraySectorCnt;
    u32 dataSectorStartNo;
} SuperBlock;

// 最好保证sizeof（INode）是SectorSize（512）的约数
#define MAX_FILENAME_LEN 16
typedef struct _Inode
{
    u32 fileMode;
    u32 filetype;
    u32 filesize;
    u32 startSector;
    char filename[MAX_FILENAME_LEN];
} Inode;

typedef enum
{
    DIR,
    TEXT,
    BINARY,
    DEVICE
} FileType;

typedef int InodeVisit(int inodeIdx, int nodeMapValue, Inode *p, u32 param);

void readSuperBlock(HdInfo *hd, u8 partIdx, SuperBlock *sb);
void mkfs(HdInfo *hd, u8 partIdx, SuperBlock *sb);
int fexist(HdInfo *hd, SuperBlock *s, char *filename, Inode *p);
int fcreate(HdInfo *hd, SuperBlock *sb, char *filename);
int showInode(int idx, int nodeMapValue, Inode *p, u32 param);
void accessInodeInfo(HdInfo *hd, SuperBlock *s, int inodeIdx, Inode *p, int isWrite);
int fInodeIter(HdInfo *hd, SuperBlock *s, InodeVisit visit, Inode *pInode, u32 param);
int fdelte(HdInfo *hd, SuperBlock *sb, char *filename);

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
    int hdCount = identifyHd(allHd);
    assert(hdCount > 0);
    assert(allHd[0].partCnt > 0);

    u8 partIdx = 0;
    SuperBlock sb;
    readSuperBlock(allHd, partIdx, &sb);
    // // if (sb.type != FS_TYPE) {
        mkfs(allHd, partIdx, &sb);
        readSuperBlock(allHd, partIdx, &sb);
    // // }

    assert(sb.type == FS_TYPE);

    // test file functions
    Inode node;
    fInodeIter(allHd, &sb, showInode, &node, 0); // 遍历所有Inode

    fcreate(allHd, &sb, "test1.txt");
    fcreate(allHd, &sb, "test2.txt");
    fcreate(allHd, &sb, "test3.txt");
    fcreate(allHd, &sb, "test4.txt");
    fInodeIter(allHd, &sb, showInode, &node, 0); // 遍历所有Inode

    fdelte(allHd, &sb, "test3.txt");
    fInodeIter(allHd, &sb, showInode, &node, 0); // 遍历所有Inode

    printfColor(red, "start fs task. %d ..\n", 12);
    while (1)
    {
        // for (int i = 0; i < 999999; i++)
            ;
        // printf("diff: %x - %x \n", getTicks(), getScheduleTicks() );
    }
}

void readSuperBlock(HdInfo *hd, u8 partIdx, OUT SuperBlock *sb)
{
    u32 lba = hd->partInfo[partIdx].startSector;
    u8 buf[SECTOR_SIZE];
    readHd(hd->device, hd->chanel, lba + 1, 1, (u16 *)buf);
    memCpy((u8 *)sb, buf, sizeof(SuperBlock));
}

// 对硬盘分区建立化文件系统
// 扇区功能分配依次为： BootScector(1) | SuperBlock(1) | InodeMaps(1) | SectorMaps | InodeArrays | DataSectors
// @param hd 硬盘信息
// @param partIdx，分区索引
// @note inode[0] 保留为特殊用途，表示空
// 文件系统不支持目录，所有文件最大可用空间固定为512kb,且连续分配
void mkfs(HdInfo *hd, u8 partIdx, OUT SuperBlock *sb)
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
    sb->dataSectorStartNo = sb->startSectorNo + 1 + 1 + sb->inodeMapSectorCnt + sb->sectorMapSectorCnt + sb->inodeArraySectorCnt;

    u8 buf[SECTOR_SIZE * BUF_SECTOR_CNT];
    //2 将InodeMap 和 SectorMap 都置0
    memSet(buf, 0, BUF_SECTOR_CNT * SECTOR_SIZE);
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
    memCpy(buf, (u8 *)sb, sizeof(SuperBlock));
    writeHd(hd->device, hd->chanel, sb->startSectorNo + 1, 1, buf); // 写入SuperBlock
}

//////////////////////////////////////////////////////////////////////////////////////////
///   Inode 相关操作

// 遍历所有Inode, 如果回调函数visit返回0,则结束遍历
// 回调函数 参数1：Inode的索引，
// 回调函数 参数2：Inode为空，表示未g使用否则返回Inode信息
int fInodeIter(HdInfo *hd, SuperBlock *s, InodeVisit visit, Inode *pInode, u32 param)
{
    u8 inodeMapBuf[SECTOR_SIZE * BUF_SECTOR_CNT];
    int inodeMapSectorIdx = s->startSectorNo + 1 + 1;
    int inodeIdx = 0;
    int nodeMapValue;

    while (inodeIdx < s->inodeCnt)
    {
        int sectorCntRead = s->inodeMapSectorCnt > BUF_SECTOR_CNT ? BUF_SECTOR_CNT : s->inodeMapSectorCnt;
        readHd(hd->device, hd->chanel, inodeMapSectorIdx, sectorCntRead, inodeMapBuf);

        for (int i = 0; i < sectorCntRead * SECTOR_SIZE; i++)
        {
            for (int bit = 0; bit < 8; ++bit)
            {
                nodeMapValue = (inodeMapBuf[i] >> bit) & 1;
                if (nodeMapValue && pInode != NULL)
                {
                    accessInodeInfo(hd, s, inodeIdx, pInode, 0);
                }

                if (0 == visit(inodeIdx, nodeMapValue, pInode, param))
                {
                    return inodeIdx;
                }
                ++inodeIdx;
                if (inodeIdx >= s->inodeCnt)
                    return -1;
            }
        }

        inodeMapSectorIdx += sectorCntRead;
    }

    return -1;
}

// 显示Inode信息
int showInode(int idx, int nodeMapValue, Inode *p, u32 param)
{
    if (nodeMapValue && p != NULL)
    {
        printf("Node(%x): %s \n", idx, p->filename);
    }
    else
    {
        //  printf("Node(%x) is Empty \n", idx);
    }
    return 1;
}

int _getEmptyInode(int idx, int nodeMapValue, Inode *p, u32 param)
{
    return nodeMapValue;
}

// 根据inodeIdx获取或写入InodeMap 位 信息
int accessInodeMap(HdInfo *hd, SuperBlock *s, int idx, int val, int isWrite)
{
    u32 sectorNo = s->startSectorNo + 1 + 1 + idx / (SECTOR_SIZE * 8); // 哪个扇区
    u32 byteIdx = (idx / 8) % SECTOR_SIZE;                             // 哪个字节
    u32 bits = idx % 8;                                                // 哪一位

    u8 buf[SECTOR_SIZE];
    readHd(hd->device, hd->chanel, sectorNo, 1, buf);

    u8 oldVal = (buf[byteIdx] >> bits) & 1;

    if (isWrite && oldVal != val)
    {
        if (val)
        {
            buf[byteIdx] |= (1 << bits);
        }
        else
        {
            buf[byteIdx] &= ~(1 << bits);
        }
        writeHd(hd->device, hd->chanel, sectorNo, 1, buf);
        return val;
    }
    else
    {
        return (buf[byteIdx] >> bits) & 1;
    }
}

// 根据inodeIdx获取或写入Inode信息
void accessInodeInfo(HdInfo *hd, SuperBlock *s, int inodeIdx, Inode *p, int isWrite)
{
    assert(p != NULL && hd != NULL && s != NULL);

    u8 buf[SECTOR_SIZE];
    u32 inodeArrayStartNo = s->startSectorNo + 1 + 1 + s->inodeMapSectorCnt + s->sectorMapSectorCnt;
    u32 inodeStartNo = inodeArrayStartNo + inodeIdx * s->inodeSize / SECTOR_SIZE;
    u32 inodeStartByte = (inodeIdx * s->inodeSize) % SECTOR_SIZE;

    readHd(hd->device, hd->chanel, inodeStartNo, 1, buf);
    if (isWrite)
    {
        memCpy(buf + inodeStartByte, (u8 *)p, sizeof(Inode));
        writeHd(hd->device, hd->chanel, inodeStartNo, 1, buf);
    }
    else
    {
        memCpy((u8 *)p, buf + inodeStartByte, sizeof(Inode));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

int _findFilename(int idx, int val, Inode *p, u32 param)
{
    if (val && p != NULL)
    {
        return strcmp(p->filename, (char *)param);
    }
    return 1;
}

// 释放sector开始的cnt*8个扇区
int freeSector(HdInfo *hd, SuperBlock *s, int sector, int cnt)
{
    u32 sectorMapSectorNo = s->startSectorNo + 1 + 1 + s->inodeMapSectorCnt + (sector - s->dataSectorStartNo) / (SECTOR_SIZE * 8);
    u32 byteNo = ((sector - s->dataSectorStartNo) / 8) % SECTOR_SIZE;
    u8 buf[SECTOR_SIZE * 2];
    readHd(hd->device, hd->chanel, sectorMapSectorNo, 2, buf);
    for (int i = 0; i < cnt; i++)
    {
        buf[byteNo + i] = 0;
    }
    writeHd(hd->device, hd->chanel, sectorMapSectorNo, 2, buf);
    return 0;
}

// 分配cnte*8个连续的空闲扇区，返回第一个扇区号
// 扇区以8个为单位进行e分配
int allocateeSector(HdInfo *hd, SuperBlock *s, int cnt)
{
    u32 sectorMapSectorNo = s->startSectorNo + 1 + 1 + s->inodeMapSectorCnt; // 起始扇区
    u8 buf[SECTOR_SIZE * BUF_SECTOR_CNT];
    u32 leftSectorCnt = s->sectorMapSectorCnt;
    u32 dataSectorCnt = s->sectorCnt - 1 - 1 - s->inodeMapSectorCnt - s->sectorMapSectorCnt - s->inodeArraySectorCnt;

    while (leftSectorCnt > 0)
    {
        int sectorCnt2Read = leftSectorCnt > BUF_SECTOR_CNT ? BUF_SECTOR_CNT : leftSectorCnt;
        readHd(hd->device, hd->chanel, sectorMapSectorNo, leftSectorCnt, buf);
        for (int i = 0; i < sectorCnt2Read * SECTOR_SIZE - cnt;)
        {
            u32 sectorMapIdx = (s->sectorMapSectorCnt - leftSectorCnt) * SECTOR_SIZE * 8 + i * 8;
            if (sectorMapIdx + cnt >= dataSectorCnt)
            {
                return -1; // 剩余空间不足
            }
            int j = 0;
            while (j < cnt)
            {
                if (buf[i + j] == 0)
                {
                    j++;
                }
                else
                {
                    break;
                }
            }
            if (j < cnt)
            {
                i += j + 1;
            }
            else
            {
                for (j = 0; j < cnt; j++)
                {
                    buf[i + j] = 0xff;
                }

                writeHd(hd->device, hd->chanel, sectorMapSectorNo, leftSectorCnt, buf);
                return s->dataSectorStartNo + sectorMapIdx;
            }
        }
        leftSectorCnt -= sectorCnt2Read;
        sectorMapSectorNo += sectorCnt2Read;
    }
    return -1;
}

// 删除文件
int fdelte(HdInfo *hd, SuperBlock *sb, char *filename)
{
    Inode node;
    int idx = fInodeIter(hd, sb, _findFilename, &node, (u32)filename);
    if (idx == -1)
    {
        printf("fdelte(): file doesnot exists %s \n", filename);
        return -1;
    }

    freeSector(hd, sb, node.startSector, (sb->fileSectorCnt + 7) / 8);
    accessInodeMap(hd, sb, idx, 0, 1);
    return 0;
}

// 将buf中sizee个i字节写入文件fid的位置seek处
int fwrite(HdInfo *hd, SuperBlock *sb,int fid, u8* buf,int seek, int size) {
    if (seek + size >= sb->fileSectorCnt*SECTOR_SIZE) {
        printf("fwrite 写入位置%d 大于文件容量%d \n", seek+size, sb->fileSectorCnt*SECTOR_SIZE);
        return -1;
    }

    if (0 == accessInodeMap(hd, sb, fid, 0, 0)) {
        printf("fwrite 文件不存在 %d \n",fid);
        return -1;
    }
    
    Inode node;
    accessInodeInfo(hd, sb, fid, &node, 0);
    node.filesize =  node.filesize > seek+size ? node.filesize :  seek+size ;
    // todo:

    accessInodeInfo(hd, sb, fid, &node, 1);
    return size;
}

// 创建文件，返回文件InodeIdx
int fcreate(HdInfo *hd, SuperBlock *sb, char *filename)
{
    int len = strlen(filename);
    if (len >= MAX_FILENAME_LEN)
    {
        printf("fcreate(%s): filename length shouldnot longer than %d  \n", filename, len);
        return -1;
    }

    Inode node;
    int idx = fInodeIter(hd, sb, _findFilename, &node, (u32)filename);
    if (idx != -1)
    {
        printf("fcreate(): file exists %s \n", filename);
        return -1;
    }

    // 找到一个空的Inode
    int inodeIdx = fInodeIter(hd, sb, _getEmptyInode, NULL, 0);
    if (inodeIdx == -1)
    {
        printf(" fcreate() :there is not empty Inode to suer");
        return -1;
    }

    // 写入Inode信息
    memSet((u8 *)&node, 0, sizeof(Inode));
    memCpy((u8 *)node.filename, (u8 *)filename, strlen(filename));
    node.filetype = TEXT;
    node.filesize = 0;
    node.startSector = allocateeSector(hd, sb, (sb->fileSectorCnt + 7) / 8);
    if (node.startSector == -1)
    {
        printf(" %s :there is not empty Sector", __FUNCTION__);
    }

    accessInodeInfo(hd, sb, inodeIdx, &node, 1);
    accessInodeMap(hd, sb, inodeIdx, 1, 1); // 写入InodeMap

    return -1;
}

// u32 fopen(char* filename) ;
// int fread(u32 fid, u8* buf, u32 size) ;
// int fclose(u32 fid);
// int fwrite(u32 fid, u8* data, u32 size) ;
