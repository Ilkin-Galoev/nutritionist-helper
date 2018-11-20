#include "databasemodule.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QFile>
#include <QDebug>


DatabaseModule::DatabaseModule()
{
    if (!QSqlDatabase::drivers().contains("QSQLITE")){
        qDebug() << "Error: " << Q_FUNC_INFO
                 << "Unable to load database. This program needs the SQLITE driver";
        return;
    }
    _db = QSqlDatabase::addDatabase(_DB_TYPE);

    if (!QFile::exists(_DB_NAME)){
        initEmptyDB();
    }

    _db.setDatabaseName(_DB_NAME);

    if (!_db.open()) {
        qDebug() << "Error: " << Q_FUNC_INFO
                 << _db.lastError().text();
        QFile::remove(_DB_NAME);
        initEmptyDB();
    }
}

unsigned DatabaseModule::addProduct(const ProductEntity &pe)
{
    QSqlQuery q;
    q.prepare("INSERT INTO Products (name, description, proteins, fats, carbohydrates, kkal, units)"
              "VALUES( ?, ?, ?, ?, ?, ?, ?);");
    q.addBindValue(pe.name());
    q.addBindValue(pe.description());
    q.addBindValue(pe.proteins());
    q.addBindValue(pe.fats());
    q.addBindValue(pe.carbohydrates());
    q.addBindValue(pe.kilocalories());
    q.addBindValue(static_cast<int>(pe.units()));
    if(!q.exec()) {
        m_errorList << "Error: in " << Q_FUNC_INFO << q.lastError().text();
        return 0;
    }
    ///
    return q.lastInsertId().toUInt();
}

ProductEntity DatabaseModule::product(unsigned id)
{
    QSqlQuery q;
    q.prepare("SELECT * FROM Products WHERE id=?");
    q.addBindValue(id);
    if(!q.exec()){
        m_errorList << "Error: in " << Q_FUNC_INFO << q.lastError().text();
        ProductEntity();
    }
    ///
    q.next();
    ///
    if(!q.isValid()){
        qDebug() << "Error:" << Q_FUNC_INFO
                 << "In DB has no Product with id:" + QString::number(id);
    }
    ///
    QString name =        q.value("name").toString();
    QString description = q.value("description").toString();
    float proteints =     q.value("proteins").toFloat();
    float fats =          q.value("fats").toFloat();
    float carbohydrates = q.value("carbohydrates").toFloat();
    float kkal =          q.value("kkal").toFloat();
    int units =           q.value("units").toInt();
    return ProductEntity(id, name, description
                         , proteints, fats, carbohydrates, kkal
                         , static_cast<ProductEntity::UnitsType>(units));
}

QVector<ProductEntity> DatabaseModule::products()
{
    QVector<ProductEntity> products;
    ///
    QSqlQuery q("SELECT id FROM Products");
    if(!q.exec()) {
        m_errorList << "Error: in " << Q_FUNC_INFO << q.lastError().text();
        return products;
    }
    while(q.next()) {
        auto prevErrorSize =  m_errorList.size();
        ProductEntity c = this->product(q.value("id").toInt());
        auto avterErrorSize = m_errorList.size();
        if(prevErrorSize == avterErrorSize) {
            products.push_back(c);
        }
    }
    return products;
}

QVector<ProductEntity> DatabaseModule::products(const QStringList &seachLine)
{
    QVector<ProductEntity> products;
    foreach (QString snp, seachLine) {
        QSqlQuery q(QString(" SELECT id FROM Products     "
                            " WHERE name LIKE '%%1%'       "
                            "    OR description LIKE '%%2%'"
                            ).arg(snp).arg(snp));
        if(!q.exec()){
            m_errorList << "Error:" << Q_FUNC_INFO << "Wrong condition" << q.lastQuery();
            return products;
        }

        while(q.next()) {
            auto prevErrorSize =  m_errorList.size();
            ProductEntity c = this->product(q.value("id").toInt());
            auto avterErrorSize = m_errorList.size();
            if(prevErrorSize == avterErrorSize) {
                products.push_back(c);
            }
        }
    }
    return products;
}

