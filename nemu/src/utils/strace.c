

// void strace_log(Context *c){
//     uintptr_t a[4];
//     a[0] = c->GPR1;
//     a[1] = c->GPR2;
//     a[2] = c->GPR3;
//     a[3] = c->GPR4;
//     switch (a[0]) {
//         case EVENT_YIELD:
//             Log("strace log[EVENT_YIELD]: syscall ID = %d", a[0]);
//             break;
//         case EVENT_SYSCALL:
//             Log("strace log[EVENT_SYSCALL]: syscall ID = %d", a[0]);
//             // print file name
//             Log("strace log[EVENT_SYSCALL]: fd = %d file name = %s", a[1], file_table[a[1]]);
//             break;
//         default: 
//             Log("Unhandled syscall ID = %d", a[0]);
//     }

//     return ;
// }