#include <stdio.h>
#include <time.h>
#include <string.h>
#include "dirent.h"
#define _POSIX_SOURCE
#include <fcntl.h>
#include <sys/types.h>
#include "unistd.h"
#undef _POSIX_SOURCE

unsigned short mem[65536] = { 0x0000 }, regs[16], ni; // regs[15] = PC
int fileModes[16], openModes[16], lseekpos[16], fd;

void init() {
    int i = 0;
    for (; i < 16; i++) {
        regs[i] = 0;
    }
    for (i = 0; i < 65536; i++) {
        mem[i] = 0;
    }

    openModes[0] = O_RDONLY;
    openModes[1] = O_WRONLY;
    openModes[2] = O_RDWR;
    openModes[3] = O_APPEND;
    openModes[4] = O_CREAT;
    openModes[5] = O_TRUNC;
    openModes[6] = O_EXCL;
    for (i = 7; i < 16; i++) {
        openModes[i] = 0;
    }

    fileModes[0] = S_IXUSR;
    fileModes[1] = S_IWUSR; 
    fileModes[2] = S_IRUSR; 
    fileModes[3] = S_IXUSR | S_IWUSR | S_IRUSR; 
    for (i = 3; i < 16; i++) {
        fileModes[i] = 0x100; 
    }

    lseekpos[0] = SEEK_SET;
    lseekpos[1] = SEEK_CUR;
    lseekpos[2] = SEEK_END;
    for (i = 3; i < 16; i++) {
        lseekpos[i] = 0;
    }
}

void loadInstructions() {
    FILE* fp = fopen("MegaRo.bin", "r");
    unsigned short instr = 0;
    while (fread(&instr, sizeof(short), 1, fp) > 0) {
        mem[ni] = instr;
        ni++;
    }
}

int strsize(char* str) {
    int i = 0;
    while (str[i] != '\0') i++;
    return i;
}

char* readMem(int address, int nob, char* naziv) {
    char c;
    int b = 0, i = 0;

    if (nob) {
        // Bytes read
        while (1) {
            unsigned short var = mem[address];
            c = (char)((mem[address] & 0xFF00) >> 8); b++;
            strcat(naziv, &c);
            naziv[b] = '\0';
            if (b > nob) {
                break;
            }

            c = (char)(mem[address] & 0x00FF);
            strcat(naziv, &c); b++;
            naziv[b] = '\0';
            if (b > nob) {
                break;
            }

            address++;
        }
    }
    else {
        // String read
        while (1) {
            unsigned short var = mem[address];
            c = (char)((mem[address] & 0xFF00) >> 8); b++;
            strcat(naziv, &c);
            naziv[b] = '\0';
            if (!c) {
                break;
            }

            c = (char)(mem[address] & 0x00FF); b++;
            strcat(naziv, &c);
            naziv[b] = '\0';
            if (!c) {
                break;
            }

            address++;
        }
    }
    return naziv;
}

void writeMem(int loc, char* strWrite, int nob) {
    unsigned int size = strsize(strWrite);
    if (nob > size) {
        return;
    }
    if (nob) {
        // Bytes write
        for (int i = 0; i < nob; i++, loc++) {
            mem[loc] = (strWrite[i] >> 8);
            if (i + 1 < nob) {
                mem[loc] |= strWrite[i + 1];
            }
        }

        if (nob % 2) {
            mem[loc] &= 0xFF00;
        }
        else {
            mem[loc] = 0;
        }
    }
    else {
        // String write
        for (unsigned int i = 0; i < size; i += 2, loc++) {
            mem[loc] = (strWrite[i] >> 8);
            if (i + 1 < size) {
                mem[loc] |= strWrite[i + 1];
            }
        }

        if (size % 2) {
            mem[loc] &= 0xFF00;
        }
        else {
            mem[loc] = 0;
        }
    }
}

