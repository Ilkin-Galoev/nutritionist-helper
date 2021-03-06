#pragma once
#include <QVector>
#include <QString>
#include <QDate>
#include <QtSql/QSqlDatabase>

#include "entities/client.h"
#include "entities/examination.h"
#include "entities/product.h"
#include "entities/recipe.h"
#include "entities/activity.h"

class DatabaseModule
{
public:
    DatabaseModule();

    /* functions to work with Product entities */
    unsigned                addProduct(const ProductEntity& );
    void                    deleteProduct(const ProductEntity& );
    ProductEntity           product(unsigned id);
    QVector<ProductEntity>  products();
    QVector<ProductEntity>  products(const QStringList &seachLine);
    QVector<ProductEntity>  products(QPair<float,float> interval, const char type); /// type: {c, f, p, k}
    void                    changeProductInformation(const ProductEntity& );

    /* functions to work with Recipe entities */
    unsigned                addRecipe(const RecipeEntity& );
    void                    deleteRecipe(const RecipeEntity& );
    RecipeEntity            recipe(unsigned id);
    QVector<RecipeEntity>   recipes();
    QVector<RecipeEntity>   recipes(const QStringList &seachLine);
    QVector<RecipeEntity>   recipes(QPair<float,float> interval, const char type); /*<- NOT IMPLEMENTED*/  /// type: {c, f, p, k}
    void                    changeRecipeInformation(const RecipeEntity& );

    /* functions to work with Activity entities */
    unsigned                addActivity(const ActivityEntity& );
    void                    deleteActivity(const ActivityEntity& );
    ActivityEntity          activity(unsigned id);
    QVector<ActivityEntity> activities();
    QVector<ActivityEntity> activities(const QStringList &seachLine);
    QVector<ActivityEntity> activities(QPair<float,float> kkmInterval);                              // kkm - kkal/kg/min
    void                    changeActivityInformation(const ActivityEntity& );

    /* functions to work with Client entities */
    bool                    addClientAndSetID(Client & );                       //TODO: Need to be change as in the previous style
    void                    deleteClient(const Client& );
    Client                  client(int id, bool &isOk) const;
    QVector<Client>         clients(const QString& snp) const;
    QVector<Client>         clients() const;
    bool                    changeClientInformation(const Client& );

    /* functions to work with Examination entities */
    bool                    addExaminationAndSetID(Examination& );      //TODO: Need to be change as in the previous style
    void                    deleteExamination(const Examination& );
    Examination             examination(int id, bool &isOk, Client client = Client()) const;
    QVector<Examination>    examinations(Client client = Client()) const;
    QVector<Examination>    examinations(QDate from, QDate to) const;
    bool                    changeExaminationInformation(Examination & ); //without id, client_id, is_full_examination, date

    /* Specific database functions */
    bool importDB(const QString& fileName);
    bool exportDB(const QString& fileName);

    bool hasUnwatchedWorkError();           //Lets you know if there was an Unwatched Error at DataBase job time
    QStringList unwatchedWorkError();

private:
    QSqlDatabase    _db;
    const QString   _DB_TYPE = "QSQLITE";
    //const QString   _DB_NAME = "../project/database/db.sqlite";  //INFO : For DEBUG :TODO :WARNING
    const QString   _DB_NAME = "./database/db.sqlite";
    //const QString   _DB_NAME = "/Users/ilkin_galoev/Documents/7 semester/Fundamentals of Software Engineering/nutritionist-helper/project/database/db.sqlite";
    QStringList     m_errorList;

    void initEmptyDB();
    bool insertIntoCookingPoints(unsigned recipeId, const QStringList& );
    bool insertIntoProductsInRecipes(unsigned recipeId, const QVector<WeightedProduct>& );
};
