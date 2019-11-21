/***************************************************************************
 *  utils.cpp                                                              *
 *  -------------------                                                    *
 *  begin                : Jun 10, 2014                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "utils.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#include <qmath.h>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>

namespace Geoprocessing
{
  namespace Utils
  {
#if 0
    QgsVectorLayer* getSelectedLayer( QComboBox *combobox )
    {
      int inputIdx = combobox->currentIndex();
      if ( inputIdx < 0 )
      {
        return 0;
      }
      QString inputLayerId = combobox->itemData( inputIdx ).toString();
      return static_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( inputLayerId ) );
    }

///////////////////////////////////////////////////////////////////////////////

    QFrame* createHLine( QWidget *parent )
    {
      QFrame* line = new QFrame( parent );
      line->setFrameShape( QFrame::HLine );
      line->setFrameShadow( QFrame::Sunken );
      return line;
    }

///////////////////////////////////////////////////////////////////////////////
 #endif //0
    static QVariant summarizeFirst( int key, const QVector<QgsAttributes>& maps )
    {
      return maps.first().value( key );
    }

    static QVariant summarizeLast( int key, const QVector<QgsAttributes>& maps )
    {
      return maps.last().value( key );
    }

    static QVariant summarizeCount( int key, const QVector<QgsAttributes>& maps )
    {
      int count = 0;
      for ( const QgsAttributes& map : maps )
      {
        count += !map.value( key ).isNull();
      }
      return QVariant( count );
    }

    static QVariant summarizeSum( int key, const QVector<QgsAttributes>& maps )
    {
      double sum = 0.0;
      for ( const QgsAttributes& map : maps )
      {
        sum += map.value( key ).toDouble();
      }
      return QVariant( sum );
    }

    static QVariant summarizeMean( int key, const QVector<QgsAttributes>& maps )
    {
      double sum = 0.0;
      for ( const QgsAttributes& map : maps )
      {
        sum += map.value( key ).toDouble();
      }
      return QVariant( sum / maps.size() );
    }

    static QVariant summarizeMin( int key, const QVector<QgsAttributes>& maps )
    {
      double min = maps.first().value( key ).toDouble();
      for( const QgsAttributes& map : maps )
      {
        min = qMin( min, map.value( key ).toDouble() );
      }
      return QVariant( min );
    }

    static QVariant summarizeMax( int key, const QVector<QgsAttributes>& maps )
    {
      double max = maps.first().value( key ).toDouble();
      for ( const QgsAttributes& map : maps )
      {
        max = qMax( max, map.value( key ).toDouble() );
      }
      return QVariant( max );
    }

    static QVariant summarizeRange( int key, const QVector<QgsAttributes>& maps )
    {
      double min = maps.first().value( key ).toDouble();
      double max = min;
      for ( const QgsAttributes& map : maps )
      {
        min = qMin( min, map.value( key ).toDouble() );
        max = qMax( max, map.value( key ).toDouble() );
      }
      return QVariant( max - min );
    }

    static QVariant summarizeStdDev( int key, const QVector<QgsAttributes>& maps )
    {
      double sum = 0.0;
      double sum2 = 0.0;
      for ( const QgsAttributes& map : maps )
      {
        double val = map.value( key ).toDouble();
        sum += val;
        sum2 += val * val;
      }
      double n = maps.size();
      return QVariant( qSqrt( sum2 / n - ( sum * sum ) / ( n * n ) ) );
    }

    static QVariant summarizeNull( int /*key*/, const QVector<QgsAttributes>& /*maps*/ )
    {
      return QVariant();
    }

    summarizer_t getSummarizer( SummarizeMode mode )
    {
      switch ( mode )
      {
        case SummarizeFirst:
          return summarizeFirst;
        case SummarizeLast:
          return summarizeLast;
        case SummarizeSum:
          return summarizeSum;
        case SummarizeMean:
          return summarizeMean;
        case SummarizeMin:
          return summarizeMin;
        case SummarizeMax:
          return summarizeMax;
        case SummarizeRange:
          return summarizeRange;
        case SummarizeStdDev:
          return summarizeStdDev;
        case SummarizeCount:
          return summarizeCount;
        case SummarizeNull:
        default:
          return summarizeNull;
      }
    }

    QgsAttributes summarizeAttributes( const QgsFields& fields, const QVector<QgsAttributes>& attribs, summarizer_t numericSummarizer, summarizer_t nonNumericSummarizer, const QgsAttributeList &exclude )
    {
      QgsAttributes output;
      for ( int idx = 0, nIdx = fields.count(); idx < nIdx; ++idx )
      {
        // For excluded fields, take the values of the first map
        if ( exclude.contains( idx ) )
        {
          output.insert( idx, attribs.first().value( idx ) );
        }
        else
        {
          const QgsField& field = fields[ idx ];
          // Distinguish between numeric types and other types
          switch ( field.type() )
          {
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::LongLong:
            case QVariant::ULongLong:
            case QVariant::Double:
              output.insert( idx, numericSummarizer( idx, attribs ) );
              break;
            default:
              output.insert( idx, nonNumericSummarizer( idx, attribs ) );
          }
        }
      }
      return output;
    }

