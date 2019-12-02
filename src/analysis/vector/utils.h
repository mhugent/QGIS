/***************************************************************************
 *  utils.h                                                                *
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

#ifndef VECTORANALYSIS_GEOPROCESSING_UTILS_H
#define VECTORANALYSIS_GEOPROCESSING_UTILS_H

#include "qgsattributes.h"
#include <QVariant>
#include <QVector>

class QComboBox;
class QFrame;
class QGridLayout;
class QLineEdit;
class QPushButton;
class QRadioButton;

class QgsExpression;
class QgsField;
class QgsFields;
class QgsVectorLayer;
typedef QList<int> QgsAttributeList;
typedef QSet<qint64> QgsFeatureIds;

namespace Vectoranalysis
{
  namespace Utils
  {

#if 0
    QgsVectorLayer *getSelectedLayer( QComboBox *combobox );

///////////////////////////////////////////////////////////////////////////////

    QFrame *createHLine( QWidget *parent = 0 );

///////////////////////////////////////////////////////////////////////////////

#endif //0

    enum SummarizeMode
    {
      SummarizeFirst,
      SummarizeLast,
      SummarizeCount,
      SummarizeSum,
      SummarizeMean,
      SummarizeMin,
      SummarizeMax,
      SummarizeRange,
      SummarizeStdDev,
      SummarizeNull
    };
    typedef QVariant( *summarizer_t )( int, const QVector<QgsAttributes> & );
    summarizer_t getSummarizer( SummarizeMode mode );
    QgsAttributes summarizeAttributes( const QgsFields &fields, const QVector<QgsAttributes> &maps, summarizer_t numericSummarizer, summarizer_t nonNumericSummarizer, const QgsAttributeList &exclude );

#if 0
    class SummarizeUI : public QObject
    {
        Q_OBJECT

      public:
        SummarizeUI( QObject *parent = 0 ) : QObject( parent ) {}
        void setupUI( QGridLayout *layout, SummarizeMode defaultNumericSummarizeMode, SummarizeMode defaultNonNumericSummarizeMode );
        void getSettings( SummarizeMode &numericSummarizeMode, SummarizeMode &nonNumericSummarizeMode ) const;

      private:
        QComboBox *mComboNumericSummarizeMode;
        QComboBox *mComboNonNumericSummarizeMode;
    };

///////////////////////////////////////////////////////////////////////////////
#endif //0

    enum GroupMode
    {
      GroupAll,
      GroupByField,
      GroupByAllFields,
      GroupByExpression,
    };

    QMap<QString, QgsFeatureIds> groupFeatures( QgsVectorLayer *layer, bool selectedOnly, GroupMode mode, int groupField, QgsExpression *groupExpr );

#if 0
    class GroupUI : public QObject
    {
        Q_OBJECT

      public:
        GroupUI( QObject *parent = 0 ) : QObject( parent ) {}
        void setupUI( QComboBox *layerCombo, QGridLayout *layout, const QString &operation = tr( "Group" ) );
        bool getSettings( GroupMode &groupMode, int &groupField, QgsExpression *&groupExpression ) const;
      private:
        QComboBox *mLayerCombo;
        QComboBox *mComboGroupField;
        QLineEdit *mLineeditGroupExpression;
        QPushButton *mButtonGroupExpression;
        QRadioButton *mRadioGroupAll;
        QRadioButton *mRadioGroupField;
        QRadioButton *mRadioGroupExpression;

      private slots:
        void populateAttributeIndexCombo();
        void setExpressionUISensitivity();
        void showExpressionBuilder();
    };
#endif //0

  } // Utils
} // Geoprocessing

#endif // VECTORANALYSIS_GEOPROCESSING_UTILS_H
