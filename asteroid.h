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
class Asteroid
{
	public:

	XMFLOAT2 pos {};
	float size = 100.0f;
	float rot = 0;
	AsteroidType::Type type;
	AsteroidCategory::Category category;

};