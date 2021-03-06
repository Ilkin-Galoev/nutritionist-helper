#ifndef RECIPEINFO_H
#define RECIPEINFO_H

#include <QWidget>
#include "entities/recipe.h"
#include "entities/product.h"

class QPrinter;

namespace Ui {
class RecipeInfo;
}

class RecipeInfo : public QWidget
{
    Q_OBJECT

public:
    explicit RecipeInfo(QWidget *parent = nullptr);
    ~RecipeInfo();

    void setInformation(const RecipeEntity& );
    RecipeEntity recipe() const;

   void paintEvent(QPaintEvent *event) override;

signals:
    void editRecipeButtonPressed();
    void deleteRecipeButtonPressed();

private slots:
    void onPrintButtonPressed();
    void printRequest(QPrinter* printer);

private:
    Ui::RecipeInfo *ui;
    RecipeEntity _recipe;
};

#endif // RECIPEINFO_H