QVector<ProductEntity> DatabaseModule::products(QPair<int, int> interval, const char type)
{
    QVector<ProductEntity> products;
    ///
    QString stype, prefix = " WHERE %1 BETWEEN %2 AND %3";
    switch (type) {
    case 'c': { stype = "carbohydrates"; } break;
    case 'f': { stype = "fats"; } break;
    case 'p': { stype = "proteins"; } break;
    case 'k': { stype = "kkal"; } break;
    default: {
        stype = "";
        prefix = "";
    }
    }
    QSqlQuery q("SELECT id FROM Products"
                    "" + prefix.arg(stype).arg(interval.first).arg(interval.second));
    if(!q.exec()){
        m_errorList << "Error:" << Q_FUNC_INFO << "Wrong condition" << q.lastQuery();
        return products;
    }
    while(q.next()) {
        auto prevErrorSize =  m_errorList.size();
        ProductEntity c = this->product(q.value("id").toInt());
        auto avterErrorSize = m_errorList.size();
        if(prevErrorSize == avterErrorSize) {
            products.push_back(c);
        }
    }
    return products;
}

void DatabaseModule::changeProductInformation(const ProductEntity &)
{

}

unsigned DatabaseModule::addRecipe(const RecipeEntity &re)
{
    ///
    QSqlQuery q;
    q.prepare("INSERT INTO Recipes (name)"
              "VALUES( ? );");
    q.addBindValue(re.name());
    if(!q.exec()) {
        qDebug() <<
        m_errorList << "Error: in " << Q_FUNC_INFO << "!q!" << q.lastError().text();
        return 0;
    }
    ///
    auto resipeID = q.lastInsertId().toUInt();
    ///
    const auto& cookingP = re.cookingPoints();
    for (auto i = 0; i < cookingP.size(); ++i) {
        auto pointDescription = cookingP.at(i);
        QSqlQuery q2;
        q2.prepare("INSERT INTO CookingPoints (recipe_id, point_num, description)"
                  "VALUES( ?, ?, ? );");
        qDebug() << resipeID << i << pointDescription;
        q2.addBindValue(resipeID);
        q2.addBindValue(i);
        q2.addBindValue(pointDescription);

        if(!q2.exec()) {
            m_errorList << "Error: in " << Q_FUNC_INFO << "!q2!" << q2.lastError().text();
            return 0;
        }
    }
    ///
    for(const auto& product : re.products()){
        QSqlQuery q3;
        q3.prepare("INSERT INTO ProductsInRecipes (recipe_id, product_id, amound)"
                   "VALUES( ?, ?, ? );");
        q3.addBindValue(resipeID);
        q3.addBindValue(product.product().id());
        q3.addBindValue(product.amound());
        if(!q3.exec()) {
            m_errorList << "Error: in " << Q_FUNC_INFO << "!q3!" << q3.lastError().text();
            return 0;
        }
    }
    ///
    return resipeID;
}

RecipeEntity DatabaseModule::recipe(unsigned recipeId)
{
    QSqlQuery q;
    q.prepare("SELECT id, name FROM Recipes WHERE id=?");
    q.addBindValue(recipeId);
    if(!q.exec()){
        m_errorList << "Error: in " << Q_FUNC_INFO << "!q!" << q.lastError().text();
        ProductEntity();
    }
    q.next();
    if(!q.isValid()){
        qDebug() << "Error:" << Q_FUNC_INFO
                 << "In DB has no Recipe with id:" + QString::number(recipeId);
    }
    QString recipeName = q.value("name").toString();
    ///
    /// Получение пордуктов для рецепта
    ///
    QSqlQuery q2;
    q2.prepare("SELECT Products.id, Products.units, ProductsInRecipes.amound"
                 "      FROM Products INNER JOIN ProductsInRecipes ON ProductsInRecipes.product_id = Products.id "
                 "                                                      AND ProductsInRecipes.recipe_id = ?");
    q2.addBindValue(recipeId);
    if(!q2.exec()){
        qDebug() << "Error: in " << Q_FUNC_INFO << "!q2!" << q2.lastError().text();
        m_errorList << "Error: in " << Q_FUNC_INFO << "!q2!" << q2.lastError().text();
    }
    QVector<WeightedProduct> weightedProducts;
    while(q2.next()){
        auto productId = q2.value("id").toUInt();

        WeightedProduct wproduct(this->product(productId) ,q2.value("amound").toFloat());
        weightedProducts.push_back(wproduct);
    }
    ///
    /// Получение пунктов приготовления
    ///
    QSqlQuery q3;
    q3.prepare("SELECT CookingPoints.description "
               "    FROM CookingPoints"
               "    WHERE CookingPoints.recipe_id = ?");
    q3.addBindValue(recipeId);
    QStringList coockingPoints;
    if(!q3.exec()){
        m_errorList << "Error: in " << Q_FUNC_INFO << "!q3!" << q3.lastError().text();
    }
    while(q3.next()) {
        coockingPoints << q3.value("description").toString();
    }

    return RecipeEntity(recipeId, recipeName, weightedProducts, coockingPoints);
}

