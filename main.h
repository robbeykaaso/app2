#ifndef REAL_DOCUMENT_H_
#define REAL_DOCUMENT_H_

#include "reactive2.h"

class document;

class comObject : public QJsonObject{
public:
    comObject(const QString& aName, document* aParent);
    QString getID(){
        return value("id").toString();
    }
    QString getType(){
        return value("type").toString();
    }
    void addChild(std::shared_ptr<comObject> aChild){
        m_children.push_back(aChild);
    }
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
    projectObject(const QString& aName, document* aParent) : comObject(aName, aParent){
        insert("type", "project");
    }
};

class folderObject : public comObject{
public:
    folderObject(const QString& aName, document* aParent) : comObject(aName, aParent){
        insert("type", "folder");
    }
};

class pageObject : public comObject{
public:
    pageObject(const QString& aName, document* aParent) : comObject(aName, aParent){
        insert("type", "page");
    }
};

class document{
public:
    document();
private:
    friend comObject;
    void comManagement();
    std::shared_ptr<comObject> m_root_com;
    comObject* m_sel_com = nullptr;
    QHash<QString, comObject*> m_coms;
};

#endif
