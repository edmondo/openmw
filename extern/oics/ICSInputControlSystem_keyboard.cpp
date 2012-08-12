/* -------------------------------------------------------
Copyright (c) 2011 Alberto G. Salguero (alberto.salguero (at) uca.es)

Permission is hereby granted, free of charge, to any
person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the
Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice
shall be included in all copies or substantial portions of
the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------- */

#include "ICSInputControlSystem.h"

namespace ICS
{
	void InputControlSystem::loadKeyBinders(TiXmlElement* xmlControlNode)
	{
		TiXmlElement* xmlKeyBinder = xmlControlNode->FirstChildElement("KeyBinder");    
		while(xmlKeyBinder)
		{
			Control::ControlChangingDirection dir = Control::STOP;
			if(std::string(xmlKeyBinder->Attribute("direction")) == "INCREASE")
			{
				dir = Control::INCREASE;
			}
			else if(std::string(xmlKeyBinder->Attribute("direction")) == "DECREASE")
			{
				dir = Control::DECREASE;
			}

			addKeyBinding(mControls.back(), mKeys[xmlKeyBinder->Attribute("key")], dir);

			xmlKeyBinder = xmlKeyBinder->NextSiblingElement("KeyBinder");
		}
	}

	void InputControlSystem::addKeyBinding(Control* control, OIS::KeyCode key, Control::ControlChangingDirection direction)
	{
		ICS_LOG("\tAdding KeyBinder [key="
			+ keyCodeToString(key) + ", direction="
			+ ToString<int>(direction) + "]");

		ControlKeyBinderItem controlKeyBinderItem;        
		controlKeyBinderItem.control = control;
		controlKeyBinderItem.direction = direction;
		mControlsKeyBinderMap[ key ] = controlKeyBinderItem;
	}

	void InputControlSystem::removeKeyBinding(OIS::KeyCode key)
	{
		ControlsKeyBinderMapType::iterator it = mControlsKeyBinderMap.find(key);
		if(it != mControlsKeyBinderMap.end())
		{
			mControlsKeyBinderMap.erase(it);
		}
	}

	OIS::KeyCode InputControlSystem::getKeyBinding(Control* control
		, ICS::Control::ControlChangingDirection direction)
	{
		ControlsKeyBinderMapType::iterator it = mControlsKeyBinderMap.begin();
		while(it != mControlsKeyBinderMap.end())
		{
			if(it->second.control == control && it->second.direction == direction)
			{
				return it->first;
			}
			it++;
		}

		return OIS::KC_UNASSIGNED;
	}
	bool InputControlSystem::keyPressed(const OIS::KeyEvent &evt) 
	{
		if(mActive)
		{
			if(!mDetectingBindingControl)
			{
				ControlsKeyBinderMapType::const_iterator it = mControlsKeyBinderMap.find(evt.key);
				if(it != mControlsKeyBinderMap.end())
				{
					it->second.control->setIgnoreAutoReverse(false);
					if(!it->second.control->getAutoChangeDirectionOnLimitsAfterStop())
					{
						it->second.control->setChangingDirection(it->second.direction);
					}
					else
					{                   
						if(it->second.control->getValue() == 1)
						{
							it->second.control->setChangingDirection(Control::DECREASE);
						}
						else if(it->second.control->getValue() == 0)
						{
							it->second.control->setChangingDirection(Control::INCREASE);
						}
					}
				}
			}
			else if(mDetectingBindingListener)
			{
				mDetectingBindingListener->keyBindingDetected(this,
					mDetectingBindingControl, evt.key, mDetectingBindingDirection);
			}
		}
	    
		return true;
	}

	bool InputControlSystem::keyReleased(const OIS::KeyEvent &evt) 
	{
		if(mActive)
		{
			ControlsKeyBinderMapType::const_iterator it = mControlsKeyBinderMap.find(evt.key);
			if(it != mControlsKeyBinderMap.end())
			{
				it->second.control->setChangingDirection(Control::STOP);
			}
		}

		return true;
	}

	void DetectingBindingListener::keyBindingDetected(InputControlSystem* ICS, Control* control
		, OIS::KeyCode key, Control::ControlChangingDirection direction)
	{
		// if the key is used by another control, remove it
		ICS->removeKeyBinding(key);

		// if the control has a key assigned, remove it
		OIS::KeyCode oldKey = ICS->getKeyBinding(control, direction);
		if(oldKey != OIS::KC_UNASSIGNED)
		{
			ICS->removeKeyBinding(oldKey);
		}

		ICS->addKeyBinding(control, key, direction);
		ICS->cancelDetectingBindingState();
	}

}