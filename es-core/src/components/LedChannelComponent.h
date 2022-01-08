#pragma once
#ifndef ES_CORE_COMPONENTS_LEDCHANNEL_COMPONENT_H
#define ES_CORE_COMPONENTS_LEDCHANNEL_COMPONENT_H

#include "components/ImageComponent.h"
#include "components/SliderComponent.h"
#include "GuiComponent.h"

class Font;
class TextCache;

// Used to display/edit a value between some min and max values.
class LedChannelComponent : public SliderComponent
{
public:
	//Minimum value (far left of the slider), maximum value (far right of the slider), increment size (how much just pressing L/R moves by), unit to display (optional).
	LedChannelComponent(Window* window, int channel);

	void setValue(float val) override;

	bool input(InputConfig* config, Input input) override;

private:

	void onValueChanged();

	int mChannel;
};

#endif // ES_CORE_COMPONENTS_SLIDER_COMPONENT_H
