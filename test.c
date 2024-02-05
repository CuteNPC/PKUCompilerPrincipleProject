#include <stdlib.h>
#include <stdio.h>
char H[] = "M"
           "Thd"
           "\0"
           "\0"
           "\0"
           "\6"
           "\0\0"
           "\0\1"
           "\1"
           "\x80"
           "MTrk",
     E[] = "\0\xff/\0",
     N[] = "GEG>GEG"
           "JG"
           "J"
           "LJLC@>@CECEGEGEG"
           ">GEGJGJLJLC@>@CE"
           "C"
           "E"
           "GCCCLHCL"
           "JGCE"
           "EGEC"
           "GCCLHCLJ"
           "G"
           "G"
           "EECEGE"
           ">>GC"
           ">JGC"
           "GJJJLJ"
           "H"
           ">"
           ">GE"
           "@CGEGEE"
           "CEGCCCL"
           "CLJ"
           "C"
           "C"
           "LHLJ>>GJE@"
           "CEGJCCLCLJC"
           "C"
           "LH"
           "LJ>>GJ"
           "E@"
           "CEGC",
     D[] = "61"
           "186116"
           "1161"
           "181111"
           "1"
           "11"
           "18611"
           "8611611611"
           "81111"
           "1"
           "11"
           "16114"
           "21143112131611421143"
           "11213"
           "1"
           "61"
           "1421"
           "14311"
           "2131"
           "61142"
           "1143"
           "1"
           "1"
           "2131"
           "611611"
           "6116"
           "116113"
           "1211"
           "3"
           "1"
           ""
           "6116"
           "1161"
           "1611"
           ""
           "6"
           "1"
           "1312"
           "11448";
int T[] = {00,  16513, 131, 16516, 134,  16519, 137,  16522, 140};
int 
main()
{
    FILE *_, *f = fopen(""
                        "a."
                        "mid",
                        "wb");
    6;
    fwrite(H, 1, 18, f + 0);
    ;
    fwrite((&"\0\0\5J"), 4, 1, f);
    for (int i = 0, d; i < 150; i++)
        d = D[i] - 48, fwrite(&(char[]){0, 144, N[i], 90, T[d], T[d] >> 8, 144, N[i], 0}, 1, 9, f);
    fwrite(E, 1, 4, f);
    fclose(f);
    /* Play `Yanyuan Qing' @@*/ system("a.mid");
    return 0;
} // Guyutongxue,21/10/23