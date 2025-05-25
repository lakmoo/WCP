/** Lakshya Selvakumar 12202210
 * Je déclare qu'il s'agit de mon propre travail.
 * Ce travail a été réalisé intégralement par un être humain.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "comptine_utils.h"

#define BUFSIZE 1024

/** Lit des octets dans le fichier de descripteur fd et les met dans buf
* jusqu'au prochain '\n' rencontré, y compris.
* retourne : le nombre d'octets lus avant le premier '\n' rencontré
* précondition : buf doit être assez grand pour contenir les octets lus
* jusqu'au premier caractère '\n' y compris. */
int read_until_nl(int fd, char *buf)
{
	int i = 0;

	while (read(fd, buf + i, 1) == 1) {
		if (buf[i] == '\n')
			return i;
		i++;
	}
	return i;
}

/** Retourne 1 si nom_fich (chaîne terminée par un '\0') se termine par
* .cpt et 0 sinon. */
int est_nom_fichier_comptine(char *nom_fich)
{
	size_t longueur = strlen(nom_fich);
	if (longueur < 4) return 0;
	if (
		nom_fich[longueur - 4] == '.' && 
		nom_fich[longueur - 3] == 'c' && 
		nom_fich[longueur - 2] == 'p' && 
		nom_fich[longueur - 1] == 't' )
			return 1;
	return 0;
}

/** Alloue sur le tas et initialise une struct comptine avec le fichier
* de format comptine (extension ".cpt") de nom de base base_name situé dans
* le répertoire dir_name. Le titre et le nom de fichier de la comptine
* retournée sont eux-mêmes alloués sur le tas pour contenir :
* - la première ligne du fichier, avec son '\n', suivi de '\0'
* - une copie de la chaine base_name avec son '\0'
* Retourne : l'adresse de la struct comptine nouvellement créée ou bien
* NULL en cas d'erreur */
struct comptine *init_cpt_depuis_fichier(const char *dir_name, const char *base_name)
{
	int fd;
	/* Créer une variable "chemin" qui aura le chemin vers la comptine */
	char chemin[strlen(dir_name) + strlen(base_name) + 2];
	strcpy(chemin, dir_name);
	strcat(chemin, "/");
	strcat(chemin, base_name);

	/* Vérifier que le fichier demandé est bien ouvert */
	if ((fd = open(chemin, O_RDONLY)) == - 1) {
		perror("open");
		return NULL;
	}
	
	/* Allouer sur le tas l'espace necassaire pour la comptine */
	struct comptine *ctn = malloc(sizeof(struct comptine));
	if (ctn == NULL) {
		printf("Impossible d'allouer la mémoire\n");
		close(fd);
		return NULL;
	}

	/* Allouer de la mémoire pour le titre de la comptine */
	char buf[BUFSIZE];
	int size = read_until_nl(fd, buf);
	buf[size] = '\n';
	buf[size + 1] = '\0';
	ctn->titre = (char *) malloc(sizeof(char) * (size + 2));
	if (ctn->titre == NULL) {
		printf("Impossible d'allouer la mémoire\n");
		close(fd);
		return NULL;
	}
	strcpy(ctn->titre, buf);

	/* Allouer de la mémoire pour stocker dans nom_fichier une copine de la chaîne base_name + "\0" */
	ctn->nom_fichier = (char *) malloc(sizeof(char) * (strlen(base_name) + 1));
	if (ctn->nom_fichier == NULL) {
		printf("Impossible d'allouer la mémoire\n");
		close(fd);
		return NULL;
	}
	strcpy(ctn->nom_fichier, base_name);

	close(fd);
	return ctn;
}

/** Libère toute la mémoire associée au pointeur de comptine cpt */
void liberer_comptine(struct comptine *cpt)
{
	free(cpt->titre);
	free(cpt->nom_fichier);
	free(cpt);
}

/** Alloue sur le tas un nouveau catalogue de comptines en lisant les fichiers
* de format comptine (ceux dont le nom se termine par ".cpt") contenus dans le répertoire de nom dir_name.
* retourne : un pointeur vers une struct catalogue dont :
* - nb est le nombre de fichiers comptine dans dir_name
* - tab est un tableau de nb pointeurs de comptines, avec pour chacunes
* + nom_fichier égal au nom de base du fichier comptine correspondant
* + titre égal à la première ligne du fichier
* retourne NULL en cas d'erreur. */
struct catalogue *creer_catalogue(const char *dir_name)
{
	/* Acceder au repertoire */
	DIR* dir = opendir(dir_name);
	if (dir == NULL) {
		perror("opendir");
		return NULL;
	}
	

	/* Compter le nombre de fichiers comptines dans dir_name */
	int n_cpt = 0;
	while (readdir(dir)) 
		n_cpt++;
	rewinddir(dir);

	/* Allouer la mémire pour le catalogue */
	struct catalogue* cat = malloc(sizeof(struct catalogue));
	if (cat == NULL) {
		printf("Impossible d'allouer la mémoire\n");
		closedir(dir);
		return NULL;
	}
	cat->nb = n_cpt - 2; /* On retire . et .. */ 
	/*On alloue de la mémoire pour les comptines dans le catalogue */
	cat->tab = (struct comptine **) malloc(sizeof(struct comptine*) * n_cpt);
	if (cat->tab == NULL) {
		printf("Impossible d'allouer la mémoire\n");
		closedir(dir);
		return NULL;
	}

	/*On initialise chaque comptins contenue dans le repertoire dir_name*/
	int i = 0;
	struct dirent* tmp;
	while ((tmp = readdir(dir))) {
		if (strcmp(tmp->d_name, ".") != 0 && strcmp(tmp->d_name, "..") != 0) {
			cat->tab[i++] = init_cpt_depuis_fichier(dir_name, tmp->d_name);
			if (cat->tab[i - 1] == NULL) {
				perror("readdir");
				cat->nb = i;
				liberer_catalogue(cat);
				closedir(dir);
				return NULL;
			}
		}
	}

	closedir(dir);
	return cat;
}

/** Libère toutes les ressources associées à l'adresse c et c lui-même */
void liberer_catalogue(struct catalogue *c)
{
	int i;
	for (i = 0; i < c->nb; i++) {
		liberer_comptine(c->tab[i]);
	}
	free(c->tab);
	free(c);
}
