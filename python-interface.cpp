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
#include <iostream>
#include <string>
#include <netdb.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <vector>
#include <stdlib.h>

#include "ircbot.h"
#include "python-interface.h"

using namespace std;

IrcBot* PythonInterface::ircbot;
SystemUtils* PythonInterface::systemUtils;

/* Define the Python -> C API new module methods */
static PyMethodDef IrcMethods[] = {
    {"sendMessage", PythonInterface::sendPythonMessage, METH_VARARGS, "Send a basic message"},
    {"sendChannelMessage", PythonInterface::sendPythonChannelMessage, METH_VARARGS, "Sent a channel message"},
    {"sendMessageToSource", PythonInterface::sendPythonMessageToSource, METH_VARARGS, "Send a source message"},
    {"sendPrivateMessage", PythonInterface::sendPythonPrivateMessage, METH_VARARGS, "Send a private message"},
    {"runSystemCommand", PythonInterface::runSystemCommand, METH_VARARGS, "Run system command"},
    {"runSystemCommandWithOutput", PythonInterface::runSystemCommandWithOutput, METH_VARARGS, "Run system command with output"},
    {"getBotName", PythonInterface::getBotName, METH_VARARGS, "Get the irc bot name"},
    {"getChannelName", PythonInterface::getChannelName, METH_VARARGS, "Get the irc channel name"},
    {NULL, NULL, 0, NULL} // Sentinel value, end of Python API methods.
};

/* Define the new Python module */
static struct PyModuleDef ircmodule = {
    PyModuleDef_HEAD_INIT,
    "ircbot",  // name
    NULL,      // no docstring
    -1,        // size (-1 means module has global state)
    IrcMethods // module methods
};

/* Create the new Python module */
PyMODINIT_FUNC PyInitIrc(void)
{
    return PyModule_Create(&ircmodule);
}

PyObject * PythonInterface::sendPythonMessage(PyObject *self, PyObject *args)
{
  const char * message;
  if(!PyArg_ParseTuple(args, "s", &message))
      Py_RETURN_FALSE;
  string cMessage = message;
  ircbot->sendMessage(cMessage);
  Py_RETURN_TRUE;
}

PyObject * PythonInterface::sendPythonChannelMessage(PyObject *self, PyObject *args)
{
  string msg = (*ircbot).getChannelSendString();
  const char * message;
  if(!PyArg_ParseTuple(args, "s", &message))
      Py_RETURN_FALSE;
  string messageOnly = message;
  msg  += messageOnly;
  // add carridge return and new line to the end of messages
  msg += "\r\n";
  // update the history file
  ircbot->writeHistory(ircbot->getNick(), "", messageOnly + "\r\n");
  send(ircbot->connectionSocket,msg.c_str(),msg.length(),0);
  Py_RETURN_TRUE;
}

PyObject * PythonInterface::runSystemCommand(PyObject *self, PyObject *args)
{
  const char * command;
  if(!PyArg_ParseTuple(args, "s", &command))
      Py_RETURN_FALSE;
  system(command);
  Py_RETURN_TRUE;
}

PyObject * PythonInterface::runSystemCommandWithOutput(PyObject *self, PyObject *args)
{
  const char * command;
  if(!PyArg_ParseTuple(args, "s", &command))
      Py_RETURN_TRUE;
  string ret =  systemUtils->runProcessWithReturn(command);
  return PyBytes_FromString(ret.c_str());
}

PyObject * PythonInterface::getBotName(PyObject *self, PyObject *args)
{
  return PyBytes_FromString(ircbot->getNick().c_str());
}

PyObject * PythonInterface::getChannelName(PyObject *self, PyObject *args)
{
  return PyBytes_FromString(ircbot->getChannel().c_str());
}

PyObject * PythonInterface::sendPythonPrivateMessage(PyObject *self, PyObject *args)
{
  const char *username, *message;
  if(!PyArg_ParseTuple(args, "ss", &username, &message))
      Py_RETURN_FALSE;
  ircbot->outputToUser(username, message);
  Py_RETURN_TRUE;
}


