#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h> //only for windows. Used by Sleep in switch case 'W'

typedef struct {
	char *firstName;	//etunimi
	char *lastName;		//sukunimi
	char *id;			//opiskelijanumero
	int grades[6];		//kierroskohtaiset pisteet
	int total;			//kokonaispisteet
}Student;

typedef struct {
	Student *students;	//lista oppilaista
	int count;			//oppilaiden lukumäärä
}Course;

void addStudent(Course *c, int idx, char *First, char *Last, char *id) {	//funktio lisää listaan uuden opiskelijan ja alustaa kaikki pisteet nollaksi
	
	c->students = realloc(c->students, (idx + 1) * sizeof(Student));		//varaa tilaa isommalle opiskelijalistalle
	Student *s = &(c->students[idx]);										//osoitin lisättävään oppilaaseen
	s->firstName = malloc(strlen(First)+1);
	s->lastName = malloc(strlen(Last) + 1);
	s->id = malloc(strlen(id) + 1);
	//First[strlen(First)] = '\0';
	strcpy(s->firstName, First);
	strcpy(s->lastName, Last);
	//s->lastName[strlen(Last)] = '\0';
	strcpy(s->id, id);
	//s->id[strlen(id)] = '\0';
	s->total = 0;
	for (int i = 0; i < 6; i++)
		s->grades[i] = 0;
	c->count += 1;
	return;
}

int updatePoints(Course *c, int count, char *id, int round, int pts) {		//päivittää oppilaan pisteet, mikäli oppilas löytyy rekisteristä
	int total = 0;

	for (int i = 0; i < count; i++) {

		if (!strcmp(id, c->students[i].id)) {				//etsitään oppilas opiskelijanumeron avulla
			c->students[i].grades[round - 1] = pts;

			for (int j = 0; j < 6; j++) {					//lasketaan uudeet kokonaispisteet
				total += c->students[i].grades[j];
			}
			c->students[i].total = total;					//päivitetään kokonaispisteet
			return 1;
		}
	}
	printf("Student with id %s not found.\n", id);			//tulostetaan virheilmoitus mikäli opiskelijaa ei löydy
	return 0;
}

int readFile(Course *c, char *input) {
	char cmd[2], name[50];
	sscanf(input, "%s %s", cmd, name);			//Tallennetaan tiedostonimi muuttujaan name
	FILE *f = fopen(name, "r");
	if (f == NULL) {
		printf("Error reading file. (Check that file exists)\n");
		return 0;
	}
	printf("%s found.\n", name);
	int n;
	fscanf(f, "%d\n", &n);
	int g[6], tot;		//tähän tallennetaan kierrosten pisteet
	char first[50], last[50], id[50]; //bufferit nimille
	for (int i = 0; i < n; i++) {
		fscanf(f, "%s %s %s %d %d %d %d %d %d %d\n",
			id, last, first, &g[0], &g[1], &g[2], &g[3], &g[4], &g[5], &tot);		//luetaan tiedot muuttujiin oppilas kerrallaan

		addStudent(c, i, first, last, id);			//lisätään oppilas tietokantaan

		for (int j = 0; j < 6; j++)
			updatePoints(c,i+1, id, j+1, g[j]);		//päivitetään pisteet tietokantaan
	}

	fclose(f);
	return 1;
}

void print2file(Course c, char *input) {		//tallennetaan muistissa oleva taulukko käyttäjän nimeämään tiedostoon
	char cmd[2], filename[50];
	int ret = sscanf(input, "%s %s", cmd, filename);
	if (ret < 2)
		return;
	
	FILE *f = fopen(filename, "w");
	fprintf(f, "%d\n", c.count);
	for (int i = 0; i < c.count; i++) {
		Student *s = &(c.students[i]);
		fprintf(f, "%s %s %s %d %d %d %d %d %d %d\n",
			s->id, s->lastName, s->firstName, s->grades[0], s->grades[1],
			s->grades[2], s->grades[3], s->grades[4], s->grades[5], s->total);
	}
	fclose(f);
	printf("Data saved to file: %s\n", filename);
}

