/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2014-2016 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <mainwindow.h>
#include <common_features/grid.h>
#include <common_features/item_rectangles.h>
#include <editing/edit_level/level_edit.h>
#include <PGE_File_Formats/file_formats.h>

#include "../lvl_scene.h"
#include "../items/item_block.h"
#include "../items/item_water.h"
#include "../lvl_item_placing.h"

void LvlScene::setScreenshotSelector()
{
    m_captureFullSection=true;
    emit screenshotSizeCaptured();
}

void LvlScene::setScreenshotSelector(bool enabled, bool accept)
{
    bool do_signal=false;
    if( (enabled) && (m_resizeBox==nullptr) )
    {
        m_mw->on_actionSelect_triggered(); //Reset mode

        m_resizeBox = new ItemResizer( QSize(captutedSize.width(), captutedSize.height()), Qt::yellow, 2 );
        this->addItem(m_resizeBox);
        m_resizeBox->setPos(captutedSize.x(), captutedSize.y());
        m_resizeBox->type=4;
        m_resizeBox->_minSize = QSizeF(320, 200);
        this->setFocus(Qt::ActiveWindowFocusReason);
        //DrawMode=true;
        //m_mw->activeLvlEditWin()->changeCursor(WorldEdit::MODE_Resizing);
        SwitchEditingMode(MODE_Resizing);
        m_mw->resizeToolbarVisible(true);
    }
    else
    {
        if( m_resizeBox != nullptr )
        {
            if(accept)
            {
                #ifdef _DEBUG_
                WriteToLog(QtDebugMsg, QString("SCREENSHOT SELECTION ZONE -> to %1 x %2").arg(pResizer->_width).arg(pResizer->_height));
                #endif

                captutedSize = QRectF( m_resizeBox->pos().x(),
                                       m_resizeBox->pos().y(),
                                       m_resizeBox->_width,
                                       m_resizeBox->_height);
                do_signal=true;
            }
            delete m_resizeBox;
            m_resizeBox = nullptr;
            m_mw->on_actionSelect_triggered();
            m_mw->resizeToolbarVisible(false);
            //resetResizingSection=true;
        }
        m_busyMode=false;
    }
    m_captureFullSection=false;

    if(do_signal) emit screenshotSizeCaptured();
}

