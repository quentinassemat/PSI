import matplotlib.pyplot as plt


# affichage des graphes  de temps de calcul du serveur quand card client vaut 1
# le temps de calcul du client ne change que très peu : bien pas dépendant d
CARD = [200, 1000, 3000, 5000]
SERVER_ROUND_2 = [0.121000,  0.140000, 0.124000, 0.150000] # round 3 c'est la computation du serveur

plt.plot(CARD, SERVER_ROUND_2, marker=".")
plt.xlabel("Taille de l'ensemble du serveur")
plt.ylabel('Temps (ms)')
plt.show()

SERVER_ROUND_3 = [707.322000, 3406.617000, 10271.709000, 17196.722000] # round 3 c'est la computation du serveur

plt.plot(CARD, SERVER_ROUND_3, marker=".")
plt.xlabel("Taille de l'ensemble du serveur")
plt.ylabel('Temps (ms)')
plt.show()

SERVER_ROUND_4 = [0.275000, 1.035000, 2.783000, 4.698000] # round 3 c'est la computation du serveur

plt.plot(CARD, SERVER_ROUND_4, marker=".")
plt.xlabel("Taille de l'ensemble du serveur")
plt.ylabel('Temps (ms)')
plt.show()

#affichage des graphes quand le server vaut 1

CLIENT_ROUND_1= [1195.961000, 6142.402000, 17723.624000, 29781.314000] # round 3 c'est la computation du serveur

plt.plot(CARD, CLIENT_ROUND_1, marker=".")
plt.xlabel("Taille de l'ensemble du client")
plt.ylabel('Temps (ms)')
plt.show()

CLIENT_ROUND_2 = [0.374000, 1.635000, 6.148000, 9.456000] # round 3 c'est la computation du serveur

plt.plot(CARD, CLIENT_ROUND_2, marker=".")
plt.xlabel("Taille de l'ensemble du client")
plt.ylabel('Temps (ms)')
plt.show()

CLIENT_ROUND_4 = [0.629000, 2.423000, 7.644000, 12.549000] # round 3 c'est la computation du serveur

plt.plot(CARD, CLIENT_ROUND_4, marker=".")
plt.xlabel("Taille de l'ensemble du client")
plt.ylabel('Temps (ms)')
plt.show()
