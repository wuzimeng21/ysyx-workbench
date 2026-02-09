/***************************************************************************************

* PA2 irigbuf.c

***************************************************************************************/

#include <utils.h>  // for using Log && log_write
#include <common.h>

# define IRINGBUF_NUM 1000
# define IRINGBUF_WIDTH 128

char iringbuf[IRINGBUF_NUM][IRINGBUF_WIDTH];
// head out, tail insert
int iringbuf_head, iringbuf_tail;


void init_iringbuf(){
    // loop , this buffer like a circle
    iringbuf_head = 0;
    iringbuf_tail = 0;
    return ;
}


void insert_iringbuf(const char *inst_trace) {
    strncpy(iringbuf[iringbuf_tail], inst_trace, IRINGBUF_WIDTH - 1);
    iringbuf_tail = (iringbuf_tail + 1) % IRINGBUF_NUM;
    if (iringbuf_tail == iringbuf_head) {
        iringbuf_head = (iringbuf_head + 1) % IRINGBUF_NUM;
    }
    return ;
}


// 弹出并打印环形缓冲区中的所有指令
void pop_iringbuf() {
    Log("==================== IRINGBUF (Recent Instructions) ====================");
    if (iringbuf_head == iringbuf_tail) {
        Log("No instructions in iringbuf (buffer is empty)!\n");
        return;
    }
    
    int i = iringbuf_head;
    int count = 0;
    
    // 遍历环形缓冲区
    while (i != iringbuf_tail) {
        // 检查是否是当前指令（在错误指令处标记）
        if (i == ((iringbuf_tail - 1 + IRINGBUF_NUM) % IRINGBUF_NUM)) {
            Log("--> %s", iringbuf[i]);  // 标记出错的指令
        } else {
            Log("    %s", iringbuf[i]);  // 普通指令
        }
        
        i = (i + 1) % IRINGBUF_NUM;
        count++;
        
        // 防止无限循环
        if (count > IRINGBUF_NUM) {
            Log("Error: iringbuf seems corrupted!");
            break;
        }
    }
    
    Log("Total %d instructions in iringbuf", count);
    Log("========================================================================");
    return ;
}