QVector<RecipeEntity> DatabaseModule::recipes()
{
    QVector<RecipeEntity> recipe;
    ///
    QSqlQuery q("SELECT id FROM Recipes");
    if(!q.exec()) {
        m_errorList << "Error: in " << Q_FUNC_INFO << q.lastError().text();
        return recipe;
    }
    while(q.next()) {
        auto prevErrorSize =  m_errorList.size();
        RecipeEntity c = this->recipe(q.value("id").toInt());
        auto avterErrorSize = m_errorList.size();
        if(prevErrorSize == avterErrorSize) {
            recipe.push_back(c);
        }
    }
    return recipe;
}

QVector<RecipeEntity> DatabaseModule::recipes(const QStringList &seachLine)
{
    QVector<RecipeEntity> recipes;
    foreach (QString snp, seachLine) {
        QSqlQuery q(QString(" SELECT id FROM Recipes     "
                            " WHERE name LIKE '%%1%'       "
                            ).arg(snp));
        if(!q.exec()){
            m_errorList << "Error:" << Q_FUNC_INFO << "Wrong condition" << q.lastQuery();
            return recipes;
        }

        while(q.next()) {
            auto prevErrorSize =  m_errorList.size();
            RecipeEntity c = this->recipe(q.value("id").toInt());
            auto avterErrorSize = m_errorList.size();
            if(prevErrorSize == avterErrorSize) {
                recipes.push_back(c);
            }
        }
    }
    return recipes;
}

QVector<RecipeEntity> DatabaseModule::recipes(QPair<int, int> interval, const char type)
{
    QString str = "ERROR: FUNCTION [" + QString(Q_FUNC_INFO) + "] IS NOT IMPLEMENTED"
                  "\n >>> empty QVector<RecipeEntity> returned";
    qDebug() << str;
    m_errorList << str;
    return QVector<RecipeEntity>();
}

void DatabaseModule::changeRecipeInformation(const RecipeEntity &)
{

}

unsigned DatabaseModule::addActivity(const ActivityEntity &ae)
{
    QSqlQuery q;
    q.prepare("INSERT INTO Activities (type, kkal_m_km)"
              "VALUES( ?, ? );");
    q.addBindValue(ae.type());
    q.addBindValue(ae.kkm());
    if(!q.exec()){
        m_errorList << "Error: in " << Q_FUNC_INFO << q.lastError().text();
        return 0;
    }
    ///
    return q.lastInsertId().toUInt();
}

ActivityEntity DatabaseModule::activity(unsigned id)
{
    QSqlQuery q;
    q.prepare("SELECT * FROM Activities WHERE id=?");
    q.addBindValue(id);
    if(!q.exec()){
        m_errorList << "Error: in " << Q_FUNC_INFO << q.lastError().text();
        ActivityEntity();
    }
    ///
    q.next();
    ///
    if(!q.isValid()){
        qDebug() << "Error:" << Q_FUNC_INFO
                 << "In DB has no Activity with id:" + QString::number(id);
    }
    ///
    QString type = q.value("type").toString();
    float kkm =  q.value("kkal_m_km").toFloat();

    return ActivityEntity(id, type, kkm);
}

QVector<ActivityEntity> DatabaseModule::activities()
{
    QVector<ActivityEntity> activities;
    ///
    QSqlQuery q("SELECT id FROM Activities");
    if(!q.exec()) {
        m_errorList << "Error: in " << Q_FUNC_INFO << q.lastError().text();
        return activities;
    }
    while(q.next()) {
        auto prevErrorSize =  m_errorList.size();
        ActivityEntity c = this->activity(q.value("id").toInt());
        auto avterErrorSize = m_errorList.size();
        if(prevErrorSize == avterErrorSize) {
            activities.push_back(c);
        }
    }
    return activities;
}

