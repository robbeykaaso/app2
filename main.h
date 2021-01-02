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
    QString getName(){
        return value("name").toString();
    }
    void initialize(const QJsonObject& aParam){
        for (auto i : aParam.keys())
            insert(i, aParam.value(i));
    }
    std::shared_ptr<comObject> addChild(std::shared_ptr<comObject> aChild){
        m_children.push_back(aChild);
        return aChild;
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

#define OBJTYPE(aType) \
class aType##Object : public comObject{ \
public: \
    aType##Object(const QString& aName, document* aParent, const QString& aID = "") : comObject(aName, aParent, aID){ \
        insert("type", #aType); \
    } \
};

class document{
public:
    document();
private:
    friend comObject;
    void comManagement();
    void frontManagement();
    void backManagement();
    void initializeTemplate();
    void regCreateShape(const QString& aType);
    QJsonObject prepareEventList(const std::vector<std::shared_ptr<comObject>>& aList, int aSelect);
    QJsonObject preparePageView(const QJsonObject& aPageConfig);
    QJsonObject prepareRoutineView(const QJsonObject& aRoutineConfig);
    QJsonValue modifyValue(const QJsonValue& aTarget, const QStringList& aKeys, const int aIndex, const QJsonValue aValue);
    std::function<void(rea::stream<QJsonObject>*)> getShowParam(const QString& aBoardName);
    std::shared_ptr<comObject> m_root_com;
    comObject* m_sel_com = nullptr;
    comObject* m_sel_obj = nullptr;

    std::vector<std::shared_ptr<comObject>> m_root_front;
    int m_sel_front = - 1;
    comObject* m_sel_front_obj = nullptr;

    std::vector<std::shared_ptr<comObject>> m_root_back;
    int m_sel_back = - 1;
    comObject* m_sel_back_obj = nullptr;

    QHash<QString, comObject*> m_coms;

    QString m_last_sel_board = "";
    QJsonObject m_page_template;
    QJsonObject m_param_template;
    QJsonObject m_shape_template;
};

#endif
