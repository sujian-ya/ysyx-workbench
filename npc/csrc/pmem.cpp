#include "pmem.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <utils.h> // 使用Log和Assert宏

// 内存空间，使用uint32_t以字为单位，更通用
uint32_t pmem[PMEM_SIZE / 4];

// 初始化内存，加载指令
void pmem_init(const char* bin_file) {
    FILE* fp = NULL;
    if (bin_file == NULL) {
        Log("No image is given. Use the default built-in image.");
        pmem[0] = 0x00000013; // NOP (addi x0, x0, 0)
        pmem[1] = 0x00100093; // addi x1, x0, 1
        pmem[2] = 0x00200113; // addi x2, x0, 2
        pmem[3] = 0x00100073; // EBREAK
        return;
    } else {
        fp = fopen(bin_file, "rb");
        Assert(fp, "Can not open '%s'", bin_file);
        
        Log("Image is given. Use the specified image.");
        fseek(fp, 0, SEEK_END);
        size_t file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        assert(file_size <= PMEM_SIZE);

        size_t ret = fread(pmem, file_size, 1, fp);
        fclose(fp);
        
        // 检查返回值，如果读取失败则终止程序
        if (ret != 1) {
            Assert(0, "Failed to read image file '%s'", bin_file);
        }
    }
    return;
}

uint32_t pmem_read(uint32_t addr) {
    assert((addr & 0x3) == 0); // 地址必须4字节对齐，直接位数比较加快处理速度
    uint32_t index = (addr - 0x80000000) >> 2;
    assert(index < PMEM_SIZE); // 确保地址在范围内
    return pmem[index];
}
