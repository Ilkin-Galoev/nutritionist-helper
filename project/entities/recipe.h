#pragma once
#include <QString>
#include <QStringList>
#include <QVector>

#include "product.h"

class RecipeEntity
{
public:
    RecipeEntity() = delete;
    RecipeEntity(int id, QString name, QVector<WeightedProduct> products, QStringList cookingPoints);

    int id() const;
    QString name() const;
    QVector<WeightedProduct> products() const;
    QStringList cookingPoints() const;

private:
    int                       m_id;
    QString                   m_name;
    QVector<WeightedProduct>  m_products;
    QStringList               m_cookingPoints;
};