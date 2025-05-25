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
#include <sys/types.h>
#include <sys/socket.h>
/* spécifique à internet */
#include <arpa/inet.h> /* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"

#define PORT_WCP 4321
#define BUFSIZE 1024

void usage(char *nom_prog)
{
	fprintf(stderr, "Usage: %s addr_ipv4\n"
			"client pour WCP (Wikicomptine Protocol)\n"
			"Exemple: %s 208.97.177.124\n", nom_prog, nom_prog);
}

/** Retourne (en cas de succès) le descripteur de fichier d'une socket
 *  TCP/IPv4 connectée au processus écoutant sur port sur la machine d'adresse
 *  addr_ipv4 */
int creer_connecter_sock(char *addr_ipv4, uint16_t port);

/** Lit la liste numérotée des comptines dans le descripteur fd et les affiche
 *  sur le terminal.
 *  retourne : le nombre de comptines disponibles */
uint16_t recevoir_liste_comptines(int fd);

/** Demande à l'utilisateur un nombre entre 0 (compris) et nc (non compris)
 *  et retourne la valeur saisie. */
uint16_t saisir_num_comptine(uint16_t nb_comptines);

/** Écrit l'entier ic dans le fichier de descripteur fd en network byte order */
void envoyer_num_comptine(int fd, uint16_t nc);

/** Affiche la comptine arrivant dans fd sur le terminal */
void afficher_comptine(int fd);


int main(int argc, char *argv[])
{
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}

	/* Boucle infinie pour que les client puissent demander indéfiniment à lire une comptine */
	while(1) {
		int sock = creer_connecter_sock(argv[1], PORT_WCP);

		uint16_t n_comptines = recevoir_liste_comptines(sock);
		uint16_t n = saisir_num_comptine(n_comptines);
		
		/* Condition d'arrêt de la boucle */
		if (n < 0 || n >= n_comptines) break;
		
		envoyer_num_comptine(sock, n);
		afficher_comptine(sock);
		
		close(sock);
	}

	return 0;
}


int creer_connecter_sock(char *addr_ipv4, uint16_t port)
{
	/* Configuration du socket */
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket");
		exit(1);
	}

	/* Configuration de l'adresse */
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	if (inet_pton(AF_INET, addr_ipv4, &serv_addr.sin_addr) != 1) {
		perror("Adresse invalide");
		exit(2);
	}

	/* Connexion au serveur */
	int statut = connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (statut == -1) {
		printf("Erreur lors de la connexion\n");
		exit(3);
	}

	return sock;
}


uint16_t recevoir_liste_comptines(int fd)
{
    char buf[BUFSIZE];
    int taille = 0;
    uint16_t i = 0;
    while ((taille = read_until_nl(fd, buf)) > 0) {
    	buf[taille] = '\n';
        buf[taille + 1] = '\0';
        printf("%s", buf);
        i++;
    }

    return i;
}


uint16_t saisir_num_comptine(uint16_t nb_comptines)
{
	uint16_t nc = UINT16_MAX;

	printf("Quelle comptine souhaitez-vous lire ? (Saisissez un nombre entre 0 et %d\n): ", nb_comptines - 1);
	scanf("%" SCNu16, &nc);

	return nc;
}


void envoyer_num_comptine(int fd, uint16_t nc)
{
	uint16_t n = htons(nc);
	write(fd, &n, sizeof(uint16_t));
}


void afficher_comptine(int fd)
{
	char buf[BUFSIZE];
	int i = 0;
	while(1) {
		if ((i = read_until_nl(fd, buf)) == 0) {
			if ((i = read_until_nl(fd, buf)) == 0) return;
			else printf("\n");
		}
		buf[i+1] = '\0';
		printf("%s", buf);
	}
}