#if 0
    void SummarizeUI::setupUI( QGridLayout *layout, SummarizeMode defaultNumericSummarizeMode, SummarizeMode defaultNonNumericSummarizeMode )
    {
      mComboNumericSummarizeMode = new QComboBox();
      mComboNumericSummarizeMode->addItem( tr( "First" ),  SummarizeFirst );
      mComboNumericSummarizeMode->addItem( tr( "Last" ),   SummarizeLast );
      mComboNumericSummarizeMode->addItem( tr( "Count" ),  SummarizeCount );
      mComboNumericSummarizeMode->addItem( tr( "Sum" ),    SummarizeSum );
      mComboNumericSummarizeMode->addItem( tr( "Mean" ),   SummarizeMean );
      mComboNumericSummarizeMode->addItem( tr( "Min" ),    SummarizeMin );
      mComboNumericSummarizeMode->addItem( tr( "Max" ),    SummarizeMax );
      mComboNumericSummarizeMode->addItem( tr( "Range" ),  SummarizeRange );
      mComboNumericSummarizeMode->addItem( tr( "StdDev" ), SummarizeStdDev );
      mComboNumericSummarizeMode->addItem( tr( "Null" ),   SummarizeNull );
      mComboNumericSummarizeMode->setCurrentIndex( defaultNumericSummarizeMode );

      mComboNonNumericSummarizeMode = new QComboBox();
      mComboNonNumericSummarizeMode->addItem( tr( "First" ), SummarizeFirst );
      mComboNonNumericSummarizeMode->addItem( tr( "Last" ),  SummarizeLast );
      mComboNonNumericSummarizeMode->addItem( tr( "Null" ),  SummarizeNull );
      mComboNonNumericSummarizeMode->setCurrentIndex( defaultNonNumericSummarizeMode == SummarizeNull ? 2 : defaultNonNumericSummarizeMode );

      int row = layout->rowCount();
      layout->addWidget( new QLabel( tr( "Summarize numeric attributes:" ) ), row, 0, 1, 1 );
      layout->addWidget( mComboNumericSummarizeMode, row, 1, 1, 1 );
      layout->addWidget( new QLabel( tr( "Summarize non-numeric attributes:" ) ), row + 1, 0, 1, 1 );
      layout->addWidget( mComboNonNumericSummarizeMode, row + 1, 1, 1, 1 );
    }

    void SummarizeUI::getSettings( SummarizeMode &numericSummarizeMode, SummarizeMode &nonNumericSummarizeMode ) const
    {
      int i1 = mComboNumericSummarizeMode->currentIndex();
      numericSummarizeMode = static_cast<Utils::SummarizeMode>( mComboNumericSummarizeMode->itemData( i1 ).toInt() );
      int i2 = mComboNonNumericSummarizeMode->currentIndex();
      nonNumericSummarizeMode = static_cast<Utils::SummarizeMode>( mComboNonNumericSummarizeMode->itemData( i2 ).toInt() );
    }

