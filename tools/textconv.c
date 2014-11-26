#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>

#define AUTHORS "Manuel Jerger"
#define VERSION "0.1"

#define TRUE ~(uint)0
#define FALSE (uint)0


#define MSG_SEP         0x00
#define END_OF_MEMORY   0x01

#define SPEED1          0x02
#define SPEED2          0x03
#define SPEED3          0x04
#define SPEED4          0x05
#define SPEED5          0x06
#define SPEED6          0x07
#define SPEED7          0x08
#define SPEED8          0x09

#define INVERT          0x0A
#define HALT            0x0B


#define WAIT1           0x0E
#define WAIT2           0x0F
#define WAIT3           0x10
#define WAIT4           0x11
#define WAIT5           0x12
#define WAIT6           0x13
#define WAIT7           0x14
#define WAIT8           0x15

#define PICTURE1        0x16
#define PICTURE2        0x17
#define PICTURE3        0x18
#define PICTURE4        0x19
#define PICTURE5        0x1A
#define PICTURE6        0x1B
#define PICTURE7        0x1C
#define PICTURE8        0x1D
#define SPACER1         0x1E
#define SPACER2         0x1F
#define SPACE           0x20


void readPicture (void);
void readFile (void);
void readStdin (void);

void info(char *);
void err(char *);


char *program_name = "textconv";


char *input, *output, *picture;

int quiet = TRUE, usehex = FALSE, useEE = FALSE;

int tresh=127;             // pixel val treshold
int picdata[64][8];        // raw picture data
int inb[2048];             // utf8-free input (only commands left)
int inpsize;               // length of valid input
int outb[128];             // resulting eeprom hex (0xFF max)

int picpos=0;              // number of actually used eeprom pictures
int lastpos=127;		   // last eeprom pos
int maxmem=127;            // max mem pos we can write text & commands, without pictures (at least 1x EOM at end)
int picid[8] = {-1,-1,-1,-1,-1,-1,-1,-1};       // mapping of pic IDs in input -> IDs in eeprom
int separated = 0;           // true if last character was a \n -> message separator

////////////////////////////////////////////////////////////////////////


// checks if inb has n valid chars starting at position p
int has(int p, int n) {}

// convert input to output
int convert() {
    int ip=0, op=0;
    
    if (useEE) {            // for direct eeprom flashing : two dummy bytes
        outb[op++] = 0x20;
        outb[op++] = 0x20;
    }
    
    int eflag = FALSE;      // for escape char detection

    while (ip < inpsize) {

        if (op == maxmem) {
            err("no more memory left in device for textdata");
            exit (EXIT_FAILURE);
        }

        int c = inb[ip];

        // if escape chr was last
        if (eflag) {
            eflag = FALSE;

            // its just an escaped backslash..
            if (c == '\\') {
                outb[op++] = c;
                ++ip;
            // read whole command, three chars incl backslash
            } else {
                ++ip;
                int num = -1; // for numbered commands
                char nc;
                // check if next char is a number
                if (ip+1 < inpsize) { 
                    nc = inb[ip];
                    
                    if (nc-0x30 >= 0 && nc-0x30 <= 0x09) { 
                        num = nc-0x30;
                    }
                }
                if (c == 'I') {             // invert
                    outb[op++] = INVERT;
                    
                } else if (c == 'H') {      // halt
                    outb[op++] = HALT;
                    
                } else {
                    ++ip;
                    int fail = FALSE;
                    
                        
                    if (num >= 1) {
                        switch (c) {
                            case 'S' : case 's' :     // 8 speeds
                                if (num <= 8) { outb[op++] = num-1 + SPEED1; }
                                else { fail = TRUE; }
                                break;
                            case 'W' : case 'w' :     // 8 waits
                                if (num <= 8) { outb[op++] = num-1 + WAIT1; }
                                else { fail = TRUE; }
                                break;
                            case 'P' : case 'p' :     // 8 pictures
                                if (num <= 8) { outb[op++] = convertPicture (num-1) + PICTURE1; }
                                else { fail = TRUE; }
                                break;
                            case 'D' : case 'd' :     // 2 spacers
                                if (num <= 2) { outb[op++] = num-1 + SPACER1; }
                                else { fail = TRUE; }
                                break;
                        }
                    }
                    
                }
                
            }

            
        // no eflag set
        } else {
        
            
            // read on only char : 

            // newline = msg sep (onlye once, swallow empty lines)
            if (c == '\n') {
                if (separated) { 
                    separated = 0; 
                } else {
                    outb[op++] = 0x00; 
                    separated = 1; 
                }
                
            } else {
                separated = 0;    

                // escape character '\' special handling
                if (c == '\\') {
                    eflag = TRUE;   // if next char is also a '\', it gets copied
                } else if (c < SPACE) {
                    
                    // ???????

                // ascii 127, printable characters + converted utf8 chars
                } else if (c >= SPACE) {
                    outb[op++] = c;
                }
            }
            
            ++ip;
        }
    }
    
    // append end-of-message
    //if (op > 0) {
        if (outb[op-1] == 0x00) {
            outb[op-1] = 0x01;        // replace last MSG Sep. by EOM
        } else {
            outb[op++] = 0x01;      // additional EOM
        }
    //}
        

    return op;
}


 
////////////////////////////////////////////////////////////////////////

