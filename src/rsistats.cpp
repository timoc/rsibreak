/* This file is part of the KDE project
   Copyright (C) 2006 Bram Schoenmakers <bramschoenmakers@kde.nl>

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

#include <qwhatsthis.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "rsistats.h"
#include "rsiglobals.h"

class RSIStatItem
{
  public:
    RSIStatItem( const QString &description )
    {
      m_description = new QLabel( description, 0 );
      m_value = QVariant(0);
    }
    RSIStatItem() : m_value( QVariant(0) ) {}
    ~RSIStatItem() {}

    QLabel *getDescription() const { return m_description; }
    QVariant getValue()      const { return m_value; }

    void setValue( QVariant v ) { m_value = v; }

    void addDerivedItem( RSIStat stat ) { m_derived += stat; }
    QValueList<RSIStat> getDerivedItems() const { return m_derived; }

  private:
    QVariant m_value;
    QLabel *m_description;

    /** Contains a list of RSIStats which depend on *this* item. */
    QValueList< RSIStat > m_derived;
};

RSIStats *RSIStats::m_instance = 0;

RSIStats::RSIStats()
{
    kdDebug() << "RSIStats::RSIStats() entered" << endl;

    m_statistics.insert( TOTAL_TIME, RSIStatItem(i18n("Total recorded time") ) );
    m_statistics[TOTAL_TIME].addDerivedItem( ACTIVITY_PERC );

    m_statistics.insert( ACTIVITY, RSIStatItem(i18n("Total time of activity") ) );
    m_statistics[ACTIVITY].addDerivedItem( ACTIVITY_PERC );

    m_statistics.insert( IDLENESS, RSIStatItem(i18n("Total time being idle") ) );

    m_statistics.insert( ACTIVITY_PERC, RSIStatItem(i18n("Percentage of activity") ) );

    m_statistics.insert( MAX_IDLENESS, RSIStatItem(i18n("Maximum idle period") ) );
    m_statistics[MAX_IDLENESS].addDerivedItem( IDLENESS );

    m_statistics.insert( TINY_BREAKS, RSIStatItem(i18n("Total amount of tiny breaks") ) );
    m_statistics[TINY_BREAKS].addDerivedItem( PAUSE_SCORE );
    m_statistics[TINY_BREAKS].addDerivedItem( LAST_TINY_BREAK );

    m_statistics.insert( LAST_TINY_BREAK, RSIStatItem(i18n("Last tiny break") ) );

    m_statistics.insert( TINY_BREAKS_SKIPPED,
            RSIStatItem(i18n("Number of skipped tiny breaks (user)") ) );
    m_statistics[TINY_BREAKS_SKIPPED].addDerivedItem( PAUSE_SCORE );


    m_statistics.insert( IDLENESS_CAUSED_SKIP_TINY,
            RSIStatItem(i18n("Number of skipped tiny breaks (idle)") ) );

    m_statistics.insert( BIG_BREAKS, RSIStatItem(i18n("Total amount of big breaks") ) );
    m_statistics[BIG_BREAKS].addDerivedItem( PAUSE_SCORE );
    m_statistics[BIG_BREAKS].addDerivedItem( LAST_BIG_BREAK );

    m_statistics.insert( LAST_BIG_BREAK, RSIStatItem(i18n("Last big break") ) );

    m_statistics.insert( BIG_BREAKS_SKIPPED,
            RSIStatItem(i18n("Number of skipped big breaks (user)") ) );
    m_statistics[BIG_BREAKS_SKIPPED].addDerivedItem( PAUSE_SCORE );

    m_statistics.insert( IDLENESS_CAUSED_SKIP_BIG,
            RSIStatItem(i18n("Number of skipped big breaks (idle)") ) );

    // FIXME: Find better name and whatsthis
    m_statistics.insert( PAUSE_SCORE, RSIStatItem(i18n("Pause score") ) );

    // initialise labels
    QMap<RSIStat,RSIStatItem>::Iterator it;
    for ( it = m_statistics.begin(); it != m_statistics.end(); ++it )
    {
      m_labels[it.key()] = new QLabel( 0 );
      QWhatsThis::add( m_labels[it.key()], getWhatsThisText( it.key() ) );
      QWhatsThis::add( m_statistics[it.key()].getDescription(), getWhatsThisText( it.key() ) );
    }


    // initialise statistics
    updateLabels();
    reset();
}

RSIStats::~RSIStats()
{
    kdDebug() << "RSIStats::~RSIStats() entered" << endl;

    QMap<RSIStat,QLabel *>::Iterator it;
    for ( it = m_labels.begin() ; it != m_labels.end(); ++it )
    {
      delete it.data();
      it.data() = 0L;
    }

    QMap<RSIStat,RSIStatItem>::Iterator it2;
    for ( it2 = m_statistics.begin() ; it2 != m_statistics.end(); ++it2 )
    {

      QLabel *l = it2.data().getDescription();
      delete l;
      l = 0L;
    }
}

