#pragma once

#include <Engine/EMain.h>

// Platform.h defines WIN32_LEAN_AND_MEAN, which drops <commdlg.h> from
// Windows.h - the editor's file dialogs (OPENFILENAME) need it explicitly.
#include <commdlg.h>

#include <regex>
#include <comdef.h>
#include <algorithm>
#include <iostream>