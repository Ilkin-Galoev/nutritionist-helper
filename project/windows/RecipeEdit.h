#pragma once
#include <QWidget>
#include "entities/recipe.h"
#include "windows/ProductSeach.h"

namespace Ui {
class RecipeEdit;
}

class RecipeEdit : public QWidget
{
    Q_OBJECT

public:
    explicit RecipeEdit(QWidget *parent = nullptr);
    ~RecipeEdit();

    void setInformation(const RecipeEntity& );
    RecipeEntity recipe() const;

signals:
    void formNewRecipeReady();
    void formEditedRecipeReady();
    void productSearchLineReady(const QString& );
    void productSearchProteinReady(const int from, const int to);
    void productSearchFatsReady(const int from, const int to);
    void productSearchCarbohydratesReady(const int from, const int to);
    void productSelectedForShow();

private slots:
    void onPushButtonSave();
    void onPushButtonCancel();
    void onPushButtonAddIngredient();
    void onPushButtoDeleteIngredient();
    void onPushButtonAddDescription();
    void onPushButtonDeleteDescription();

private:
    Ui::RecipeEdit *ui;
    RecipeEntity _recipe;
    bool _isEditingMod = false;

    ProductSeach* _productSeach;
};
