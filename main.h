#ifndef REAL_DOCUMENT_H_
#define REAL_DOCUMENT_H_

#include "reaC++.h"

class document;

class comObject : public QJsonObject{
public:
    comObject(const QString& aName, document* aParent, const QString& aID = "");
    QString getID(){
        return value("id").toString();
    }
    QString getType(){
        if (contains("atype"))
            return value("atype").toString();
        else
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

OBJTYPE(data)

class projectObject : public comObject{
public:
    projectObject(const QString& aName, document* aParent, const QString& aID = "") : comObject(aName, aParent, aID){
        insert("type", "project");
    }
    QJsonObject generateDataDocument(){
        return getDataRoot()->generateDocument();
    }
    std::shared_ptr<comObject> addChildData(std::shared_ptr<comObject> aData){
        return getDataRoot()->addChild(aData);
    }
    std::shared_ptr<projectObject> getDataRoot(){
        if (!m_data){
            m_data = std::make_shared<projectObject>(getName(), getParent());
            m_data->insert("value", "#" + getID());
        }
        return m_data;
    }
private:
    std::shared_ptr<projectObject> m_data;
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
    QJsonObject prepareEventList(const std::vector<std::shared_ptr<projectObject>>& aList, int aSelect);
    QJsonObject preparePageView(const QJsonObject& aPageConfig);
    QJsonObject prepareRoutineView(const QJsonObject& aRoutineConfig);
    QJsonValue modifyValue(const QJsonValue& aTarget, const QStringList& aKeys, const int aIndex, const QJsonValue aValue);
    std::function<void(rea::stream<QJsonObject>*)> getShowParam(const QString& aBoardName);
    std::shared_ptr<comObject> m_root_com;
    comObject* m_sel_com = nullptr;
    comObject* m_sel_obj = nullptr;

    std::vector<std::shared_ptr<projectObject>> m_root_front;
    int m_sel_front = - 1;
    comObject* m_sel_front_obj = nullptr;
    comObject* m_sel_front_data = nullptr;

    std::vector<std::shared_ptr<projectObject>> m_root_back;
    int m_sel_back = - 1;
    comObject* m_sel_back_obj = nullptr;
    comObject* m_sel_back_data = nullptr;

    QHash<QString, comObject*> m_coms;

    QString m_last_sel_board = "";
    QJsonObject m_page_template;
    QJsonObject m_param_template;
    QJsonObject m_shape_template;
};

#endif