RSIStats *RSIStats::instance()
{
    if ( !m_instance )
        m_instance = new RSIStats();

    return m_instance;
}

void RSIStats::reset()
{
    kdDebug() << "RSIStats::reset() entered" << endl;

    QMap<RSIStat,RSIStatItem>::ConstIterator it;
    for( it = m_statistics.begin(); it != m_statistics.end(); ++it )
    {
        RSIStat stat = it.key();
        QVariant v = m_statistics[stat].getValue();
        if ( v.type() == QVariant::Int || v.type() == QVariant::Double )
            m_statistics[stat].setValue( 0 );
        else if ( v.type() == QVariant::Time )
            m_statistics[stat].setValue( QTime());
        updateStat( stat, false );
    }
}

void RSIStats::increaseStat( RSIStat stat, int delta )
{
    // kdDebug() << "RSIStats::increaseStat() entered" << endl;

    QVariant v = m_statistics[stat].getValue();

    if ( v.type() == QVariant::Int )
      m_statistics[stat].setValue( v.toInt() + delta );
    else if ( v.type() == QVariant::Double )
      m_statistics[stat].setValue( v.toDouble() + (double)delta );
    else if ( v.type() == QVariant::Time )
                m_statistics[stat].setValue(
                    QTime(v.toTime()).addSecs( delta ));

    updateStat( stat );
}

void RSIStats::setStat( RSIStat stat, QVariant val, bool ifmax )
{
    // kdDebug() << "RSIStats::increaseStat() entered" << endl;

    QVariant v = m_statistics[stat].getValue();

    if ( !ifmax ||
           (v.type() == QVariant::Int && val.toInt()>v.toInt()) ||
           (v.type() == QVariant::Double && val.toDouble()>v.toDouble()) ||
           (v.type() == QVariant::Time && val.toTime()>v.toTime()))
        m_statistics[stat].setValue( val );
    
    // WATCH OUT: IDLENESS is derived from MAX_IDLENESS and needs to be 
    // updated regardless if a new value is set.
    updateStat( stat );
}

void RSIStats::updateDependentStats( RSIStat stat )
{
    // kdDebug() << "RSIStats::updateDependentStats " << stat << endl;

    QValueList<RSIStat> stats = m_statistics[ stat ].getDerivedItems();
    QValueList<RSIStat>::ConstIterator it;
    for( it = stats.begin() ; it != stats.end(); ++it )
    {
        switch( (*it) )
        {
        case PAUSE_SCORE:
        {
          double a = m_statistics[ TINY_BREAKS_SKIPPED ].getValue().toDouble();
          double b = m_statistics[ BIG_BREAKS_SKIPPED ].getValue().toDouble();
          double c = m_statistics[ IDLENESS_CAUSED_SKIP_TINY ].getValue().toDouble();
          double d = m_statistics[ IDLENESS_CAUSED_SKIP_BIG ].getValue().toDouble();

          double skipped = a;
          if ( a > c )
            skipped -= c;

          skipped += 2 * b;
          if ( b > d )
            skipped -= 2 * d;

          double total = m_statistics[ TINY_BREAKS ].getValue().toDouble();
          total += 2 * m_statistics[ BIG_BREAKS ].getValue().toDouble();

          if ( total > 0 )
            m_statistics[ *it ].setValue( 100 - ( ( skipped / total ) * 100 ) );
          else
            m_statistics[ *it ].setValue( 0 );

          updateStat( *it );
          break;
        }

        case IDLENESS:
        {
            increaseStat( IDLENESS );
            break;
        }
        case ACTIVITY_PERC:
        {
          /*
                                           seconds of activity
              activity_percentage =  100 - -------------------
                                               total seconds
          */

          double activity = m_statistics[ ACTIVITY ].getValue().toDouble();
          double total = m_statistics[TOTAL_TIME].getValue().toDouble();

          if ( total > 0 )
            m_statistics[ *it ].setValue( (activity / total) * 100 );
          else
            m_statistics[ *it ].setValue( 0 );

          updateStat( *it );
          break;
        }

        case LAST_BIG_BREAK:
        {
            setStat( LAST_BIG_BREAK, QTime::currentTime() );
            break;
        }

        case LAST_TINY_BREAK:
        {
            setStat( LAST_TINY_BREAK, QTime::currentTime() );
            break;
        }

        default: ;// nada
      }
    }
}

void RSIStats::updateStat( RSIStat stat, bool updateDerived )
{
    // kdDebug() << "RSIStats::updateStat() entered" << endl;

    if ( updateDerived )
      updateDependentStats( stat );
    updateLabel( stat );
}

