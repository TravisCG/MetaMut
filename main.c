#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXMETACIRCLE    40
#define MAXRADIUS        20
#define GENOMSIZE        MAXMETACIRCLE * 3
#define POPULATIONSIZE   100
#define WINNERSIZE       5
#define GENERATIONNUM    100
#define SCREENWIDTH      1280
#define SCREENHEIGHT     720
#define MUTNUM           40

typedef struct _Genome {
	double fitness;
	int genes[GENOMSIZE];
} Genome;

Genome population[POPULATIONSIZE];
Genome winnerpop[WINNERSIZE];

char screen[SCREENWIDTH * SCREENHEIGHT];
char pattern[SCREENWIDTH * SCREENHEIGHT];

void readpbm(char *filename){
	FILE *pbm;
	char line[100];
	int i, c = 0;

	pbm = fopen(filename, "r");
	fgets(line, 100, pbm);
	fgets(line, 100, pbm);
	fgets(line, 100, pbm);
	while(fgets(line, 100, pbm) != NULL){
		for(i = 0; i < strlen(line) - 1; i++, c++){
			if(line[i] == '0') pattern[c] = 0;
			if(line[i] == '1') pattern[c] = 1;
		}
	}
	fclose(pbm);
}

void savepbm(){
	FILE *pbm;
	int i;
	char filename[20];

	sprintf(filename, "%f.pbm", drand48());

	pbm = fopen(filename, "w");
	fprintf(pbm, "P1\n1280 720\n");
	for(i = 0; i < SCREENWIDTH * SCREENHEIGHT; i++){
		fprintf(pbm, "%d", screen[i]);
		if( (i + 1) % 70 == 0){
			fprintf(pbm, "\n");
		}
	}
	fclose(pbm);
}

void fill(Genome *genome){
	int i,j,k, index;
	double sum, inters = 0.0, unio = 0.0;

	#pragma omp parallel for
	for(i = 0; i < SCREENWIDTH; i++){
		for(j = 0; j < SCREENHEIGHT; j++){
			sum = 0.0;
			for(k = 0; k < MAXMETACIRCLE; k++){
				sum += (double)genome->genes[k * 3 + 2] / sqrt( ((i - genome->genes[k * 3])*(i - genome->genes[k * 3])) + ((j - genome->genes[k * 3 + 1])*(j - genome->genes[k * 3 + 1])) );
			}

			index = j * SCREENWIDTH + i;
			if(sum > 1.0){
				screen[index] = 1;
			}
			else{
				screen[index] = 0;
			}

			if(pattern[index] == 1 && screen[index] == 1){
				inters++;
			}
			if(pattern[index] == 1 || screen[index] == 1){
				unio++;
			}
		}
	}

	genome->fitness = inters / unio;
	if(genome->fitness > 0.9){
		printf("inters: %f\nunion: %f\n", inters, unio);
	}
}

/*OBSOLOTE, for optimization purposes, I put the calculation into fill() */
double jaccard(){
	int i, inters = 0, unio = 0;

	for(i = 0; i < SCREENWIDTH * SCREENHEIGHT; i++){
		if(screen[i] == 1 && pattern[i] == 1){
			inters++;
		}
		if(screen[i] == 1 || pattern[i] == 1){
			unio++;
		}
	}

	return((double)inters / (double)unio);
}

void select_best(int gnum){
	int i,minindex,j;
	double min = 1.0;

	for(i = 0; i < POPULATIONSIZE; i++){
		fill(&population[i]);
		if(i == 5){
			savepbm();
		}
		printf("%d\t%f\n", gnum, population[i].fitness); //for some statistics
	}

	for(i = 0; i < WINNERSIZE; i++){
		winnerpop[i] = population[i];
		if(winnerpop[i].fitness < min){
			min = winnerpop[i].fitness;
			minindex = i;
		}
	}

	for(i = WINNERSIZE; i < POPULATIONSIZE; i++){
		if(population[i].fitness > min){
			winnerpop[minindex] = population[i];
			min = 1.0;
			for(j = 0; j < WINNERSIZE; j++){
				if(winnerpop[j].fitness < min){
					min = winnerpop[j].fitness;
					minindex = j;
				}
			}
		}
	}
}

void newpopulation(){
	int i, j, male, female, crossing;

	for(i = 0; i < POPULATIONSIZE; i++){
		female   = (int)(drand48() * WINNERSIZE);
		male     = (int)(drand48() * WINNERSIZE);
		crossing = (int)(drand48() * MAXMETACIRCLE) * 3;
		for(j = 0; j < crossing; j++){
			population[i].genes[j] = winnerpop[female].genes[j];
		}
		for(j = crossing; j < GENOMSIZE; j++){
			population[i].genes[j] = winnerpop[male].genes[j];
		}
	}
}

void mutation(){
	double limit[] = {SCREENWIDTH, SCREENHEIGHT, MAXRADIUS};
	int i, indiv, gene, mutat;

	for(i = 0; i < MUTNUM; i++){
		indiv = (int)(drand48() * POPULATIONSIZE);
		gene  = (int)(drand48() * GENOMSIZE);
		mutat = (int)(drand48() * limit[gene % 3]);
		population[indiv].genes[gene] = mutat;
	}
}

int main(int argc, char **argv){
	int i, j;
	double limit[] = {SCREENWIDTH, SCREENHEIGHT, MAXRADIUS};

	/* Initialization of all genes with random values */
	for(i = 0; i < POPULATIONSIZE; i++){
		for(j = 0; j < GENOMSIZE; j++){
			population[i].genes[j] = (int)(drand48() * limit[j % 3]);
		}
		population[i].fitness = drand48();
	}

	readpbm(argv[1]);

	for(i = 0; i < GENERATIONNUM; i++){
		select_best(i);
		newpopulation();
		mutation();
	}

	return EXIT_SUCCESS;
}
