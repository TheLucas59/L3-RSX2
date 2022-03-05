# Lucas Plé

## RSX 2

### TP1 - DNS

Pour compiler les programmes :
```bash
make all
```

Pour envoyer un message en UDP :
```bash
./SendUDP <adresse_ip> <port>
```

Pour recevoir un message en UDP :
```bash
./ReceiveUDP <port>
```

Pour envoyer une requête DNS :
```bash
./DNS <nom_de_domaine>
```

Q1 : Pour réaliser les deux programmes permettant de communiquer avec des datagrammes UDP, il faut créer la socket, la lier avec l'adresse à laquelle on souhaite envoyer ou recevoir un message puis envoyer ou attendre de recevoir ce message.

Les deux programmes se ressemblent et il faut surtout faire attention à la création de la structure `struct sockaddr_in` qui peut être compliquée à comprendre au premier abord. 

Pour envoyer un message, il faut créer la socket ainsi qu'une structure `struct sockaddr_in` qui contiendra le port ainsi que l'adresse IP passée en paramètre à qui on souhaite envoyer le message. Il faut ensuite appeler la fonction `sendto` avec ces paramètres pour que le message s'envoie correctement. Après avoir vérifié qu'aucune erreur n'est arrivée, il ne faut pas oublier de fermer la socket avant de sortir du programme pour qu'elle ne reste pas éternellement ouverte.

Pour recevoir un message, il faut créer les mêmes éléments que précédemment, avec l'adresse localhost et le port passé en paramètre. Il faut ensuite bind manuellement la socket à la structure car la fonction `recv` qui permet de recevoir le message juste après ne bind pas la socket directement, comme le fait `sendto`. Il faut utiliser une taille de buffer suffisamment grande pour recevoir correctement les messages, et suffisamment petite pour ne pas saturer la mémoire de l'ordinateur. Encore une fois, il ne faut pas oublier de fermer la socket avant de sortir du programme.

Q2 : Pour réaliser la requête DNS, j'ai repris une grande partie du code donné en exemple pour bien comprendre comment les requêtes DNS fonctionnaient. Il faut de toute façon procéder de la même façon que précédemment avec une socket et une structure `struct sockaddr_in` pour envoyer et recevoir la requête.

Q3 & Q4 : On récupère le datagramme de réponse grâce à la fonction `recv` qui range tout ce qu'elle reçoit dans un buffer. Il faut maintenant décortiquer la réponse octet par octer. La [RFC 1035](https://www.ietf.org/rfc/rfc1035.txt) section 4 décrit la structure d'un message DNS. Ce document m'a beaucoup aidé pour trouver les différents champs qui la composent ainsi que comment calculer la taille des champs variables.

L'adresse IP du nom de domaine que l'on cherche se trouve dans la section réponse du datagramme, qui se trouve aux environs du milieu du datagramme. Il faut donc avancer dans le datagramme jusqu'à sa position. 

On passe tout d'abord l'en-tête qui a toujours une taille fixe de 12 octets. Certains champs nous sont utiles comme le QDCount ou le ANCount permettant de connaître le nombre d'enregistrements de questions et de réponses pour parcourir le datagramme. C'est ici assez simple d'avancer dans le datagramme.

On arrive en suite à la section de question. Cette section est, à quelques valeurs d'octets près, exactement la même que la requête que nous avons envoyé au serveur puisqu'il s'agit de notre question. Elle fait donc exactement la même taille en nombre d'octets. On peut donc se déplacer directement dans le buffer de réponse puisque l'on sait combien d'octets on a envoyé grâce à `sendto`.

Nous sommes ensuite dans la section qui nous intéresse : la section réponse. Le premier champ a une taille variable : NAME. Il y a deux façons de procéder : soit ce champ est formé comme le QName de la section question et dans ce cas on effectue le même traitement que précédemment, soit ce champ est un pointeur pour ne pas faire de répétition. Dans ce cas, il faut regarder les 2 bits de poids fort du premier octet. S'ils sont tous les deux à 1, le champ est un pointeur et sa taille est forcément de 2 octets. On avance donc de 2 octets dans le datagramme. Les champs suivants sont assez importants pour le contrôle des données. Les champs Type, Class et RDLength font 2 octets, alors que le TTL en fait 4. Avant d'avancer dans le datagramme, on récupère la valeur des 3 premiers champs pour contrôler les données. Si le champ Type vaut A (1), le champ Class IN (1), le champ RDLength doit avoir la valeur 4 et cela signifie que le champ RDData est une adresse iPv4. Si ces conditions sont réunies, on peut lire l'adresse IP sans problème dans le champ RDData. On lit les 4 octets dans un tableau et on peut ensuite la reconstruire dans une chaîne de caractère. On a ici le cas simple où la réponse DNS nous donne une adresse ip pour le nom de domaine que l'on a entré.

Il se peut également que le nom de domaine que l'on demande est en fait un alias. Dans ce cas, le serveur va nous le dire et nous donner le véritable nom de domaine caché derrière l'alias. Pour faire ça, il fixe le champ Type de la réponse à la valeur CNAME (5). Cela signifie que le champ RDData contient une chaîne DNS, à savoir une chaîne de caractères au même format que le QName de la section question. Il faut donc le parcourir et aller chercher si besoin les morceaux de chaîne de caractères pointées. On a ici besoin de sauvegarder l'offset puisqu'il faut aller chercher les chaînes de caractères pointées et non plus simplement passer au-dessus. Pour pouvoir récupérer la valeur donnée sur 14 octets, j'ai trouvé une fonction sur cette [page](https://www.geeksforgeeks.org/extract-k-bits-given-position-number/) permettant d'extraire un nombre entier sur un certain nombre de bits donnés. Une fois que l'on a l'offset, on peut parcourir la réponse à partir de cet indice pour continuer à construire notre chaîne de caractères d'alias avec ce morceau pointé. S'il n'y a pas de pointeur dans la chaîne DNS, on lit simplement les octets un par un correspondant à des caractères jusqu'à ce que l'on ait lu autant d'octets que le champ RDLength nous donne à lire.

Comme le serveur peut nous donner un certain nombre de réponses, il faut répéter ces deux cas autant de fois qu'il y a de réponses données par le serveur.

Ce programme permet d'isoler les types `CNAME` et `A` d'une réponse DNS. Il existe une dizaine d'autres valeurs de type comme pour l'IpV6 mais la plupart d'entre eux sont soit utilisés très rarement soit dépréciés.  
