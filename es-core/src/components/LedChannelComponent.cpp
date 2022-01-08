#include "components/LedChannelComponent.h"
#include "resources/Font.h"

#include "leds.h"

#define MOVE_REPEAT_DELAY 500
#define MOVE_REPEAT_RATE 40

LedChannelComponent::LedChannelComponent(Window* window, int channel) : SliderComponent(window, 0.0f, 255.0f, 1.0f, "/255")
{
	mChannel = channel;
}

bool LedChannelComponent::input(InputConfig* config, Input input)
{
	//aggiorno lo stato del led
	if ( Led_Controller.Active )
	{
		if (config->isMappedLike("a", input))
		{
			Write_Channel(mChannel, (int)getValue());
			return true;
		}
		else if (config->isMappedLike("leftshoulder", input))
		{
			if(input.value)
			{
				setValue(getValue() + 5.0f);
				return true;
			}
		}
		else if (config->isMappedLike("rightshoulder", input)) 
		{
			if(input.value)
			{
				setValue(getValue() - 5.0f);
				return true;
			}
		}
	}

	return SliderComponent::input(config, input);
}