int main (int argc, char **argv)
{

    program_name = argv[0];
    
        //setlocale (LC_ALL, "");
        //bindtextdomain (PACKAGE, LOCALEDIR);
        //textdomain (PACKAGE);

        //atexit (close_stdout);
        


    // while args left..
    int a = 1; int last_a = 1;
    do {
        last_a = a;
        // two args
        if (a+1 < argc) {

            // infile
            if (!strcmp (argv[a], "-i")) {
                input = argv[a+1];
                a += 2;

            // eeprom pictures (8x8 pixel pgm)
            } else if (!strcmp (argv[a], "-p")) {
                picture = argv[a+1];
                a += 2;

            // outfile
            } else if (!strcmp (argv[a], "-o")) {
                output = argv[a+1];
                a += 2;
            }
            
        } 
        
        // one arg (a may have changed)
        if (a < argc) {

            // use readable hex format as output
            if (!strcmp (argv[a], "--hex")) {
                usehex = TRUE;
                a++;
            
            // prepend two dummy bytes
            } else if (!strcmp (argv[a], "--ee")) {
                useEE = TRUE;
                a++;
                        
            // verbose
            } else if (!strcmp (argv[a], "--verbose")) {
                quiet = FALSE;
                a++;

                
            } else if (!strcmp (argv[a], "--help")) {
                fprintf (stdout, "\nConvert cleartext commands and readable characters to blinken64 eeprom format.\n");
                fprintf (stdout, "\nUsage: %s [-i infile] [-o outfile] [--hex] [--verbose]\n", program_name);
                fprintf (stdout, "\n    -i infile        read clear text from file");
                fprintf (stdout, "\n    -o outfile       dump output also to file");
                fprintf (stdout, "\n   --hex             use readable hex format as output");
                fprintf (stdout, "\n   --ee              output for flashing eeprom directly (adds two dummy bytes at the beginning)");
                fprintf (stdout, "\n   --verbose         print err/log on stdout\n\n");

                exit (EXIT_SUCCESS);
            }
        }

    } while ( (a < argc) && (a != last_a) );
    
	// direct eeprom flash: picture positions moved left by two
    if (useEE) { lastpos = 128; } else { lastpos = 126; } 
    
    // load picture data
    if (picture != NULL ) { readPicture(); }
    
    // read from file
    if (input != NULL) { 
		readFile();
    
    // read from stdin
    } else {
        // TODO
    }
    
    // start converting
    int len = convert();
    // if memory contains pictures, dump the whole range
    //if (picpos > 0) {len = 127;}
                
  

    // dump result to file
    if (output != NULL) {
        FILE* f;

        info ("dumping output to file");

        f = fopen(output,"w");
        
        int p;
        if (usehex) {
            for (p=0; p<lastpos; p++) { fprintf(f,"0x%1x,",outb[p]);  }
        } else {
            for (p=0; p<lastpos; p++) { fprintf(f,"%c",(char)outb[p]);  }
        }
        fclose(f);
    }
    
    info ("Done\nresult:\n\n");
    
    // print to stdout
    int p;
    
    if (usehex) {
        for (p=0; p<lastpos; p++) { fprintf(stdout,"0x%1x,",outb[p]);  }
    } else {
        for (p=0; p<lastpos; p++) { fprintf(stdout,"%c",(char)outb[p]);  }
    }
    fprintf(stdout,"\n\n");
    
    exit (EXIT_SUCCESS);
}