QVector<ActivityEntity> DatabaseModule::activities(const QStringList &seachLine)
{
    QVector<ActivityEntity> activities;
    foreach (QString snp, seachLine) {
        QSqlQuery q(QString(" SELECT id FROM Activities     "
                            " WHERE type LIKE '%1%'       "
                            ).arg(snp));
        if(!q.exec()){
            m_errorList << "Error:" << Q_FUNC_INFO << "Wrong condition" << q.lastQuery();
            return activities;
        }
        while(q.next()) {
            auto prevErrorSize =  m_errorList.size();
            ActivityEntity c = this->activity(q.value("id").toInt());
            auto avterErrorSize = m_errorList.size();
            if(prevErrorSize == avterErrorSize) {
                activities.push_back(c);
            }
        }
    }
    return activities;
}

QVector<ActivityEntity> DatabaseModule::activities(QPair<float, float> kkmInterval)
{
    QVector<ActivityEntity> activities;

    QSqlQuery q(QString("SELECT id FROM Activities "
                        "WHERE kkal_m_km BETWEEN %1 AND %2")
                .arg(kkmInterval.first)
                .arg(kkmInterval.second));
    if(!q.exec()){
        m_errorList << "Error:" << Q_FUNC_INFO << "Wrong condition" << q.lastQuery();
        return activities;
    }
    while(q.next()) {
        auto prevErrorSize =  m_errorList.size();
        ActivityEntity c = this->activity(q.value("id").toInt());
        auto avterErrorSize = m_errorList.size();
        if(prevErrorSize == avterErrorSize) {
            activities.push_back(c);
        }
    }

    return activities;
}

void DatabaseModule::changeActivityInformation(const ActivityEntity &)
{

}

bool DatabaseModule::addExaminationAndSetID(Examination &examination)
{
    QString strValues = "client_id, is_full_examination, date";
    QString questionMSequense;

    QVector<FormField> fieldValues = examination.fields();
    foreach (const FormField& field, fieldValues) {
        strValues += ", " + field.name();
        questionMSequense += ", ?";
    }

    QSqlQuery q;
    q.prepare("INSERT INTO Examinations (" + strValues + ")"
              "VALUES( ?, ?, ?" + questionMSequense + " );");
    q.addBindValue(examination.client().id());
    q.addBindValue(examination.isFullExamination());
    q.addBindValue(examination.date().toString(Qt::ISODate));
    foreach (const FormField& field, fieldValues) {
        q.addBindValue(field.value());
    }
    if(!q.exec()) {
        qDebug() << "Error: in addExamination(...)\n\t"
                 <<  q.lastError().text();
        return false;
    }

    examination.setId(q.lastInsertId().toInt());

    return true;
}

bool DatabaseModule::addClientAndSetID(Client &client)
{
    QSqlQuery q;
    q.prepare("INSERT INTO Clients (surname, name, patronymic, birth_date, gender, age, tel_number)"
              "VALUES( ?, ?, ?, ?, ?, ?, ? );");
    q.addBindValue(client.surname());
    q.addBindValue(client.name());
    q.addBindValue(client.patronymic());
    q.addBindValue(client.birthDate().toString(Qt::ISODate));
    q.addBindValue(client.gender());
    q.addBindValue(client.age());
    q.addBindValue(client.telNumber());
    if(!q.exec()) {
        qDebug() << "Error: in addClient(...)\n\t" <<  q.lastError().text();
        return false;
    }

    client.setId(q.lastInsertId().toInt());

    return true;
}

bool DatabaseModule::changeClientInformation(const Client &client)
{
    bool isFoundClient = false;
    this->client(client.id(), isFoundClient);
    if (!isFoundClient){
        return false;
    }

    QSqlQuery q;
    q.prepare("UPDATE Clients "
              "SET surname = ?, name = ?, patronymic = ?, birth_date = ?, gender = ?, age = ?, tel_number = ?"
              "WHERE id = ?"
              );
    q.addBindValue(client.surname());
    q.addBindValue(client.name());
    q.addBindValue(client.patronymic());
    q.addBindValue(client.birthDate().toString(Qt::ISODate));
    q.addBindValue(client.gender());
    q.addBindValue(client.age());
    q.addBindValue(client.telNumber());
    q.addBindValue(client.id());

    if(!q.exec()) {
        qDebug() << "Error: in changeClientInformation(...)\n\t" <<  q.lastError().text();
        return false;
    }

    return true;
}

