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
	ImgButton removeBtn;
	ImgButton editBtn;
	ImgButton lookAtBtn;

	WaypointElement();

	void render(Window* w) override;
	bool mouseButtonInput(const Window* w, int button, int action, int mods) override;
};
