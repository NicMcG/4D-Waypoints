#include "WaypointElement.h"

WaypointElement::WaypointElement()
{
	nameText = Text{};
	nameText.size = 2;
	nameText.fancy = true;
	nameText.shadow = true;

	coordsText = Text{};
	coordsText.size = 1;
	coordsText.shadow = true;
	
	removeBtn = ImgButton("assets/remove.png", 32, 32, 0, 0);
	editBtn = ImgButton("assets/edit.png", 32, 32, 0, 0);
	lookAtBtn = ImgButton("assets/lookat.png", 32, 32, 0, 0);

	height = 32;
}

void WaypointElement::render(Window* w)
{
	Button::render(w);

	nameText.xOffset = xOffset + 2;
	nameText.yOffset = yOffset + 2;
	nameText.render(w);
	
	coordsText.xOffset = nameText.xOffset + 2;
	coordsText.yOffset = yOffset + height - 10;
	coordsText.render(w);

	removeBtn.xOffset = xOffset + width - 32;
	removeBtn.yOffset = yOffset;
	removeBtn.render(w);

	editBtn.xOffset = xOffset + width - 64 - 4;
	editBtn.yOffset = yOffset;
	editBtn.render(w);
	
	lookAtBtn.xOffset = xOffset + width - 96 - 8;
	lookAtBtn.yOffset = yOffset;
	lookAtBtn.render(w);
}

bool WaypointElement::mouseButtonInput(const Window* w, int button, int action, int mods)
{
	if (removeBtn.mouseButtonInput(w, button, action, mods)) return true;
	if (editBtn.mouseButtonInput(w, button, action, mods)) return true;
	if (lookAtBtn.mouseButtonInput(w, button, action, mods)) return true;

	return Button::mouseButtonInput(w, button, action, mods);
}
