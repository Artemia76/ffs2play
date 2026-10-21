/****************************************************************************
**
** Copyright (C) 2017 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/ffs2play
**
** FFS2Play is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** FFS2Play is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** The license is as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this software. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
****************************************************************************/

/****************************************************************************
** singleton.h is part of FF2Play project
**
** This provide template for singleton instanciation
****************************************************************************/

#ifndef SINGLETON_H
#define SINGLETON_H
#include <QtGlobal>

#include <memory>

///
/// Lazily created instance, destroyed at program exit.
/// Initialization of the function local static is thread safe since C++11.
/// The instance is returned by reference : it always exists and cannot be reassigned.
///
template <class T>
class Singleton
{
private:
    using CreateInstanceFunction = T* (*)();
public:
    static T& instance(CreateInstanceFunction pCreate)
    {
        static const std::unique_ptr<T> holder(pCreate());
        return *holder;
    }
private:
    Singleton() = delete;
    Q_DISABLE_COPY(Singleton)
};

#endif // SINGLETON_H
