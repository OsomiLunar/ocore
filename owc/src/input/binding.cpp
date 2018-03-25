#include "input/binding.h"
using namespace oi::wc;
using namespace oi;

Binding::Binding() : bindingType(0U), code(0U) {}
Binding::Binding(const Key_s k): bindingType((u8) BindingType::KEYBOARD), code((u16) ((Key)k).getIndex()) { }
Binding::Binding(const MouseButton_s c) : bindingType((u8)BindingType::MOUSE_BUTTON), code((u16)((MouseButton)c).getIndex()) { }
Binding::Binding(const MouseAxis_s ma) : bindingType((u8)BindingType::MOUSE_AXIS), code((u16)((MouseAxis)ma).getIndex()) { }
Binding::Binding(const ControllerButton_s cb, u8 controllerId) : bindingType((u8)BindingType::CONTROLLER_BUTTON), code((u16)((ControllerButton)cb).getIndex()) { }
Binding::Binding(const ControllerAxis_s ca, u8 controllerId) : bindingType((u8)BindingType::CONTROLLER_AXIS), code((u16)((ControllerAxis)ca).getIndex()) { }

u32 Binding::toUInt() const {
	return ((u32)bindingType << 24U) | ((u32)controllerId << 16U) | (u32)code;
}

BindingType Binding::getBindingType() const { return (BindingType) bindingType; }
u32 Binding::getControllerId() const { return (u32)controllerId; }
u32 Binding::getCode() const { return (u32) code; }

InputType Binding::getInputType() const {

	switch (getBindingType()) {

	case BindingType::CONTROLLER_AXIS:
	case BindingType::CONTROLLER_BUTTON:
		return InputType::CONTROLLER;

	case BindingType::MOUSE_AXIS:
	case BindingType::MOUSE_BUTTON:
		return InputType::MOUSE;

	default:
		return InputType::KEYBOARD;
	}

}

Key Binding::toKey() const {

	if (getBindingType() == BindingType::KEYBOARD)
		return Key::get(getCode());

	return Key::get(0);
}

MouseButton Binding::toMouseButton() const {

	if (getBindingType() == BindingType::MOUSE_BUTTON)
		return MouseButton::get(getCode());

	return MouseButton::get(0);
}

MouseAxis Binding::toMouseAxis() const {

	if (getBindingType() == BindingType::MOUSE_AXIS)
		return MouseAxis::get(getCode());

	return MouseAxis::get(0);
}

ControllerButton Binding::toButton() const {

	if (getBindingType() == BindingType::CONTROLLER_BUTTON)
		return ControllerButton::get(getCode());

	return ControllerButton::get(0);
}

ControllerAxis Binding::toAxis() const {

	if (getBindingType() == BindingType::CONTROLLER_AXIS)
		return ControllerAxis::get(getCode());

	return ControllerAxis::get(0);
}

String Binding::toString() const {

	if (getBindingType() == BindingType::KEYBOARD)
		return toKey().getName() + " key";

	if (getBindingType() == BindingType::MOUSE_BUTTON)
		return toMouseButton().getName() + " mouse";

	if (getBindingType() == BindingType::MOUSE_AXIS)
		return toMouseAxis().getName() + " mouse_axis";

	if (getBindingType() == BindingType::CONTROLLER_BUTTON)
		return toButton().getName() + " button #" + controllerId;

	if (getBindingType() == BindingType::CONTROLLER_AXIS)
		return toAxis().getName() + " axis #" + controllerId;

	return "Undefined";
}

Binding::Binding(String ostr) {

	auto split = ostr.split(" ");

	controllerId = 0U;

	if (split.size() != 1U) {

		String end = split[split.size() - 1];

		if (end.equalsIgnoreCase("key")) {

			bindingType = (u8)BindingType::KEYBOARD;
			code = (u8)Key(split[0]).getIndex();

		} else if (end.equalsIgnoreCase("mouse")) {

			bindingType = (u8) BindingType::MOUSE_BUTTON;
			code = (u8) MouseButton(split[0]).getIndex();

		} else if (end.equalsIgnoreCase("mouse_axis")) {

			bindingType = (u8)BindingType::MOUSE_AXIS;
			code = (u8) MouseAxis(split[0]).getIndex();

		} else if (split.size() == 3U) {

			u32 num = (u32)end.cutBegin(1).toLong();

			if (num > 255U)
				goto failed;

			controllerId = num;

			if (split[1].equalsIgnoreCase("button")) {

				bindingType = (u8)BindingType::CONTROLLER_BUTTON;
				code = (u8)ControllerButton(split[0]).getIndex();

			}
			else if (split[1].equalsIgnoreCase("axis")) {

				bindingType = (u8)BindingType::CONTROLLER_AXIS;
				code = (u8)ControllerAxis(split[0]).getIndex();
			}
		}

		goto success;
	}

failed:
	code = 0U;
	bindingType = 0xFFU;

success:
	;
}