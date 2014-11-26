#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define BLACK 0
#define WHITE 255


int height, width;
int input[128][128];

int rawData[256][8][8];
int vpos[256][3];   // minleft, width, maxright
int exists[256];      // 0 = char does not exist
int firstchar=0xffff,lastchar=0;
char *infile;



void readPicture (char* fileName) { 
    FILE* f;
    char s[60];
    char c;
    int i,j; 
    int maxgrey;

    printf("\nReading picture  %s... \n",fileName);
    f = fopen(fileName,"r");  

    fscanf(f,"%s", s);  // pgm descriptor
    
    fscanf(f,"%c",&c);  // newline
    
    fscanf(f,"%c",&c);  // sharp or number
    if (c == '#') {
        do {
            fscanf(f,"%c",&c);
        } while (c != '\n');  // skip line
    }
    
    fscanf(f,"%d %d", &width, &height);

    if (width != 128 || height != 128) {
      printf("\nError: picture dimensions do not match 128*128 pixels");
    }

    fscanf(f,"%d\n",&maxgrey); 

    for (j = 0; j < 128; ++j) {
        for (i = 0; i < 128; ++i) {
            fscanf(f,"%d ", &(input[i][j])); 
        }
    }

    fclose (f);
}


void dumpCFont (char* fileName) {
    
    int i, j; 
    FILE* f;

    printf("\nDumping progmem arrays as '%s'...\n", fileName);

    f = fopen(fileName,"w");
    
    fprintf(f, "//font.h  -  generated using fonconv; input file was %s",infile);
    
    // boundaries etc
    fprintf(f, "\n#define firstchar %d",firstchar);
    fprintf(f, "\n#define lastchar %d", lastchar);
    
    // font arr
    fprintf(f, "\nconst uint8_t font[] PROGMEM = {\n");
    
    
    int posarr[255];    // begin positions of char
    
    int c,k,l,pos,n=0;
    for (c=0; c<255; ++c) {
        if (exists[c]) {
            n++;
            
            fprintf(f,"\n// %d\t%c\t%d",c,(char)c,pos, vpos[c][0],vpos[c][2]);
            
            for (k = vpos[c][0]; k <= vpos[c][2]; k++) {
                int b=0;
                fprintf(f,"\n0b");
                for (l = 7; l >= 0; --l) {
                    if (rawData[c][l][k] == 0) {
                        b |= (1<<l);
                        fprintf(f,"1");
                    } else { fprintf(f,"0"); }
                }
                
                if (! (c == lastchar && k == vpos[c][2]) ) { fprintf(f,","); } // ommit last comma
            }
            
            posarr[c] = vpos[c][1];
        } else {
            posarr[c] = 0;
        }
        
        pos += vpos[c][1];
    }
    
    fprintf(f, "};\n");
    
    
    // fontpos
    fprintf(f, "\nconst int8_t widths[] PROGMEM = { ");
    
    for (c=firstchar; c<=lastchar; c+=2) {
        fprintf(f, "%d", ( ( (uint)posarr[c] & 0b1111) << 4) | ( (uint)posarr[c+1] & 0b1111)) ;
//        fprintf(f, "%d", (posarr[c] << 4) | posarr[c+1]);
        if (c != lastchar) { fprintf(f, ","); }
    }
        
    fprintf(f, "};\n");
    
    
    // stats    
    int widthmem = (lastchar-firstchar+1)>>1;
    int fontmem = pos;
    printf("mem usage for %d characters:\n\t%d byte fontdata\n\t%d byte width data\n\t----\nsum =\t%d bytes",n,fontmem,widthmem, fontmem+widthmem);
};

void splitraw() {
    
    int i,j,k,l,c=0;
    
    // for each 8x8 pixel char block in each direction...
    for (j = 0; j < 16; ++j) {
        for (i = 0; i < 16; ++i) {
            
            // min left and max right for width calc
            int maxright=0, minleft=7;
            
            // for each row of character data
            for (l = 0; l < 8; ++l) {
                int firstbit=7, lastbit=0;
                
                for (k = 0; k < 8; ++k) {
                    if (input[8*j+k][8*i+l] == 0) {    // pixel is set
                        rawData[c][l][k] = 0;
                        if (k < firstbit) { firstbit = k; }  // determine ..
                        if (k > lastbit) { lastbit = k; }    // ..boundaries
                    } else {
                        rawData[c][l][k] = 1;
                    }
                }
                
                if (firstbit < minleft) { minleft = firstbit; }
                if (lastbit > maxright) { maxright = lastbit; }
            }
            
            // char width
            int charwidth = maxright-minleft+1;
            
            exists[c]=1;
            
            // emtpty char..
            if (minleft > maxright) { charwidth =0; minleft=0; maxright=0; exists[c]=0;}
            
            //printf("%c (%d)\n",(char)c,c);
            vpos[c][0] = minleft;
            //printf("minleft: %d  ",minleft);
            vpos[c][1] = charwidth;
            //printf("width: %d  ",charwidth);
            vpos[c][2] = maxright;
           // printf("maxright: %d  \n",maxright);
            
            if (exists[c]) { 
                lastchar = c; 
                if (c < firstchar) { firstchar = c; }
            }
            
            c++;
        }
    }
}


int main (int argc, char *argv[]) {
    printf("\nfontconvert: converter for 128x128 pixel font (8x8 pixel per char, ascii table layout w/ 8x8 chars) to progmem c array\n\n");
    if (argc < 3) { 
        printf("usage:\n\tfontconvert font.pgm font8.h\n\n");
        exit(1); 
    }
    infile = argv[1];
    readPicture (argv[1]);
    splitraw();
    dumpCFont(argv[2]);
    
    printf("\n");
    return 0;
}
