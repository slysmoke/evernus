/**
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <QtGlobal>

#if defined(_MSC_VER) && _MSC_VER == 1900
#   define EVERNUS_VS_TEMPLATE_LAMBDA_HACK 1
#else
#   define EVERNUS_VS_TEMPLATE_LAMBDA_HACK 0
#endif

#ifndef _MSC_VER
#   define EVERNUS_CLANG_LAMBDA_CAPTURE_BUG 1
#else
#   define EVERNUS_CLANG_LAMBDA_CAPTURE_BUG 0
#endif

#ifdef NDEBUG
#   ifndef EVERNUS_CREATE_DUMPS
#       define EVERNUS_CREATE_DUMPS 1
#   endif
#else
#   undef EVERNUS_CREATE_DUMPS
#   define EVERNUS_CREATE_DUMPS 0
#endif