void RSIStats::updateLabel( RSIStat stat )
{
    // kdDebug() << "RSIStats::updateLabel() entered" << endl;

    QLabel *l = m_labels[ stat ];
    QColor c;
    double v;

    switch ( stat )
    {
        // integer values representing a time
        case TOTAL_TIME:
        case ACTIVITY:
        case IDLENESS:
        case MAX_IDLENESS:
        l->setText( RSIGlobals::instance()->formatSeconds(
                        m_statistics[ stat ].getValue().toInt() ) );
        break;

        // plain integer values
        case TINY_BREAKS:
        case TINY_BREAKS_SKIPPED:
        case IDLENESS_CAUSED_SKIP_TINY:
        case BIG_BREAKS:
        case BIG_BREAKS_SKIPPED:
        case IDLENESS_CAUSED_SKIP_BIG:
        l->setText( QString::number(
                        m_statistics[ stat ].getValue().toInt() ) );
        break;

        // doubles
        case PAUSE_SCORE:
        v = m_statistics[ stat ].getValue().toDouble();
        c = QColor( (int)(255 - 2.55 * v), (int)(1.60 * v), 0 );
        l->setPaletteForegroundColor( c );
        m_statistics[stat].getDescription()->setPaletteForegroundColor( c );
        l->setText( QString::number(
                        m_statistics[ stat ].getValue().toDouble(), 'f', 1 ) );
        break;
        case ACTIVITY_PERC:
        v = m_statistics[stat].getValue().toDouble();
        c = QColor( (int)(2.55 * v), (int)(160 - 1.60 * v), 0 );
        l->setPaletteForegroundColor( c );
        m_statistics[stat].getDescription()->setPaletteForegroundColor( c );
        l->setText( QString::number(
                        m_statistics[ stat ].getValue().toDouble(), 'f', 1 ) );
        break;

        // times
        case LAST_BIG_BREAK:
        case LAST_TINY_BREAK:
        {
            KLocale *localize = KGlobal::locale();
            QTime when(m_statistics[ stat ].getValue().toTime());
            when.isValid() ? l->setText( localize->formatTime(when, true, true) )
                           : l->setText( QString::null );
            break;
        }

        default: ; // nada
    }

    // some stats need a %
    if ( stat == PAUSE_SCORE || stat == ACTIVITY_PERC )
        l->setText( l->text() + "%" );
}

void RSIStats::updateLabels()
{
    // kdDebug() << "RSIStats::updateLabels()" << endl;

    QMapConstIterator<RSIStat,RSIStatItem> it;
    for ( it = m_statistics.begin() ; it != m_statistics.end() ; ++it )
      updateLabel( it.key() );
}

QVariant RSIStats::getStat( RSIStat stat ) const
{
    // kdDebug() << "RSIStats::getStat() entered" << endl;

    return m_statistics[ stat ].getValue();
}

QLabel *RSIStats::getLabel( RSIStat stat ) const
{
    // kdDebug() << "RSIStats::getFormattedStat() entered" << endl;

    return m_labels[ stat ];
}

QLabel *RSIStats::getDescription( RSIStat stat ) const
{
    // kdDebug() << "RSIStats::getDescription() entered" << endl;

    return m_statistics[stat].getDescription();
}

QString RSIStats::getWhatsThisText( RSIStat stat ) const
{
    switch ( stat )
    {
      case TOTAL_TIME: return i18n("This is the total time RSIBreak is running.");
      case ACTIVITY: return i18n("This is the total amount of time you used the "
                          "keyboard or mouse.");
      case IDLENESS: return i18n("This is the total amount of time you did not use "
                          "the keyboard or mouse.");
      case ACTIVITY_PERC: return i18n("This is a percentage of activity, based on the "
                          "seconds of activity vs. total time RSIBreak is running.");
      case MAX_IDLENESS: return i18n("This is the biggest period of inactivity measured "
                          "while RSIBreak is running.");
      case TINY_BREAKS: return i18n("This is the total amount of tiny breaks");
      case LAST_TINY_BREAK: return i18n("This is the time of the last tiny break.");
      case TINY_BREAKS_SKIPPED: return i18n("This is the total amount of tiny breaks "
                          "which you skipped.");
      case IDLENESS_CAUSED_SKIP_TINY: return i18n("This is the total amount of tiny breaks "
                             "which are skipped because you where idle.");
      case BIG_BREAKS: return i18n("This is the total amount of big breaks.");
      case LAST_BIG_BREAK: return i18n("This is the time of the last big break.");
      case BIG_BREAKS_SKIPPED: return i18n("This is the total amount of big breaks "
                          "which you skipped.");
      case IDLENESS_CAUSED_SKIP_BIG: return i18n("This is the total amount of big breaks "
                          "which are skipped because you where idle." );
      case PAUSE_SCORE: return i18n("This is an indication how well you bahaved "
                          "with the brakes.");
      default: ;
    }

    return QString::null;
}