///////////////////////////////////////////////////////////////////////////////
#endif //0

    static QString allAttributes( QgsVectorLayer* layer, const QgsFeature& feature )
    {
      QStringList values;
      for ( int idx = 0, nIdx = layer->fields().count(); idx < nIdx; ++idx )
      {
        if ( !layer->dataProvider()->pkAttributeIndexes().contains( idx ) )
        {
          values.append( feature.attribute( idx ).toString().replace( '\\', "\\\\" ).replace( ':', "\\:" ) );
        }
      }
      return values.join( "::" );
    }

    QMap<QString, QgsFeatureIds> groupFeatures( QgsVectorLayer* layer, bool selectedOnly, GroupMode mode, int groupField, QgsExpression* groupExpr )
    {
      QMap<QString, QgsFeatureIds> clusters;
      if ( mode == GroupAll )
      {
        if ( selectedOnly )
        {
          clusters.insert( QString(), layer->selectedFeatureIds() );
        }
        else
        {
          clusters.insert( QString(), layer->allFeatureIds() );
        }
      }
      else if ( mode == GroupByField )
      {
        QgsFeature f;
        if ( selectedOnly )
        {
          for( const QgsFeatureId& id : layer->selectedFeatureIds() )
          {
            QgsFeatureRequest req( id );
            req.setFlags( QgsFeatureRequest::NoGeometry );
            if ( layer->getFeatures( req ).nextFeature( f ) )
            {
              clusters[f.attribute( groupField ).toString()].insert( f.id() );
            }
          }
        }
        else
        {
          QgsFeatureRequest req;
          req.setFlags( QgsFeatureRequest::NoGeometry );
          QgsFeatureIterator it = layer->getFeatures( req );
          while ( it.nextFeature( f ) )
          {
            clusters[f.attribute( groupField ).toString()].insert( f.id() );
          }
        }
      }
      else if ( mode == GroupByAllFields )
      {
        QgsFeature f;
        if ( selectedOnly )
        {
          for ( const QgsFeatureId& id : layer->selectedFeatureIds() )
          {
            QgsFeatureRequest req( id );
            req.setFlags( QgsFeatureRequest::NoGeometry );
            if ( layer->getFeatures( req ).nextFeature( f ) )
            {
              clusters[allAttributes( layer, f )].insert( f.id() );
            }
          }
        }
        else
        {
          QgsFeatureRequest req;
          req.setFlags( QgsFeatureRequest::NoGeometry );
          QgsFeatureIterator it = layer->getFeatures( req );
          while ( it.nextFeature( f ) )
          {
            clusters[allAttributes( layer, f )].insert( f.id() );
          }
        }
      }
      else if ( mode == GroupByExpression )
      {
        QgsFeature f;
        if ( selectedOnly )
        {
          for ( const QgsFeatureId& id : layer->selectedFeatureIds() )
          {
            QgsFeatureRequest req( id );
            req.setFlags( QgsFeatureRequest::NoGeometry );
            if ( layer->getFeatures( req ).nextFeature( f ) )
            {
              QgsExpressionContext ctx; ctx.setFeature( f ); ctx.setFields( layer->fields() );
              clusters[groupExpr->evaluate( &ctx ).toString()].insert( f.id() );
            }
          }
        }
        else
        {
          QgsFeatureRequest req;
          req.setFlags( QgsFeatureRequest::NoGeometry );
          QgsFeatureIterator it = layer->getFeatures( req );
          while ( it.nextFeature( f ) )
          {
            QgsExpressionContext ctx; ctx.setFeature( f ); ctx.setFields( layer->fields() );
            clusters[groupExpr->evaluate( &ctx ).toString()].insert( f.id() );
          }
        }
      }

      return clusters;
    }