Client DatabaseModule::client(int id, bool& isOk) const
{
    QSqlQuery q;
    q.prepare("SELECT * FROM Clients WHERE id=?");
    q.addBindValue(id);
    if(!q.exec()) {
        qDebug() << "Error: DatabaseModule::client(int, bool)";
        isOk = false;
        return Client();
    }

    q.next();

    if(!q.isValid()){
        qDebug() << "Error:" << Q_FUNC_INFO
                 << "In DB has no Client with id:" + QString::number(id);
        isOk = false;
        return Client();
    }

    QString surname = q.value("surname").toString();
    QString name = q.value("name").toString();
    QString patronymic = q.value("patronymic").toString();
    QChar gender = q.value("gender").toString().at(0);
    QDate birthDate = QDate::fromString(q.value("birth_date").toString(), Qt::ISODate);
    short age = q.value("age").toInt();
    QString telNumber = q.value("tel_number").toString();

    isOk = true;

    return Client(id, name, surname, patronymic, birthDate, gender, age, telNumber);
}

QVector<Client> DatabaseModule::clients(const QString& snp) const
{
    QStringList snpList = snp.toLower().split(QRegExp("[\\s,.]+"), QString::SkipEmptyParts);

    QVector<Client> clients;

    foreach (QString snp, snpList) {
        snp[0] = snp[0].toUpper();

        QSqlQuery q(QString(" SELECT id FROM Clients       "
                            " WHERE surname LIKE '%1%'    "
                            "    OR name LIKE '%2%'       "
                            "    OR patronymic LIKE '%3%' "
                            ).arg(snp).arg(snp).arg(snp));

        if(!q.exec()){
            qDebug() << "Error:" << Q_FUNC_INFO
                     << "Wrong condition";
            qDebug() << q.lastQuery();
            return clients;
        }

        while(q.next()) {
            bool isOk;
            Client client = this->client(q.value("id").toInt(), isOk);
            clients.push_back(client);
        }
    }

    return clients;
}

QVector<Client> DatabaseModule::clients() const
{
    QVector<Client> clients;

    QSqlQuery q("SELECT id FROM Clients");
    if(!q.exec()) {
        qDebug() << "Error: DatabaseModule::clients()";
        return clients;
    }

    while(q.next()) {
        bool isOk;
        Client c = this->client(q.value("id").toInt(), isOk);
        if(isOk) {
            clients.push_back(c);
        }
    }

    return clients;
}

Examination DatabaseModule::examination(int id, bool& isOk, Client client) const
{
    Examination examination;

    QSqlQuery q;
    q.prepare("SELECT * FROM Examinations WHERE id=?");
    q.addBindValue(id);
    if(!q.exec()) {
        qDebug() << "Error: DatabaseModule::examination(int, bool)";
        isOk = false;
        return examination;
    }
    q.next();

    if (!client.isInit()) {
        client = this->client(q.value("client_id").toInt(), isOk);
        if (!isOk) {
            qDebug() << "Error: DatabaseModule::examination(int, bool)"
                     << "'client' information doesn't found";
            return examination;
        }
    }
    examination.setClient(client);

    examination.setId(q.value("id").toInt());
    examination.setIsFullExamination(q.value("is_full_examination").toBool());
    examination.setDate(QDateTime::fromString(q.value("date").toString(), Qt::ISODate));

    foreach (FormField field, examination.fields()) {
        QString fieldValue = q.value(field.name()).toString();
        examination.setFieldValue(field.name(), fieldValue);
    }

    isOk = true;
    return examination;
}

QVector<Examination> DatabaseModule::examinations(Client client) const
{
    QVector<Examination> examinations;

    QString addQuery = client.isInit() ? "WHERE client_id=" + QString::number(client.id())
                                       : "";
    QSqlQuery q("SELECT id FROM Examinations " + addQuery);
    if(!q.exec()) {
        qDebug() << "DatabaseModule::examinations(const Client &)";
        return examinations;
    }

    while(q.next()) {
        bool isOk;
        Examination exm = this->examination(q.value("id").toInt(), isOk, client);
        if (isOk) {
            examinations.push_back(exm);
        }
    }

    return examinations;
}

