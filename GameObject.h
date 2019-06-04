#pragma once

/*
 * Represente un objet quelconque du jeu, qui peut etre affiche a l'ecran et mis a jour.
 */
class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

    /*
     * Appele regulierement. dt est le temps, en secondes, depuis le dernier appel
     * a cette fonction.
     */
	virtual void update(float dt);

    /*
     * Effectue le rendu de cet objet. ptt est le temps de tick partiel.
     */
	virtual void render(float ptt);
};
