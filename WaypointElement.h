#pragma once

#include <4dm.h>

using namespace fdm;
using namespace fdm::gui;

#include "ImgButton.h"

struct Waypoint;
class WaypointElement : public Button
{
public:
	std::vector<Waypoint>::iterator it;
	Text nameText;
	Text coordsText;
	Text colorText;
	ImgButton removeBtn;
	ImgButton editBtn;

	WaypointElement();

	void render(Window* w) override;
	bool mouseButtonInput(const Window* w, int button, int action, int mods) override;
};
