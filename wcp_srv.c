/** Lakshya Selvakumar 12202210
 * Je déclare qu'il s'agit de mon propre travail.
 * Ce travail a été réalisé intégralement par un être humain.
 */

/* fichiers de la bibliothèque standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
/* bibliothèque standard unix */
#include <unistd.h> /* close, read, write */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <errno.h>
/* spécifique à internet */
#include <arpa/inet.h> /* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"
#include <pthread.h>

#define PORT_WCP 4321
#define BUFSIZE 1024

void usage(char *nom_prog)
{
	fprintf(stderr, "Usage: %s repertoire_comptines\n"
			"serveur pour WCP (Wikicomptine Protocol)\n"
			"Exemple: %s comptines\n", nom_prog, nom_prog);
}
/** Retourne en cas de succès le descripteur de fichier d'une socket d'écoute
 *  attachée au port port et à toutes les adresses locales. */
int creer_configurer_sock_ecoute(uint16_t port);

/** Écrit dans le fichier de desripteur fd la liste des comptines présents dans
 *  le catalogue c comme spécifié par le protocole WCP, c'est-à-dire sous la
 *  forme de plusieurs lignes terminées par '\n' :
 *  chaque ligne commence par le numéro de la comptine (son indice dans le
 *  catalogue) commençant à 0, écrit en décimal, sur 6 caractères
 *  suivi d'un espace
 *  puis du titre de la comptine
 *  une ligne vide termine le message */
void envoyer_liste(int fd, struct catalogue *c);

/** Lit dans fd un entier sur 2 octets écrit en network byte order
 *  retourne : cet entier en boutisme machine. */
uint16_t recevoir_num_comptine(int fd);

/** Écrit dans fd la comptine numéro ic du catalogue c dont le fichier est situé
 *  dans le répertoire dirname comme spécifié par le protocole WCP, c'est-à-dire :
 *  chaque ligne du fichier de comptine est écrite avec son '\n' final, y
 *  compris son titre, deux lignes vides terminent le message */
void envoyer_comptine(int fd, const char *dirname, struct catalogue *c, uint16_t ic);

/** Cette fonction est exécutée dans un thread.
 * Elle prend en paramètre un pointeur vers une structure `infos` qui contient
 * le socket ainsi qu'un chemin pour traiter la requête du client. La fonction
 * crée un catalogue à partir du chemin, envoie la liste des éléments de 
 * ce catalogue au client, reçoit un numéro de comptine, puis envoie la comptine
 * correspondante au client. Enfin, elle nettoie les ressources utilisées avant 
 * de terminer. */
void *f(void *args);

/* Informations utiles pour les threads */
struct infos {
	int socket;
	char *dirname;
};

int main(int argc, char *argv[])
{
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}

	int serv_sock = creer_configurer_sock_ecoute(PORT_WCP);
	
	while (1) {
		struct sockaddr_in *sa_clt; 
		int sock_clt;
		socklen_t l = sizeof(struct sockaddr_in);

		if ((sock_clt = accept(serv_sock, (struct sockaddr *) &sa_clt, &l)) == -1) {
			perror("accept");
			exit(6);
		}
		
		struct infos *arg = malloc(sizeof(struct infos));
		if (arg < 0) {
			perror("malloc");
			exit(7);
		}
		arg->socket = sock_clt;
		arg->dirname = argv[1];
		
		pthread_t t;
		pthread_create(&t, NULL, f, arg);
		pthread_detach(t);
		
	}
	close(serv_sock);
	
	return 0;
};


int creer_configurer_sock_ecoute(uint16_t port)
{
	/*Configuration du socket */
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket");
		exit(1);
	}
	
	/* Configuration de l'adresse */
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Pour réutiliser l'adesse et le port sans attendre après la fin du serveur */
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

	if (bind(sock,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
		perror("bind");
		exit(2);
	}

	/* En écoute ...*/
	if (listen(sock, 3) == -1) {
		perror("listen");
		exit(3);
	}

	return sock;
}


void envoyer_liste(int fd, struct catalogue *c)
{
	int i;
	for (i = 0; i < c->nb && i <= UINT16_MAX; i++) {
		dprintf(fd, "%6d %s", i, c->tab[i]->titre);
	}
	dprintf(fd, "\n");
}


uint16_t recevoir_num_comptine(int fd)
{
	uint16_t nc;
	read(fd, &nc, sizeof(uint16_t));
	nc = ntohs(nc);
	return nc;
}


void envoyer_comptine(int fd, const char *dirname, struct catalogue *c, uint16_t ic)
{
	/* Initialise une chaîne avec le chemin jusqu'au fichier */
	char file[strlen(dirname) + strlen(c->tab[ic]->nom_fichier) + 2];
	strcpy(file, dirname);
	strcat(file, "/");
	strcat(file, c->tab[ic]->nom_fichier);

	/* Ouverture du fichier en lecture seule */
	int f = open(file, O_RDONLY);
	if (f == -1) {
		perror("open");
		exit(4);
	}

	/* On écrit dans fd la comptine contenue dans f */
	int size;
	char buf[BUFSIZE];
	while ((size = read(f, buf, BUFSIZE)) > 0) {
		write(fd, buf, size);
	}
	write(fd, "\n\n", 2);

	close(f);
}

/* Ajout Personnel */

void *f(void *args) {
	struct infos *clt = (struct infos*) args;
	
	struct catalogue* c = creer_catalogue(clt->dirname);
	if (c == NULL) {
		perror("malloc catalogue");
		exit(5);
	}
		
	envoyer_liste(clt->socket, c);
	uint16_t n = recevoir_num_comptine(clt->socket);
	envoyer_comptine(clt->socket, clt->dirname, c, n);
		
	close(clt->socket);
	liberer_catalogue(c);
	free(args);
	return NULL;
}

