/*
 * icotoprc.h
 *
 * Tool for converting a Windows Icon file to a format 
 * useable by the PWLib application "pwrc"
 *
 * Copyright (c) 2001 Philips Electronics N.V.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is icotoprc
 *
 * The Initial Developer of the Original Code is Phlilips Electronics N.V.
 *
 */

#ifndef ICOTOPRC_H
#define ICOTOPRC_H

#include <ptlib.h>

// Compile as a normal application with a console window
class ICOToPRC : public PProcess
{
  PCLASSINFO(ICOToPRC, PProcess)
public:
  // Resume normal class definition
  void Main();

private:
};

#endif