QVector<Examination> DatabaseModule::examinations(QDate from, QDate to) const
{
    QDateTime fromT(from), toT(to, QTime(23, 59, 59));
    QVector<Examination> examinations;

    QSqlQuery q;
    q.prepare("SELECT id FROM Examinations WHERE date BETWEEN ? AND ?");
    q.addBindValue(fromT.toString(Qt::ISODate));
    q.addBindValue(toT.toString(Qt::ISODate));
    if(!q.exec()) {
        qDebug() << "DatabaseModule::examinations(QDate from, QDate to)";
        return examinations;
    }

    while(q.next()) {
        bool isOk;
        Examination exm = this->examination(q.value("id").toInt(), isOk);
        if (isOk) {
            examinations.push_back(exm);
        }
    }

    return examinations;
}

bool DatabaseModule::importDB(const QString &fileName)
{
    if (fileName.isEmpty()){
        return false;
    }

    /// Is it possible to read the file?
    _db.setDatabaseName(fileName);
    if (!_db.open()) {
        qDebug() << "Warning:" << Q_FUNC_INFO
                 << _db.lastError().text();
        /// If the new new DB file could not be seted
        /// set the previous DB file
        _db.setDatabaseName(_DB_NAME);
        return false;
    }

    if(QFile::exists(_DB_NAME)){
        if(!QFile::remove(_DB_NAME)){
            qDebug() << "Error:" << Q_FUNC_INFO
                     <<  "Old file can not be deleted";
            return false;
        }
    } else {
        qDebug() << "Warning:" << Q_FUNC_INFO
                 << "The old file is not exist";
    }

    if(!QFile::copy(fileName, _DB_NAME)){
        qDebug() << "Error:" << Q_FUNC_INFO
                 <<  "New file can not be copied";
        return false;
    }

    _db.setDatabaseName(_DB_NAME);

    //DatabaseModule();

    return true;
}

bool DatabaseModule::exportDB(const QString &fileName)
{
    if(!QFile::copy(_DB_NAME, fileName)){
        qDebug() << "Error:" << Q_FUNC_INFO
                 <<  "New file can not be copied";
        return false;
    }

    return true;
}

bool DatabaseModule::hasUnwatchedWorkError()
{
    return !m_errorList.isEmpty();
}

QStringList DatabaseModule::unwatchedWorkError()
{
    auto ret = m_errorList;
    m_errorList.clear();
    return ret;
}

void DatabaseModule::initEmptyDB()
{
    _db.setDatabaseName(_DB_NAME);
    QSqlQuery query;

    if (!_db.open()) {
        qDebug() << "Error: " << Q_FUNC_INFO
                 << _db.lastError().text();
    }

    QString q1("CREATE TABLE `Clients` ("
               "`id`           INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"
               "`surname`      TEXT NOT NULL,"
               "`name`         TEXT NOT NULL,"
               "`patronymic`	TEXT NOT NULL,"
               "`birth_date`	TEXT NOT NULL,"
               "`gender`       TEXT NOT NULL,"
               "`age`          INTEGER NOT NULL,"
               "`tel_number`	TEXT NOT NULL"
               ");"
               );
    if(!query.exec(q1)){
        qDebug() << "Error:" << Q_FUNC_INFO
                 << "DB was not init new empty table";
    }

    QString formfieldNames;
    Examination exm;
    foreach (FormField field, exm.fields()) {
        formfieldNames += QString(" `%1` TEXT, ").arg(field.name());
    }

    QString q2("CREATE TABLE `Examinations` ("
               "`id`	INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"
               "`client_id`	INTEGER NOT NULL,"
               "`is_full_examination`	INTEGER NOT NULL,"
               "`date`	TEXT NOT NULL,"
               + formfieldNames +
               "FOREIGN KEY(`client_id`) REFERENCES `Clients`(`id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ");"
               );
    if(!query.exec(q2)){
        qDebug() << "Error:" << Q_FUNC_INFO
                 << "DB was not init new empty table";
    }
}
