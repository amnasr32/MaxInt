*NOM,prénom,numéro d'étudiant*
### NASR Amira 
**Le protocole fonctionnera de la façon suivante :(voir le pdf pour plus de détails)
– le serveur accepte plusieurs connexions TCP de clients en parallèle ;
– lorsqu’un client se connecte au serveur, il peut envoyer au serveur un entier positif ou lui
demander de lui envoyer l’entier le plus grand parmi ceux qui lui ont été envoyés.
Dans la suite MAX_NAME 10 et les noms d’utilisateurs sont codés en ASCII.**
*Pour compiler :*

    - make binary
*Pour exécuter le serveur :*

    - ./serveur [numéro de port]

*Pour exécuter le client1 :*

    - ./client1 [adresse ip] [numéro de port]

*Pour exécuter le client2 :*

    - ./client2 [adresse ip] [numéro de port]
