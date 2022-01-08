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
	if(config->isMappedLike("a", input))
	{
		if ( Led_Controller.Active )
		{
			Write_Channel(mChannel, (int)mValue);
		}	
		
		return true;
	}
	
	return SliderComponent::input(config, input);
}