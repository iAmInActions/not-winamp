/***************************************************************************
 *   Copyright (C) 2009-2023 by Ilya Kotov                                 *
 *   forkotov02@ya.ru                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef VISUALPROJECTMFACTORY_H
#define VISUALPROJECTMFACTORY_H

#include <QObject>
#include <qmmp/visualfactory.h>
#include <qmmp/visual.h>

/**
    @author Ilya Kotov <forkotov02@ya.ru>
*/
class VisualProjectMFactory : public QObject, public VisualFactory
{
Q_OBJECT
Q_PLUGIN_METADATA(IID "org.qmmp.qmmp.VisualFactoryInterface.1.0")
Q_INTERFACES(VisualFactory)

public:
    VisualProperties properties() const override;
    Visual *create(QWidget *parent) override;
    QDialog *createConfigDialog(QWidget *parent) override;
    void showAbout(QWidget *parent) override;
    QString translation() const override;
};


#endif
