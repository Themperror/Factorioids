#pragma once

#include <DirectXMath.h>
namespace AsteroidType
{
	enum Type { Carbonic, Metallic, Oxide, Promethium, ENUM_MAX };
}
namespace AsteroidCategory
{
	enum Category { Small, Medium, Big, Huge, ENUM_MAX };
}
struct Asteroid
{
	XMFLOAT2 pos {};
	XMFLOAT2 velocity {};
	float health = 100.0f;
	float size = 100.0f;
	float rot = 0;
	float rotSpeed = 0;
	AsteroidType::Type type;
	AsteroidCategory::Category category;
	int variation;
};