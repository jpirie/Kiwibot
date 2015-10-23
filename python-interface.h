/*********************************************************************
 * Copyright 2015 Peter Gatens
 *
 * Kiwibot is a free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kiwibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kiwibot.  If not, see <http://www.gnu.org/licenses/>.
 *
 ********************************************************************/

#ifndef PYTHON_INTERFACE_H
#define PYTHON_INTERFACE_H

// Python interpreter include - must be first
#include <Python.h>
#include "ircbot.h"
#include "system-utils.h"




class PythonInterface {
 private:
  std::map<std::string, std::string> pythonFileHashes;
  static IrcBot* ircbot;
  static SystemUtils* systemUtils;
  bool firstPluginLoad;
  std::string saveFile;

 public:
  PythonInterface();
  PythonInterface(std::string saveFile);
  PyObject* pythonLoaderState;
  static PyObject* sendPythonMessage(PyObject *self, PyObject *args);
  static PyObject* sendPythonChannelMessage(PyObject *self, PyObject *args);
  static PyObject* sendPythonMessageToSource(PyObject *self, PyObject *args);
  static PyObject* getPluginData(PyObject *self, PyObject *args);
  static PyObject* setPluginData(PyObject *self, PyObject *args);
  static PyObject* runSystemCommand(PyObject *self, PyObject *args);
  static PyObject* sendPythonPrivateMessage(PyObject *self, PyObject *args);
  static PyObject* runSystemCommandWithOutput(PyObject *self, PyObject *args);
  static PyObject* getBotName(PyObject *self, PyObject *args);
  static PyObject* getChannelName(PyObject *self, PyObject *args);
  void runPythonLoaderStateMethod(std::string methodName, PyObject *args);
  void initState(IrcBot* ircbot);
  void closeState();
  void runPlugins(std::string username ,std::string serverInfo ,std::string userMessage, bool isPrivateMessage);
};

#endif /* PYTHON_INTERFACE_H_ */