void printStudents(Course c) {		//tulostaa taulukon oppilaiden pisteistä 
	if (c.count == 0) {
		printf("Student list empty\n"); //taulukkoa ei tulosteta jos lista tyhjä
		return;
	}
	printf("\n");
	printf("ID       Lastname             Firstname          1   2   3   4   5   6    tot\n");
	printf("-----------------------------------------------------------------------------\n");
	int n = c.count;
	for (int i = 0; i < n; i++) {
		int r[6], tot;
		tot = c.students[i].total;
		for (int j = 0; j < 6; j++)
			r[j] = c.students[i].grades[j];
		printf("%-8s %-20s %-16s %3d %3d %3d %3d %3d %3d  %4d\n",
			c.students[i].id, c.students[i].lastName, c.students[i].firstName,
			r[0], r[1], r[2], r[3], r[4], r[5], tot);
	}
	printf("\n");
	return;
}

int comp_total(const void *a, const void *b) {
	const Student *p1 = (Student*)a;
	const Student *p2 = (Student*)b;
	int r = p2->total - p1->total;
	return r;
}

void freeAll(Course *c) {
	for (int i = 0; i < c->count; i++) {
		free(c->students[i].firstName);
		free(c->students[i].lastName);
		free(c->students[i].id);
	}
	free(c->students);
	return;
}




int main() {
	printf("Course bookkeeping program\n(C) Oskari Makinen 2017\n\n");
	printf("Commands:\n");
	printf("O filename                        :Read file specified by user\n");
	printf("A ID Lastname Firstname           :Add new student to database\n");
	printf("U ID round newpoints              :Update student's points\n");
	printf("L                                 :Print out point table in descending order\n");
	printf("W filename                        :Save current data to file and exit\n");
	printf("W                                 :Exit\n\n>> ");

	Course c;
	c.students = malloc(sizeof(Student)); //alusta opiskelijalista
	c.count = 0;


	char input[200], first[50], last[50], id[50], cmd[2]; //puskureita käyttäjän syötteen tallentamiseen
	int round, points, ret;
	input[0] = '\0';								//valgrind-errorin poistoon
	fgets(input, 199, stdin);

	while (1) {						//read user commands (A U L W O)

		switch (input[0]) {			//tarkistetaan syötteen ensimmäinen kirjain ja toimitaan sen mukaan

		case 'A':					// lisää opiskelija (A OPNRO sukunimi etunimi)
			ret = sscanf(input, "%s %s %s %s\n", cmd, id, last, first);
			if (ret < 4) {
				printf("Too few arguments\n");
				break;
			}
			addStudent(&c, c.count, first, last, id);
			printf("New student added\n");
			break;

		case 'U':					// päivitä pisteet (U OPNRO kierros pisteet)
			ret = sscanf(input, "%s %s %d %d\n", cmd, id, &round, &points);
			if (ret < 4) {
				printf("Too few arguments\n");
				break;
			}
			if (round > 6 || round < 1) {
				printf("Round must be within range 1-6\n");
				break;
			}
			ret = updatePoints(&c, c.count, id, round, points);
			if(ret)
				printf("Updated points for student with ID: %s\n", id);
			break;

		case 'L':					// listaa opiskelijat, korkeimmat kok. pisteet ensin (OPNRO sukunimi etunimi k1 k2 k3 k4 k5 k6 total)
			qsort(c.students, c.count, sizeof(Student), comp_total);
			printStudents(c);
			break;

		case 'W':					// tallenna nykyinen lista tiedostoon (W tiedostonimi) ja poistu ohjelmasta
			print2file(c, input);
			printf("Cleaning up...\n");
			freeAll(&c);
			printf("Exiting program\n");
			Sleep(5000);			//just for fun
			exit(1);
			break;

		case 'O':					//lataa tiedot muistiin (O tiedostonimi) ja korvaa aikaisempi taulukko
			if(c.count>0){
				freeAll(&c);
				c.count = 0;
				c.students = malloc(sizeof(Student));
			}
			ret = readFile(&c, input);
			if(ret)
				printf("Fileread successful\n");
			break;

		default:
			printf("Invalid command\n");
		}

		printf(">> ");
		fgets(input, 199, stdin);
	}
	return 1;
}