#if 0

    void GroupUI::setupUI( QComboBox *layerCombo, QGridLayout *layout, const QString& operation )
    {
      mLayerCombo = layerCombo;

      mRadioGroupAll = new QRadioButton( tr( "%1 all" ).arg( operation ) );
      mRadioGroupAll->setChecked( true );
      mRadioGroupField = new QRadioButton( tr( "%1 by attribute:" ).arg( operation ) );
      mRadioGroupExpression = new QRadioButton( tr( "%1 by expression:" ).arg( operation ) );

      mLineeditGroupExpression = new QLineEdit();
      mLineeditGroupExpression->setReadOnly( true );
      mLineeditGroupExpression->setEnabled( false );

      mButtonGroupExpression = new QPushButton( "..." );
      mButtonGroupExpression->setEnabled( false );

      QWidget* widgetGroupExpression = new QWidget();
      widgetGroupExpression->setLayout( new QHBoxLayout() );
      widgetGroupExpression->layout()->setMargin( 0 );
      widgetGroupExpression->layout()->addWidget( mLineeditGroupExpression );
      widgetGroupExpression->layout()->addWidget( mButtonGroupExpression );

      mComboGroupField = new QComboBox();
      mComboGroupField->setEnabled( false );

      int row = layout->rowCount();
      layout->addWidget( mRadioGroupAll, row, 0, 1, 2 );
      layout->addWidget( mRadioGroupField, row + 1, 0, 1, 1 );
      layout->addWidget( mComboGroupField, row + 1, 1, 1, 1 );
      layout->addWidget( mRadioGroupExpression, row + 2, 0, 1, 1 );
      layout->addWidget( widgetGroupExpression, row + 2, 1, 1, 1 );

      connect( mLayerCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( populateAttributeIndexCombo() ) );
      connect( mLayerCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setExpressionUISensitivity() ) );
      connect( mRadioGroupField, SIGNAL( toggled( bool ) ), mComboGroupField, SLOT( setEnabled( bool ) ) );
      connect( mRadioGroupExpression, SIGNAL( toggled( bool ) ), this, SLOT( setExpressionUISensitivity() ) );
      connect( mButtonGroupExpression, SIGNAL( clicked() ), this, SLOT( showExpressionBuilder() ) );

      populateAttributeIndexCombo();
    }

    bool GroupUI::getSettings( GroupMode &groupMode, int& groupField, QgsExpression *&groupExpression ) const
    {
      groupField = mRadioGroupField->isChecked() ? mComboGroupField->itemData( mComboGroupField->currentIndex() ).toInt() : -1;
      groupMode =
        mRadioGroupAll->isChecked() ? Utils::GroupAll :
        mRadioGroupField->isChecked() ? ( groupField == Utils::GroupByAllFields ? Utils::GroupByAllFields : Utils::GroupByField ) :
            /*mRadioDissolveExpression->isChecked() ?*/ Utils::GroupByExpression;

      groupExpression = 0;
      if ( mRadioGroupExpression->isChecked() )
  {
        groupExpression = new QgsExpression( mLineeditGroupExpression->text() );
        if ( groupExpression->hasParserError() )
        {
          QMessageBox::warning( mRadioGroupExpression, tr( "Invalid Expression" ), tr( "Invalid expression:\n%1" ).arg( groupExpression->parserErrorString() ) );
          return false;
        }
        else if ( groupExpression->hasEvalError() )
        {
          QMessageBox::warning( mRadioGroupExpression, tr( "Invalid Expression" ), tr( "Invalid expression:\n%1" ).arg( groupExpression->evalErrorString() ) );
          return false;
        }
      }
      return true;
    }

    void GroupUI::populateAttributeIndexCombo()
    {
      QString prev = mComboGroupField->currentText();
      int selIdx = 0;
      mComboGroupField->clear();
      QgsVectorLayer* layer = getSelectedLayer( mLayerCombo );
      if ( layer )
      {
        mComboGroupField->addItem( tr( "All" ), Utils::GroupByAllFields );
        mComboGroupField->insertSeparator( 1 );

        const QgsFields& fields = layer->pendingFields();
        for ( int index = 0, nIdx = fields.count(); index < nIdx; ++index )
        {
          // Don't list primary keys...
          if ( !layer->dataProvider()->pkAttributeIndexes().contains( index ) )
          {
            if ( fields[index].name() == prev )
            {
              selIdx = mComboGroupField->count();
            }
            mComboGroupField->addItem( fields[index].name(), index );
          }
        }
        mComboGroupField->setCurrentIndex( selIdx );
      }
    }

    void GroupUI::setExpressionUISensitivity()
    {
      bool enabled = mRadioGroupExpression->isChecked() && getSelectedLayer( mLayerCombo ) != 0;
      mLineeditGroupExpression->setEnabled( enabled );
      mButtonGroupExpression->setEnabled( enabled );

    }

    void GroupUI::showExpressionBuilder()
    {
      QgsVectorLayer* layer = getSelectedLayer( mLayerCombo );
      QgsExpressionBuilderDialog dialog( layer, mLineeditGroupExpression->text(), mLineeditGroupExpression );
      if ( dialog.exec() == QDialog::Accepted )
      {
        mLineeditGroupExpression->setText( dialog.expressionText() );
      }
    }
#endif //0

  } // Utils
} // Geoprocessing