void readStdin () {
    
}



// process unicode low level (we need only a few special chars for german text)
void readFile () {
    
    FILE *f;
    int pos=0;
    
    info ("reading input utf8 file");
    
    f = fopen(input,"r");
  
    int ic;
    do {
        ic = fgetc(f);
    
        
        // ascii, utf8 compatible
        if (ic < 128) { 
            inb[pos++] = ic;
            
        // utf8
        } else {
            int fail = FALSE;
            
            if (ic == 195) {
                ic = fgetc(f);
                switch (ic) {
                    case 132 : inb[pos++] = 129; break; //Ä
                    case 150 : case 182 :  inb[pos++] = 130; break;//Ö//ö
                    case 156 : case 188 :  inb[pos++] = 131; break;//Ü //ü
                    case 159 : inb[pos++] = 132; break; //ß
                    case 164 : inb[pos++] = 133; break; //ä
                    default : fail = TRUE;
                }
            } else if (ic == 226) {
                ic = fgetc(f);
                if (ic == 130) {
                    ic = fgetc(f);
                    if (ic == 172) {        // €
                        inb[pos++] = 128;
                        
                    } else { fail = TRUE; }
                } else { fail = TRUE; }
             }
             
             if (fail) { err ("unknow char in UTF range"); }
        }
    } while (ic != EOF);
    
    inpsize = pos;
    
    fclose(f);
    
}

void info (char * str) {
    if (!quiet) {
        fprintf(stdout,"\n%s",str);
    }
}

void err (char * str) {
    if (!quiet) {
        fprintf(stdout, "\nfail : %s",str);
    }
}


////////////////////////////////////////////////////////////////////////

void readPicture () {
    FILE* f;
    char s[60];
    char c;
    int x,y;
    int width, height, maxgrey;
    
    info ("reading input picture");
    f = fopen(picture,"r");

    fscanf(f,"%s", s);  // pgm descriptor

    fscanf(f,"%c",&c);  // newline

    fscanf(f,"%c",&c);  // sharp or number
    if (c == '#') {
        do {
            fscanf(f,"%c",&c);
        } while (c != '\n');  // skip line
    }

    fscanf(f,"%d %d", &width, &height);

    if (width != 64 || height != 8) {
      err("picture dimensions do not match w*h = 64*8 pixels");
      fclose (f);
      exit(EXIT_FAILURE);
    }

    fscanf(f,"%d\n",&maxgrey);
    tresh = maxgrey/2;

    for (y = 0; y < 8; ++y) {
        for (x = 0; x < 64; ++x) {
            fscanf(f,"%d ", &(picdata[x][y]));
        }
    }

    fclose (f);
}



// converts slice of loaded image to one eeprom picture (#idx) and places result at corresponding position at the end of memory
// decrements maxmem, returns real pic id
int convertPicture (int idx) {
    
    // pic is not used yet
    if (picid[idx] == -1) {    
        picid[idx] = picpos;

        // copy data to eeprom
        int i,b;
        for (i = 0; i < 8; ++i) {

            int col = 0;
            for (b = 0; b < 8; ++b) {
                if (picdata[idx*8+i][b] > tresh) {
                    col |= (1<<b);
                }
            }
            
			outb[lastpos - picpos*8 - 8 + i] = col;
            
        }
        
        maxmem -= 8;
        picpos++;
    }
    
    // return the actual used pic-ID used in eeprom
    return picid[idx];  
}