PyObject * PythonInterface::sendPythonMessageToSource(PyObject *self, PyObject *args)
{
  const char *username, *message, *isPrivate;

  if(!PyArg_ParseTuple(args, "sss", &username, &message, &isPrivate)) {
      PyErr_Print();
      Py_RETURN_FALSE;
  }

  // TODO: Should be able to parse it as a char or actually an int/bool and skip this conversion
  bool isPrivateMessage = (*isPrivate != '0');

  string msg = ircbot->getChannelSendString();
  msg  += message;
  msg += "\r\n";

  if (isPrivateMessage)
  {
    sendPythonPrivateMessage(self, args);
  }
  else 
  {
    // update the history file
    ircbot->writeHistory(ircbot->getNick(), "", msg + "\r\n");
    send(ircbot->connectionSocket, msg.c_str(), msg.length(), 0);
  }
  Py_RETURN_TRUE;
}

PythonInterface::PythonInterface(string thisSaveFile) 
{
  saveFile = thisSaveFile;
}

PythonInterface::PythonInterface() 
{
  map<string, string> pythonFileHashes; // a map from file name to hash of the file // ? maybe runwithplugins
}

void PythonInterface::closeState() 
{
  // Let go of the reference to the interpreter state
  Py_XDECREF(this->pythonLoaderState);
  // Shut down the interpreter
  Py_Finalize();
}

void PythonInterface::runPythonLoaderStateMethod(string methodName, PyObject *args)
{
  PyObject * function = PyObject_GetAttrString(pythonLoaderState, methodName.c_str());
  PyObject * pValue = PyObject_CallObject(function, args);

  // Calling the loader failed
  if(pValue == NULL) {
      PyErr_Print();
      std::cerr << "Call to "  << methodName <<  " didn't complete. " << std::endl;
      // Must cleanup before exiting
      Py_XDECREF(args);
      Py_XDECREF(function);
      Py_XDECREF(pValue);
      exit(1);
  }

  // Cleanup
  Py_XDECREF(args);
  Py_XDECREF(function);
  Py_XDECREF(pValue);
}

void PythonInterface::runPlugins(string username, string serverInfo, string userMessage, bool isPrivateMessage) {

    PyObject * pArgs = Py_BuildValue("ssss", username.c_str(), serverInfo.c_str(), userMessage.c_str(), isPrivateMessage ? "1" : "0");

    runPythonLoaderStateMethod("main", pArgs);
}

void PythonInterface::initState(IrcBot* bot) 
{

  this->ircbot = bot;

  // Add a built-in module, must be before Py_Initialize 
  PyImport_AppendInittab("ircbot", PyInitIrc);

  // Initialize the Python interpreter.
  Py_Initialize();

  // Extend the python path to include our new locations for plugins
  PyRun_SimpleString("import sys\nsys.path.append(\".\")\nsys.path.append(\"./python/\")\nsys.path.append(\"./python/plugins\")");

  PyObject * ircModule = PyImport_ImportModule("ircbot");

  cerr << "-- Loading plugin: " << "python/loader.py" << endl;

  PyObject * loaderModule = PyImport_ImportModule("loader");
  if(!loaderModule || !ircModule) 
  {
      PyErr_Print();
      exit(1);
  }

  PyObject *loaderDict;
  try 
  {
      loaderDict = PyModule_GetDict(loaderModule);
  }
  catch(...)
  {
      PyErr_Print();
      exit(1);
  }
  if(!loaderDict) 
  {
      PyErr_Print();
      exit(1);
  }



  // Register the new module with the loader's dictionary (i.e. make it importable to the loader)
  PyDict_SetItemString(loaderDict, "ircbot", ircModule);

  // Save the state of the loader module, make sure we keep a reference to it open.
  Py_XINCREF(loaderModule); 
  this->pythonLoaderState = loaderModule;

  // Run the "run" method of the loader with no arguments
  PyObject * pArgs = Py_BuildValue("()");
  runPythonLoaderStateMethod("updatePlugins", pArgs);

  // Cleanup the PyObjects we no longer need (must be manually deallocated)
  Py_XDECREF(ircModule);
  Py_XDECREF(loaderDict);

}


