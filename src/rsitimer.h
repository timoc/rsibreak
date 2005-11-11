/* This file is part of the KDE project
   Copyright (C) 2005 Tom Albers <tomalbers@kde.nl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#ifndef RSITimer_H
#define RSITimer_H

#include <qdatetime.h>

class QTimer;
class QLabel;
class QDateTime;
class QStringList;
class QPushButton;
class RSIDock;
class KAccel;
class KPassivePopup;
class QPixmap;
class RSIWidget;

/**
 * @class RSITimer
 * This class controls the timings and arranges the maximizing
 * and minimizing of the widget
 * @author Tom Albers <tomalbers@kde.nl>
 */
class RSITimer : public QWidget
{
    Q_OBJECT

    public:
        /**
         * Constructor
         * @param parent Parent Widget
         * @param name Name
         */
        RSITimer( QWidget *parent = 0, const char *name = 0 );
        ~RSITimer();

    private slots:
        void slotMinimize();
        void slotMaximize();
        void slotReadConfig();
        void slotStop();
        void slotRestart();

    protected:
        virtual void timerEvent( QTimerEvent* );
        virtual void closeEvent( QCloseEvent* );

    private:
        void startMinimizeTimer();
        void findImagesInFolder(const QString& folder);
        void readConfig();
        void setCounters();
        void loadImage();
        int idleTime();
        void breakNow( int t );

        QPixmap*        m_backgroundimage;
        QString         m_basePath;
        QTime           m_targetTime;
        QTimer*         m_timer_max;
        QTimer*         m_timer_min;
        QTimer*         m_timer_slide;
        QLabel*         m_countDown;
        KAccel*         m_accel;

        RSIWidget*      m_RSIWidget;

        KPassivePopup*  m_popup;
        QLabel*         m_tool;

        bool            m_searchRecursive;
        bool            m_idleLong;
        bool            m_targetReached;
        bool            m_idleDetection;

        int             m_needBreak;
        int             m_timeMaximized;
        int             m_timeMinimized;
        int             m_bigInterval;
        int             m_bigTimeMaximized;
        int             m_currentInterval;
        int             m_slideInterval;

        int             m_idleIndex;
        int             m_idleIndexAmount;

        int             m_normalTimer;
        RSIDock*        m_tray;
        QStringList     m_files;
        QStringList     m_files_done;
        QPushButton*    m_miniButton;
        QPushButton*    m_lockButton;
};

#endif
