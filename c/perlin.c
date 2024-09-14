#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#define WIDTH 200
#define HEIGHT 100
#define FRAMES 200

int p[512];

// to calculate the hashes for each corner of the unit cell
int permutation[256] = {151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

// smoothness the noise values as it reaches the exteams (0, 1)
// slope at 0 or 1 = 0
double fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// to linearly interpolate between l, h
// w [0.0, 1.0]
double lerp(double w, double l, double h) {
    return l + w * (h - l);
}

// dot product between the random chosen gradient vector (xi, yj, zk)
// and the distance vector 
double grad(int hash, double x, double y, double z) {

    int h = hash & 0xf;
    double u = h < 8 ? x : y, 
           v = h < 4 ? y : h == 12 || h == 14 ? x : z;

    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double noise(double x, double y, double z) {
    int X = (int)floor(x) & 255,
        Y = (int)floor(y) & 255,
        Z = (int)floor(z) & 255;
    x -= floor(x);
    y -= floor(y);
    z -= floor(z);
    double u = fade(x),
           v = fade(y),
           w = fade(z);
    int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,
        B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;

    return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),
                                   grad(p[BA  ], x-1, y  , z   )),
                           lerp(u, grad(p[AB  ], x  , y-1, z   ),
                                   grad(p[BB  ], x-1, y-1, z   ))),
                   lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),
                                   grad(p[BA+1], x-1, y  , z-1 )),
                           lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
                                   grad(p[BB+1], x-1, y-1, z-1 ))));
}


void init_perlin() {
    // int permutation[256];
    // for (int i = 0; i < 256; i++) {
    //     permutation[i] = i;
    // }

    srand(time(NULL));

    for (int i = 255; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = permutation[i];
        permutation[i] = permutation[j];
        permutation[j] = temp;
    }
    for (int i = 0; i < 512; i++) {
        p[i] = permutation[i & 255];
    }
}

void generate_noise_frame(double noise_map[HEIGHT][WIDTH], 
                          double scale, double z) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            double dx = (double)x / WIDTH;
            double dy = (double)y / HEIGHT;
            noise_map[y][x] = noise(dx * scale, dy * scale, z);
        }
    }
}

void print_noise_frame(double noise_map[HEIGHT][WIDTH]) {
    // printf("\033[H");  // Move cursor to home position
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            double value = noise_map[y][x];
            if (value < -0.2) printf(" ");
            else if (value < 0) printf(".");
            else if (value < 0.2) printf(":");
            else if (value < 0.4) printf("-");
            else if (value < 0.6) printf("=");
            else if (value < 0.8) printf("+");
            else printf("#");
        }
        printf("\n");
    }
    fflush(stdout);
}

int main() {
    init_perlin();
    double noise_map[HEIGHT][WIDTH];
    double scale = 7.0;  

    printf("\033[2J");  // Clear the screen
    printf("\033[?25l");  // Hide the cursor

    for (int frame = 0; frame < FRAMES; frame++) {
        double z = (double)frame / 30.0;  // Change this divisor to adjust speed
        generate_noise_frame(noise_map, scale, z);
        print_noise_frame(noise_map);
        usleep(50000);  // Sleep for 50ms (20 fps)
    }

    printf("\033[?25h");  // Show the cursor
    return 0;
}
