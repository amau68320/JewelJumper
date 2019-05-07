#pragma once
class GameObject
{
public:
	GameObject();
	virtual ~GameObject();
	virtual void update(float dt);
	virtual void render(float ptt);
};