void interpret() {
    int i = 0;
    unsigned short int ins, r, n, k;
    char r1, n1;
    while (i <= ni) {
        ins = mem[regs[15]++];

        switch (ins & 0xF000) {
        case 0x0000:
            regs[(ins >> 8) & 0x000F] = mem[regs[ins & 0x000F]];
            break;
        case 0x1000:
            regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] + regs[ins & 0x000F];
            break;
        case 0x2000:
            regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] - regs[ins & 0x000F];
            break;
        case 0x3000:
            regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] & regs[ins & 0x000F];
            break;
        case 0x4000:
            regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] | regs[ins & 0x000F];
            break;
        case 0x5000:
            regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] ^ regs[ins & 0x000F];
            break;
        case 0x6000:
            r = regs[ins & 0x000F];
            n = (r & 0x000F);
            if (((r >> 4) & 0x0003) == 0) {
                k = (regs[(ins >> 4) & 0x000F] | 0x7FFF);
                if (k == 0xFFFF) regs[(ins >> 8) & 0x000F] = (regs[(ins >> 4) & 0x000F] >> n) & ~(((0x1 << 16) >> n) << 1);
                else regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] >> n;
            }
            else if (((r >> 4) & 0x0003) == 1) regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] >> n;
            else if (((r >> 4) & 0x0003) == 2) regs[(ins >> 8) & 0x000F] = (regs[(ins >> 4) & 0x000F] << n) | (regs[(ins >> 4) & 0x000F] >> (16 - n));
            else regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] << n;
            break;
        case 0x7000:
            regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] * regs[ins & 0x000F];
            break;
        case 0x8000:
            mem[regs[ins & 0x000F]] = regs[(ins >> 4) & 0x000F];
            regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F];
            break;
        case 0x9000:
            k = ins & 0x00FF;
            regs[(ins >> 8) & 0x000F] = k;
            break;
        case 0xA000:
            regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] > regs[ins & 0x000F];
            break;
        case 0xB000:
            r = regs[(ins >> 4) & 0x000F];
            n = regs[ins & 0x000F];
            r1 = !((r & 0x8000) == 0x0000);
            n1 = !((n & 0x8000) == 0x0000);
            if (r1 && !n1) {
                regs[(ins >> 8) & 0x000F] = 0x0001;
            }
            else if (!r1 && n1) {
                regs[(ins >> 8) & 0x000F] = 0x0000;
            }
            else {
                regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] > regs[ins & 0x000F];
            }
            break;
        case 0xC000:
            regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] < regs[ins & 0x000F];
            break;
        case 0xD000:
            r = regs[(ins >> 4) & 0x000F];
            n = regs[ins & 0x000F];
            r1 = !((r & 0x8000) == 0x0000);
            n1 = !((n & 0x8000) == 0x0000);
            if (r1 && !n1) {
                regs[(ins >> 8) & 0x000F] = 0x0000;
            }
            else if (!r1 && n1) {
                regs[(ins >> 8) & 0x000F] = 0x0001;
            }
            else {
                regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] < regs[ins & 0x000F];
            }
            break;
        case 0xE000:
            regs[(ins >> 8) & 0x000F] = regs[(ins >> 4) & 0x000F] == regs[ins & 0x000F];
            break;
        case 0xF000:
            regs[(ins >> 8) & 0x000F] = regs[15];
            regs[15] = ins & 0x000F;
            int ret;
            if ((ins >> 4) & 0x000F == 0x000F) {
                //System calls
                char fn[2 << 10]; fn[0] = '\0';
                char naziv[2 << 15]; naziv[0] = '\0';
                int postoji;
                switch (regs[0] & 0x0007) {

                case 0x0000:
                    strcat(fn, "./storage/"); strcat(fn, readMem(regs[1], 0, naziv));
                    struct stat buff;
                    postoji = stat(fn, &buff);
                    if (postoji == 0) {
                        printf("%s attributes are:\n", fn);
                        printf("Mode: %d\n", buff.st_mode);
                        printf("File ID: %d\n", buff.st_ino);
                        printf("Owner ID: %d\n", buff.st_uid);
                        printf("File size (in bytes): %d\n", buff.st_size);
                        struct tm* tm;
                        char file_modified_time[100];
                        tm = localtime(&buff.st_atime);
                        sprintf(file_modified_time, "%.2d.%.2d.%.2d , %.2d:%.2d:%.2d", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
                        printf("Last access: %s\n", file_modified_time);
                        tm = localtime(&buff.st_mtime);
                        sprintf(file_modified_time, "%.2d.%.2d.%.2d , %.2d:%.2d:%.2d", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
                        printf("Last change: %s\n", file_modified_time);
                    }
                    else {
                        printf("File/Diretory %s not found", fn);
                    }


                    break;
                case 0x0001:
                    strcat(fn, "./storage/"); strcat(fn, readMem(regs[1], 0, naziv));
                    if ((fd = creat(fn, fileModes[regs[2] & 0x000F] | fileModes[(regs[2] >> 4) & 0x000F] | fileModes[(regs[2] >> 8) & 0x000F] | fileModes[(regs[2] >> 12) & 0x000F])) < 0)
                        perror("creat() error\n");
                    else {
                        regs[1] = fd;
                        printf("File %s created\n", fn);
                    }
                    break;

                case 0x0002:
                    strcat(fn, "./storage/"); strcat(fn, readMem(regs[1], 0, naziv));
                    if ((fd = open(fn, openModes[regs[2] & 0x000F] | openModes[(regs[2] >> 4) & 0x000F] | openModes[(regs[2] >> 8) & 0x000F] | openModes[(regs[2] >> 12) & 0x000F]/*, openModes[regs[3] & 0x000F]*/)) < 0)
                        perror("open() error\n");
                    else {
                        regs[1] = fd;
                        printf("File %s opened\n", fn);
                    }
                    break;
                case 0x0003:

                    if (regs[1] != 0) {
                        close(regs[1]);
                        printf("File closed\n");
                    }
                    else {
                        printf("Error closing file, fd=0\n");
                    }
                    break;
                case 0x0004:
                    strcat(fn, "./storage/"); strcat(fn, readMem(regs[1], 0, naziv));
                    if (unlink(fn) != 0)
                        perror("unlink() error\n");
                    else {
                        printf("File %s unlinked\n", fn);
                    }
                    break;
                case 0x0005:
                    if (regs[1] < 0) {
                        printf("No file is opened\n");
                    }

                    else {
                        char buff[1 << 10];
                        if ((ret = read(regs[1], buff, regs[3])) > 0) {
                            buff[ret] = '\0';
                            printf("Succesfully read block: %s\n", buff);
                            writeMem(regs[2], buff, regs[3]);
                        }
                        else {
                            printf("Error while reading block\n");
                        }
                    }
                    break;
                case 0x0006:
                    if (regs[1] < 0) {
                        printf("No file is open\n");
                    }
                    else {
                        char strWrite[1 << 10]; strWrite[0] = '\0';
                        strcat(strWrite, readMem(regs[2], regs[3], naziv));
                        (regs[2], regs[3]);
                        char scopy[1 << 10];

                        int i = 0;
                        for (; i < strsize(strWrite); i++) {
                            scopy[i] = strWrite[i];
                        }
                        scopy[i] = '\0';
                        if ((ret = write(regs[1], scopy, regs[3])) == -1) {
                            printf("Writing to file was unsuccesful\n");
                        }
                        else {
                            printf("Written bytes: %d\n", ret);
                        }
                    }
                    break;
                case 0x0007:
                    if (regs[1] < 0) {
                        printf("No file is open\n");
                    }
                    else {
                        ret = lseek(regs[1], regs[2], lseekpos[regs[3] & 0x000F]);
                        if (ret == -1) {
                            printf("Offset change unsucceful\n");
                        }
                        else {
                            printf("Succesfully changed offset. Current value is %d\n", ret);
                        }
                    }
                    break;

                }

            }
            regs[15] = regs[(ins >> 8) & 0x000F];
            break;
        }
        ++i;
    }

}

int main()
{
    init();
    loadInstructions();
    interpret();

    return 0;
}