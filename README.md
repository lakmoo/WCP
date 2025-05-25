# WikiComptines - Partage de comptines via terminal

Ce projet permet à des utilisateurs de **partager des comptines directement depuis leur terminal**, assurant une communication fluide et sans interruption. Il a été réalisé dans le cadre d’un devoir de programmation réseau, et un **rapport détaillé** est disponible dans ce dépôt.

## Fonctionnalités principales

- Partage de comptines entre utilisateurs connectés
- Serveur **multithreadé** capable de gérer **plusieurs clients simultanément**
- Structuration des threads via une structure dédiée

## Contenu du dépôt

- Code source du **client** et du **serveur**
- Fichier "Makefile" pour la compilation
- Quelques comptines dans le répertoire "comptines"
- Rapport expliquant les choix techniques et les difficultés rencontrées

→ [Lire le rapport](./rapport.pdf)

## Compilation

- Le projet utilise un "Makefile". Pour compiler : make
- Lancer le serveur : ./wcp_srv repertoire_comptines
- Lancer un client : ./wcp_client adresse_ipv4
