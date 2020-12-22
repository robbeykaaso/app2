#ifndef REAL_DOCUMENT_H_
#define REAL_DOCUMENT_H_

#include "reactive2.h"

class document;

class comObject : public QJsonObject{
public:
    comObject(const QString& aName, document* aParent, const QString& aID = "");
    QString getID(){
        return value("id").toString();
    }
    QString getType(){
        return value("type").toString();
    }
    void addChild(std::shared_ptr<comObject> aChild){
        m_children.push_back(aChild);
    }
    void removeChild(const QString& aID);
    document* getParent(){
        return m_parent;
    }
    QJsonObject generateDocument();
protected:
    std::vector<std::shared_ptr<comObject>> m_children;
private:
    document* m_parent;
};

class projectObject : public comObject{
public:
    projectObject(const QString& aName, document* aParent, const QString& aID = "") : comObject(aName, aParent, aID){
        insert("type", "project");
    }
};

class folderObject : public comObject{
public:
    folderObject(const QString& aName, document* aParent, const QString& aID = "") : comObject(aName, aParent, aID){
        insert("type", "folder");
    }
};

class pageObject : public comObject{
public:
    pageObject(const QString& aName, document* aParent, const QString& aID = "") : comObject(aName, aParent, aID){
        insert("type", "page");
    }
};

class imageObject : public comObject{
public:
    imageObject(const QString& aName, document* aParent, const QString& aID = "") : comObject(aName, aParent, aID){
        insert("type", "image");
        insert("position", QJsonArray());
        insert("comment", "");
        insert("source", "");
    }
};

class textObject : public comObject{
public:
    textObject(const QString& aName, document* aParent, const QString& aID = "") : comObject(aName, aParent, aID){
        insert("type", "text");
        insert("position", QJsonArray());
        insert("comment", "");
        insert("content", "");
        insert("size", 16);
        insert("color", "green");
        insert("bold", "");
    }
};

class shapeObject : public comObject{
public:
    shapeObject(const QString& aName, document* aParent, const QString& aID = "") : comObject(aName, aParent, aID){
        insert("type", "shape");
        insert("comment", "");
        insert("position", QJsonArray());
        insert("direction", rea::Json(
                                "color", "green",
                                "bolder", rea::Json(
                                              "type", "line",
                                              "color", "red"),
                                "radius", 30));
    }
};

class document{
public:
    document();
private:
    friend comObject;
    void comManagement();
    QJsonObject preparePageView(const QJsonObject& aPageConfig);
    std::shared_ptr<comObject> m_root_com;
    comObject* m_sel_com = nullptr;
    QHash<QString, comObject*> m_coms;

    QJsonObject m_page_template;
    QJsonObject m_param_template;
    QJsonObject m_shape_template;
};

#endif
