/* Copyright 2010 Ilkka Halila
This file is part of Goblin Camp.

Goblin Camp is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Goblin Camp is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License 
along with Goblin Camp. If not, see <http://www.gnu.org/licenses/>.*/
#include "stdafx.hpp"

#include "scripting/_python.hpp"

#include "scripting/_gcampapi/Functions.hpp"
#include "Announce.hpp"
#include "scripting/API.hpp"
#include "Version.hpp"
#include "UI/MessageBox.hpp"
#include "Game.hpp"
#include "Logger.hpp"

namespace Script { namespace API {
	void Announce(const char* str) {
		::Announce::Inst()->AddMsg(str);
	}
	
	bool IsDebugBuild() {
	#ifdef DEBUG
		return true;
	#else
		return false;
	#endif
	}
	
	const char *GetVersionString() {
		return Globals::gameVersion;
	}
	
	void MessageBox(const char* str) {
		MessageBox::ShowMessageBox(str);
	}

	void Delay(int delay, PyObject* callback) {
		if (!PyCallable_Check(callback)) {
			LOG("WARNING: Attempted to add a delay to an uncallable object");
			return;
		}
		py::object function(py::handle<>(py::borrowed(callback)));
		Game::Inst()->AddDelay(delay, function);
	}

	void ExposeFunctions() {
		py::def("announce",         &Announce);
		py::def("appendListener",   &Script::AppendListener);
		py::def("getVersionString", &GetVersionString);
		py::def("isDebugBuild",     &IsDebugBuild);
		py::def("messageBox",       &MessageBox);
		py::def("delay",            &Delay);
	}
}}