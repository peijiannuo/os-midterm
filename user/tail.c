#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user.h"

char buf[512];
int nflag, cflag, pflag;

void 
tail(int fd, int num)
{
    // total_char is the total number of chars in file/pipe
    // total_line is the total number of lines in file/pipe
    // skip is the number of lines/chars to skip
    // curr is the counter of passed lines/chars
    // fd_temp is a file descriptor of temp file 
    
    int total_char, total_line, skip, curr;
    int fd_temp, n;

    total_char = total_line = skip = curr = 0;

    // read from file/pipe stream
    // store them into a temp file
    fd_temp = open("temp", O_CREATE | O_WRONLY);
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        write(fd_temp, buf, n);
        for (int i = 0; i < n; i++) {
            total_char++;
            if (buf[i] == '\n')
                total_line++;
        }
    }

    // read error
    if (n < 0) {
        printf(1, "tail: read error\n");
        exit();
    }

    total_line++;
    close(fd_temp);

    // determine the number of line/char to be skipped
    if (nflag) {
        if (total_line >= num && !pflag)
            skip = total_line - num;
        else if (total_line >= num && pflag)
            skip = num - 1;
        else if (total_line < num && pflag)
            skip = total_line;
    }
    
    if (cflag) {
        if (total_char >= num && !pflag)
            skip = total_char - num;
        else if (total_char >= num && pflag)
            skip = num - 1;
        else if (total_char < num && pflag)
            skip = total_char;
    }

    // read from the temp file and print
    fd_temp = open("temp", O_RDONLY);
    while ((n = read(fd_temp, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            if (curr >= skip)
                printf(1, "%c", buf[i]);
            if (buf[i] == '\n' && nflag)
                curr++;
            if (cflag)
                curr++;
        }
    }

    // read error
    if (n < 0) {
        printf(1, "tail: read error\n");
        exit();
    }

    close(fd_temp);

    // delete temp file
    unlink("temp");
    exit();
}

int 
main(int argc, char *argv[]) 
{
    int fd;
    
    // read from pipe
    if (argc == 1) {
        nflag = 1;
        tail(0, 10);
    }

    // read from file
    else if (argc == 2) {
        
        // open error
        if ((fd = open(argv[1], 0)) < 0) {
            printf(1, "tail: cannot open %s\n", argv[1]);
            exit();
        }

        nflag = 1;
        tail(fd, 10);
        close(fd);
    }

    // read from file with option
    else if (argc == 4) {

        // open error
        if ((fd = open(argv[3], 0)) < 0) {
            printf(1, "tail: cannot open %s\n", argv[3]);
            exit();
        }

        // second argument is -n
        if (argv[1][1] == 'n')
            nflag = 1;
        
        // second argument is -c
        else if (argv[1][1] == 'c')
            cflag = 1;
        
        // second argument is other value
        else {
            printf(1, "tail: invalid option\n");
            exit();
        }

        // third argument with +
        if (argv[2][0] == '+') {
            pflag = 1;
            tail(fd, atoi(argv[2] + 1));
        }

        // third argument without +
        else {
            pflag = 0;
            tail(fd, atoi(argv[2]));
        }

        close(fd);
    }

    // invalid argument number
    else {
        printf(1, "tail: invalid argument\n");
        exit();
    }

    exit();
}
